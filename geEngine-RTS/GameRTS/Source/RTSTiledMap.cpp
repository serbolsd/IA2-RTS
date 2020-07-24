#include "RTSTiledMap.h"
#include "RTSTexture.h"
#include "RTSFunctionsCoords.h"
#include "RTSTree.h"
#include <geDebug.h>
#include <geColor.h>
#include <cmath>

float lerp(float a, float b, float f)
{
  return (a * (1.0 - f)) + (b * f);
}

RTSTiledMap::RTSTiledMap() {
  m_mapSize = Vector2I::ZERO;
  m_scrStart = Vector2I::ZERO;
  m_scrEnd = Vector2I::ZERO;
  m_iCamera = Vector2I::ZERO;
  m_fCamera = Vector2::ZERO;
}

RTSTiledMap::RTSTiledMap(sf::RenderTarget* pTarget, const Vector2I& mapSize) {
  init(pTarget, mapSize);
}

RTSTiledMap::~RTSTiledMap() {
  destroy();
  for (int i = 0; i < m_objects.size(); i++)
  {
    delete m_objects[i];
  }
}

bool
RTSTiledMap::init(sf::RenderTarget* pTarget, const Vector2I& mapSize) {
  if (m_mapGrid.size()) {
    destroy();
  }

  /* initialize random seed: */
  srand(time(NULL));

  m_pTarget = pTarget;

  m_mapGrid.resize(mapSize.x * mapSize.y);
  m_mapGridVisited.resize(mapSize.x * mapSize.y);
  m_mapSize = mapSize;
  for (int32 iterX = 0; iterX < m_mapSize.x; ++iterX) {
    for (int32 iterY = 0; iterY < m_mapSize.y; ++iterY) {
      m_mapGrid[iterY*mapSize.x + iterX].setIndexOnGrid(iterX, iterY);
      m_mapGrid[iterY*mapSize.x + iterX].setIndex(iterY*mapSize.x + iterX);
    }
  }
  for (int i = 0; i <m_mapSize.x; ++i)
  {
    for (int j = 0; j < m_mapSize.y; ++j)
    {
      for (int n = 0; n < 8; ++n)
      {
        int nuevox = i + movx[n];
        int nuevoy = j + movy[n];
        if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) {    // Revisamos que esté en los límites
           m_mapGrid[j*mapSize.x + i].m_connections.push_back(&m_mapGrid[nuevoy *m_mapSize.x + nuevox]);

        }
      }
    }
  }
  m_mapGridCopy = m_mapGrid;

  setCameraStartPosition(0, 0);

  m_mapTextures.resize(TERRAIN_TYPE::kNumObjects);

  String textureName;
  for (uint32 i = 0; i < TERRAIN_TYPE::kNumObjects; ++i) {
    if (TERRAIN_TYPE::kArrow == i)
    {
      if (GameOptions::s_MapIsIsometric)
      {
        textureName = "Textures/Terrain/iso_Arrow.png";
      }
      else
      {
        textureName = "Textures/Terrain/Arrow.png";
      }
    }
    else
    {
      if (GameOptions::s_MapIsIsometric)
      {
        textureName = "Textures/Terrain/iso_terrain_" + toString(i) + ".png";
      }
      else
      {
        textureName = "Textures/Terrain/terrain_" + toString(i) + ".png";
      }
    }
    m_mapTextures[i].loadFromFile(m_pTarget, textureName);
  }
  m_textureObjects.onInit(pTarget);
  //To see a mouse position on render target
  m_mouseInWindow.setFillColor(sf::Color::Magenta);
  m_mouseInWindow.setRadius(4);
  m_mouseInWindow.setOrigin(4, 4);
  m_outLineTail.reserve(5);
 
  preCalc();
  m_stateMachine.init();
  DijkstraSearchForZones(m_StepsToZone);
  for (int i = 0; i < m_mapZones.size(); i++)
  {
    for (int j = 0; j < m_mapZones[i].m_myTiles.size(); j++)
    {
      m_mapGrid[m_mapZones[i].m_myTiles[j]->getIndex()].m_myZone = &m_mapZones[i];
      m_mapZones[i].m_myTiles[j]->m_myZone = &m_mapZones[i];
    }
  }
  checkLimitOfzones();
  actual = nullptr;
  return true;
}

void
RTSTiledMap::destroy() {
  for (size_t i = 0; i < m_mapSize.x; i++)
  {
    for (size_t j = 0; j < m_mapSize.y; j++)
    {
      for (size_t o = 0; o < m_mapGrid[j*m_mapSize.x + i].m_myObject.size(); o++)
      {
        if (nullptr!=m_mapGrid[j*m_mapSize.x + i].m_myObject[o])
        {
          //delete m_mapGrid[j*m_mapSize.x + i].m_myObject[o];
          //m_mapGrid[j*m_mapSize.x + i].m_myObject[o] = nullptr;
        }
      }
      m_mapGrid[j*m_mapSize.x + i].m_myObject.clear();
    }

  }
  m_mapGrid.clear();
  m_mapGridCopy.clear();
  m_mapTextures.clear();

  m_mapSize = Vector2I::ZERO;
  setCameraStartPosition(0, 0);
  preCalc();
  m_stateMachine.onDelete();

}

int8
RTSTiledMap::getCost(const int32 x, const int32 y) const {
  GE_ASSERT((x >= 0) && (x < m_mapSize.x) && (y >= 0) && (y < m_mapSize.y));
  return m_mapGrid[(y*m_mapSize.x) + x].getCost();
}

void
RTSTiledMap::setCost(const int32 x, const int32 y, const int8 cost) {
  GE_ASSERT((x >= 0) && (x < m_mapSize.x) && (y >= 0) && (y < m_mapSize.y));
  m_mapGrid[(y*m_mapSize.x) + x].setCost(cost);
}

int8
RTSTiledMap::getType(const int32 x, const int32 y) const {
  GE_ASSERT((x >= 0) && (x < m_mapSize.x) && (y >= 0) && (y < m_mapSize.y));
  return m_mapGrid[(y*m_mapSize.x) + x].getType();
}

void
RTSTiledMap::setType(const int32 x, const int32 y, const uint8 idtype) {
  GE_ASSERT((x >= 0) && (x < m_mapSize.x) && (y >= 0) && (y < m_mapSize.y));
  m_mapGrid[(y*m_mapSize.x) + x].setType(idtype);
}

void
RTSTiledMap::moveCamera(const float dx, const float dy) {
  m_fCamera.x += dx;
  m_fCamera.y += dy;

  m_fCamera.x = Math::clamp(m_fCamera.x, 0.f, static_cast<float>(m_PreCalc_MaxCameraCoord.x));
  m_fCamera.y = Math::clamp(m_fCamera.y, 0.f, static_cast<float>(m_PreCalc_MaxCameraCoord.y));

  setCameraStartPosition(Math::trunc(m_fCamera.x),
                         Math::trunc(m_fCamera.y));
}

void
RTSTiledMap::setCameraStartPosition(const int32 x, const int32 y) {
  Vector2I tmpPos(x, y);
  tmpPos.x = Math::clamp(tmpPos.x, Vector2I::ZERO.x, m_PreCalc_MaxCameraCoord.x);
  tmpPos.y = Math::clamp(tmpPos.y, Vector2I::ZERO.y, m_PreCalc_MaxCameraCoord.y);
  if (m_iCamera != tmpPos)
  {
    m_bTileDrawn.clear();
    m_bTileDrawn.resize(m_mapSize.x*m_mapSize.y);
  }
  m_iCamera = tmpPos;
  GameOptions::s_CameraPosition = m_iCamera;

  if (GameOptions::s_MapIsIsometric)
  {
    m_PreCalc_ScreenDeface.x = m_scrStart.x + m_PreCalc_MidResolution.x -
      (m_iCamera.x - m_iCamera.y);
    m_PreCalc_ScreenDeface.y = m_scrStart.y + m_PreCalc_MidResolution.y -
      ((m_iCamera.x + m_iCamera.y) >> 1);
  }
  else 
  {
    m_PreCalc_ScreenDeface = m_scrStart + m_PreCalc_MidResolution - m_iCamera;
  }
  COORDS::PreCalc_ScreenDeface = m_PreCalc_ScreenDeface;
}

void
RTSTiledMap::getScreenToMapCoords(const int32 scrX, 
                                  const int32 scrY,
                                  int32 &mapX,
                                  int32 &mapY) {
  if (GameOptions::s_MapIsIsometric)
  {
    float fscrX = ((float)(scrX - m_PreCalc_ScreenDeface.x) / GameOptions::TILEHALFSIZE.x) - 1;
    float fscrY = ((float)(scrY - m_PreCalc_ScreenDeface.y) / GameOptions::TILEHALFSIZE.y);

    mapX = (Math::trunc(fscrX + fscrY)) >> 1;
    mapY = (Math::trunc(fscrY - fscrX)) >> 1;
  }
  else
  {
    mapX = (scrX - m_PreCalc_ScreenDeface.x) >> GameOptions::BITSHFT_TILESIZE.x;
    mapY = (scrY - m_PreCalc_ScreenDeface.y) >> GameOptions::BITSHFT_TILESIZE.y;
  }

  mapX = Math::clamp(mapX, 0, m_mapSize.x - 1);
  mapY = Math::clamp(mapY, 0, m_mapSize.y - 1);
}

void
RTSTiledMap::getMapToScreenCoords(const int32 mapX,//para imprimir convierte en una posicion para dibujar
                                  const int32 mapY,
                                  int32 &scrX,
                                  int32 &scrY) {
  GE_ASSERT(mapX >= 0 && mapX <= m_mapSize.x && mapY >= 0 && mapY <= m_mapSize.y);

  if (GameOptions::s_MapIsIsometric)
  {
    scrX = (mapX - mapY) << GameOptions::BITSFHT_TILEHALFSIZE.x;
    scrY = (mapX + mapY) << GameOptions::BITSFHT_TILEHALFSIZE.y;
  
    scrX += m_PreCalc_ScreenDeface.x;
    scrY += m_PreCalc_ScreenDeface.y;
  }
  else
  {
    scrX = (mapX << GameOptions::BITSHFT_TILESIZE.x) + m_PreCalc_ScreenDeface.x;
    scrY = (mapY << GameOptions::BITSHFT_TILESIZE.y) + m_PreCalc_ScreenDeface.y;
  }
}

sf::Vector2f RTSTiledMap::getMousePosOnRenderTarget()
{
  Vector2I mousePositionVI;
  sf::RenderWindow &refTowindow = *static_cast<sf::RenderWindow*>(m_pTarget);
  mousePositionVI.x = sf::Mouse::getPosition(refTowindow).x;
  mousePositionVI.y = sf::Mouse::getPosition(refTowindow).y;
  //std::cout << mousePositionVI.x << ", " << mousePositionVI.y << std::endl;
  sf::Vector2i mousePosToPixel;
  mousePosToPixel.x = mousePositionVI.x;
  mousePosToPixel.y = mousePositionVI.y;
  sf::Vector2f mouseInTargetSpace = m_pTarget->mapPixelToCoords(mousePosToPixel);
  //std::cout << mouseInTargetSpace.x << ", " << mouseInTargetSpace.y;
  //system("cls");
  return mouseInTargetSpace;
}

void RTSTiledMap::selecetStartTail()
{
  if (nullptr != m_tileStart)
  {
    m_tileStart->setcolor(m_NormalTileColor);
  }
  if (m_tileSelectedIndex >=0)
  {
    m_selectedUnit = nullptr;
    m_tileStart = &m_mapGridCopy[m_tileSelectedIndex];
    m_tileStart->setcolor(m_tileStartColor);
    m_tileStartIndexX = m_tileSelectedIndexX;
    m_tileStartIndexY = m_tileSelectedIndexY;
    //Vector2I pixelPos;
    //COORDS::getTileCenterOnPixelCoords(m_tileSelectedIndexX, m_tileSelectedIndexY, pixelPos.x, pixelPos.y);
    //m_archerUnit.m_position = { float(pixelPos.x), float(pixelPos.y) };
    //m_archerUnit.m_lasTile = { m_tileSelectedIndexX ,m_tileSelectedIndexY };
    //m_archerUnit.m_bOnMap = true;
    //m_tileStart->addObject(&m_archerUnit);
  }
  
}

void RTSTiledMap::selecetFinalTail()
{
  if (nullptr != m_tileFinish){
    m_tileFinish->setcolor(m_NormalTileColor);
  }
  if (m_tileSelectedIndex >= 0)
  {
    m_tileFinish = &m_mapGridCopy[m_tileSelectedIndex];
    m_tileFinish->setcolor(m_tileFinishColor);
    m_tileFinishIndexX = m_tileSelectedIndexX;
    m_tileFinishIndexY = m_tileSelectedIndexY;
  }
}

void RTSTiledMap::startPathFinding()
{

  clearSearch();
  m_mapGridVisited.resize(m_mapSize.x*m_mapSize.y);           // Tiles visited
  m_bTileDrawn.resize(m_mapSize.x*m_mapSize.y);
  m_StepsDone = 0;
  m_bSearching = true;
  m_bGetPathBT = false;
  int initIndex = m_tileStartIndexY * m_mapSize.x + m_tileStartIndexX;
  if (PATHFINDING::BEST==m_ePathFinding || PATHFINDING::ASTAR == m_ePathFinding)
  {
    switch (m_eEuristicType)
    {
    case EUCLIDEAN:
      m_euritic = Vector2::distance(Vector2(float(m_tileStartIndexX), float(m_tileStartIndexY)),
        Vector2(float(m_tileFinishIndexX), float(m_tileFinishIndexY)));
      break;
    case SQUAREDISTANCE:
      m_euritic = Vector2::distSquared(Vector2(float(m_tileStartIndexX), float(m_tileStartIndexY)),
        Vector2(float(m_tileFinishIndexX), float(m_tileFinishIndexY)));
      break;
    case MANHATTAN:
      break;
    }
    m_mapGridCopy[initIndex].setEuristic(m_euritic*m_euristicRelevance);
  }
 
  if (PATHFINDING::DIJKSTRA != m_ePathFinding || PATHFINDING::ASTAR != m_ePathFinding)
  {
    for (int i = 0; i < m_mapGrid.size(); i++)
    {
      m_mapGrid[i].setTentativeCost(float(INT_MAX));
      setTileCost(i, m_mapGrid[i].getType());
    }
    m_pathStack.push_front(&m_mapGridCopy[initIndex]);
  }
  m_mapGridCopy[initIndex].setIndex(initIndex);
  m_mapGridCopy[initIndex].setIndexOnGrid(m_tileStartIndexX, m_tileStartIndexY);
  actual = &m_mapGridCopy[initIndex];
  m_mapGridCopy[initIndex].setTentativeCost(0);
  m_mapGridCopy[initIndex].setCost(0);
}

void RTSTiledMap::setEuristic(const int & i)
{
  switch (i)
  {
  case 0:
    m_eEuristicType = EURISTIC_TYPE::EUCLIDEAN;
    break;
  case 1:
    m_eEuristicType = EURISTIC_TYPE::SQUAREDISTANCE;
    break;
  case 2:
    break;
  default:
    break;
  }
}

void RTSTiledMap::setPathFinding(const int & i)
{
  if (i > 4)
  {
    return;
  }
  switch (i)
  {
  case 0:
    m_ePathFinding = PATHFINDING::DEPTH;
    break;
  case 1:
    m_ePathFinding = PATHFINDING::BREADTH;
    break;
  case 2:
    m_ePathFinding = PATHFINDING::BEST;
    break;
  case 3:
    m_ePathFinding = PATHFINDING::DIJKSTRA;
    break;
  case 4:
    m_ePathFinding = PATHFINDING::ASTAR;
    break;
  default:
    break;
  }
}

void RTSTiledMap::setTileType(const int & i)
{
  if (m_tileSelectedIndex >= 0)
  {
    switch (i)
    {
    case 0://Water
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setType(0);
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setCost(3);
      break;
    case 1://Grass
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setType(1);
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setCost(1);
      break;
    case 2://Pantano
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setType(2);
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setCost(5);
      break;
    case 3://Obstacle
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setType(3);
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setCost(20);
      break;
    default:
      break;
    }
  }
}

void RTSTiledMap::setTileCost(int index, int type)
{
  if (index >= 0 && index<m_mapGrid.size())
  {
    switch (type)
    {
    case 0://Water*
      m_mapGrid[index].setCost(3);
      break;
    case 1://Grass
      m_mapGrid[index].setCost(1);
      break;
    case 2://Pantano
      m_mapGrid[index].setCost(5);
      break;
    case 3://Obstacle
      m_mapGrid[index].setCost(20);
      break;
    default:
      break;
    }
  }
}

void RTSTiledMap::addTree()
{
  if (m_tileSelectedIndex >= 0)
  {
    RTSTree* tmpObject=new RTSTree;

    tmpObject->setTile(m_tileSelectedIndexX, m_tileSelectedIndexY);

    /* generate secret number between 1 and 10: */
    int id = rand() % m_textureObjects.getTesturesTreesData().size();
    tmpObject->setIDofData(id);
    tmpObject->setType(TYPEOBJECT::TREE);
    tmpObject->setTextures(&m_textureObjects.getTesturesTrees());
    tmpObject->setTextureData(m_textureObjects.getTesturesTreesData()[id]);

    m_objects.push_back(tmpObject);
    m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].addObject(tmpObject);
    m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].m_haveObtacle = true;
    //m_mapGridCopy.clear();
    //m_mapGridCopy = m_mapGrid;
  }
}

void RTSTiledMap::addUnit(UNITTYPE::E _tipe, bool teamRed)
{
  if (m_tileSelectedIndex >= 0)
  {
    switch (_tipe)
    {
    case UNITTYPE::KTERRESTRIAL:
      if (m_mapGrid[m_tileSelectedIndex].getType()!=TERRAIN_TYPE::kGrass && 
        m_mapGrid[m_tileSelectedIndex].getType() != TERRAIN_TYPE::kMarsh)
      {
        return;
      }
      break;
    case UNITTYPE::KFLYING:
      break;
    case UNITTYPE::KMARINE:
      if (m_mapGrid[m_tileSelectedIndex].getType() != TERRAIN_TYPE::kWater)
      {
        return;
      }
      break;
    case UNITTYPE::KNUMUNITTYPES:
      return;
      break;
    default:
      return;
      break;
    }
    RTSUnit* tmpObject = new RTSUnit;
    tmpObject->m_tileIndexX = m_tileSelectedIndexX;
    tmpObject->m_tileIndexY = m_tileSelectedIndexY;

    /* generate secret number between 1 and 10: */
    tmpObject->setType(TYPEOBJECT::UNIT);
    tmpObject->m_unitType = _tipe;
    tmpObject->setStateMachine(&m_stateMachine);

    switch (_tipe)
    {
    case UNITTYPE::KTERRESTRIAL:
      if (teamRed)
      {
        tmpObject->SetAnimationData(&m_textureObjects.getTesturesUnitB(), m_textureObjects.getArcherAnimations());
        tmpObject->setInfluence(-1.0f);
      }
      else
      {
        tmpObject->SetAnimationData(&m_textureObjects.getTesturesUnit(), m_textureObjects.getArcherAnimations());
        tmpObject->setInfluence(1.0f);
      }
      break;
    case UNITTYPE::KFLYING:
      if (teamRed)
      {
        tmpObject->SetAnimationData(&m_textureObjects.getTesturesUnitB(), m_textureObjects.getflyAnimations());
        tmpObject->setInfluence(-1.0f);
      }
      else
      {
        tmpObject->SetAnimationData(&m_textureObjects.getTesturesUnit(), m_textureObjects.getflyAnimations());
        tmpObject->setInfluence(1.0f);
      }
      break;
    case UNITTYPE::KMARINE:
      if (teamRed)
      {
        tmpObject->SetAnimationData(&m_textureObjects.getTesturesUnitB(), m_textureObjects.getwaterAnimations());
        tmpObject->setInfluence(-1.0f);
      }
      else
      {
        tmpObject->SetAnimationData(&m_textureObjects.getTesturesUnit(), m_textureObjects.getwaterAnimations());
        tmpObject->setInfluence(1.0f);
      }
      break;
    default:
      return;
      break;
    }


    Vector2I pixelPos;
    COORDS::getTileCenterOnPixelCoords(m_tileSelectedIndexX, m_tileSelectedIndexY, pixelPos.x, pixelPos.y);
    tmpObject->m_position = { float(pixelPos.x), float(pixelPos.y) };
    //tmpObject->setPosition(float(pixelPos.x), float(pixelPos.y));
    tmpObject->m_lasTile = { m_tileSelectedIndexX ,m_tileSelectedIndexY };
    tmpObject->m_bOnMap = true;
    tmpObject->onInit();

    m_objects.push_back(tmpObject);
    m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].addObject(tmpObject);
    //m_mapGridCopy.clear();
    //m_mapGridCopy = m_mapGrid;
  }
}

void RTSTiledMap::selectUnit()
{
  if (m_tileSelectedIndex >= 0)
  {
    //m_tileStart = &m_mapGridCopy[m_tileSelectedIndex];
    //m_tileStart->setcolor(m_tileStartColor);
    //m_tileStartIndexX = m_tileSelectedIndexX;
    //m_tileStartIndexY = m_tileSelectedIndexY;
    for (int i = 0; i < m_mapGrid[m_tileSelectedIndex].m_myObject.size(); i++)
    {
      if (TYPEOBJECT::UNIT == m_mapGrid[m_tileSelectedIndex].m_myObject[i]->getType())
      {
        m_selectedUnit = reinterpret_cast<RTSUnit*>(m_mapGrid[m_tileSelectedIndex].m_myObject[i]);
      }
    }
    //Vector2I pixelPos;
    //COORDS::getTileCenterOnPixelCoords(m_tileSelectedIndexX, m_tileSelectedIndexY, pixelPos.x, pixelPos.y);
    //m_archerUnit.m_position = { float(pixelPos.x), float(pixelPos.y) };
    //m_archerUnit.m_lasTile = { m_tileSelectedIndexX ,m_tileSelectedIndexY };
    //m_archerUnit.m_bOnMap = true;
    //m_tileStart->addObject(&m_archerUnit);
  }
}

void RTSTiledMap::propagateInfluence(int tileIndex)
{
  float maxInf = 0.0f;
  float minInf = 0.0f;
  for (int i = 0; i <m_mapGrid[tileIndex].m_connections.size(); i++)
  {
    float inf = m_mapGrid[tileIndex].m_connections[i]->m_influence * expf(-1 * m_decay);
    maxInf = std::max(inf, maxInf);
    minInf = std::min(inf, minInf);
  }
  float newInfluence= minInf + maxInf;
  m_mapGrid[tileIndex].m_influence = lerp(m_mapGrid[tileIndex].m_influence, newInfluence, m_momentumPerSecond); 
}

void RTSTiledMap::drawTailOutline(const int & tailIndexX, const int & tailIndexY,const sf::Color & outlineColor)
{
  Vector2I tempceneterPos;
  Vector2 tempScrnPos;
  COORDS::getTileCenterOnPixelCoords(tailIndexX,
    tailIndexY,
    tempceneterPos.x,
    tempceneterPos.y);
  COORDS::getPixelToScreenCoords(tempceneterPos.x,
    tempceneterPos.y,
    tempScrnPos.x,
    tempScrnPos.y);
  sf::Vector2f tailPos = { tempScrnPos.x, tempScrnPos.y};
  if (m_bRedTeam)
  {
    m_mapGrid[tailIndexY*m_mapSize.x+ tailIndexX].m_influence=-1.0f;
  }
  else
  {
    m_mapGrid[tailIndexY*m_mapSize.x+ tailIndexX].m_influence=1.0f;
  }
  if (m_outLineTail.size()<=0)
  {
    m_outLineTail.resize(5);
  }
  if (GameOptions::s_MapIsIsometric)
  {
    m_outLineTail[0].position.x = tailPos.x - GameOptions::TILEHALFSIZE.x;
    m_outLineTail[0].position.y = tailPos.y;
    m_outLineTail[1].position.x = tailPos.x;
    m_outLineTail[1].position.y = tailPos.y - GameOptions::TILEHALFSIZE.y;
    m_outLineTail[2].position.x = tailPos.x + GameOptions::TILEHALFSIZE.x;
    m_outLineTail[2].position.y = tailPos.y;
    m_outLineTail[3].position.x = tailPos.x;
    m_outLineTail[3].position.y = tailPos.y + GameOptions::TILEHALFSIZE.y;
  }
  else
  {
    m_outLineTail[0].position.x = tailPos.x - GameOptions::TILEHALFSIZE.x;
    m_outLineTail[0].position.y = tailPos.y + GameOptions::TILEHALFSIZE.y;
    m_outLineTail[1].position.x = tailPos.x + GameOptions::TILEHALFSIZE.x;
    m_outLineTail[1].position.y = tailPos.y + GameOptions::TILEHALFSIZE.y;
    m_outLineTail[2].position.x = tailPos.x + GameOptions::TILEHALFSIZE.x;
    m_outLineTail[2].position.y = tailPos.y - GameOptions::TILEHALFSIZE.y;
    m_outLineTail[3].position.x = tailPos.x - GameOptions::TILEHALFSIZE.y;
    m_outLineTail[3].position.y = tailPos.y - GameOptions::TILEHALFSIZE.y;
  }

  m_outLineTail[0].color = outlineColor;
  m_outLineTail[1].color = outlineColor;
  m_outLineTail[2].color = outlineColor;
  m_outLineTail[3].color = outlineColor;
  m_outLineTail[4] = m_outLineTail[0];
  m_pTarget->draw(&m_outLineTail[0], 5, sf::PrimitiveType::LinesStrip);
}

void RTSTiledMap::setArrow()
{
  if (nullptr == m_linePathTile || nullptr == m_linePathTile->getParent())
  {
    return;
  }
  MapTile* tmpParent = m_linePathTile->getParent();
  Vector2I indexParent = { tmpParent->getIndexGridX(),tmpParent->getIndexGridY() };
  Vector2I index = { m_linePathTile->getIndexGridX(),m_linePathTile->getIndexGridY() };
  Vector2I direction = index - indexParent;
  if (0 == direction.x)
  {
    if (1 == direction.y)
    {
      m_linePathTile->m_directionArrow = ARROW_TYPE::kDown;
    }
    else
    {
      m_linePathTile->m_directionArrow = ARROW_TYPE::kUp;
    }
  }
  else if (0 == direction.y)
  {
    if (1 == direction.x)
    {
      m_linePathTile->m_directionArrow = ARROW_TYPE::kRight;
    }
    else
    {
      m_linePathTile->m_directionArrow = ARROW_TYPE::kLeft;
    }
  }
  else
  {
    if (1 == direction.y && 1 == direction.x)
    {
      m_linePathTile->m_directionArrow = ARROW_TYPE::kRightDown;
    }
    else if (-1 == direction.y && 1 == direction.x)
    {
      m_linePathTile->m_directionArrow = ARROW_TYPE::kRightUp;
    }
    else if (1 == direction.y && -1 == direction.x)
    {
      m_linePathTile->m_directionArrow = ARROW_TYPE::kLeftDown;
    }
    else
    {
      m_linePathTile->m_directionArrow = ARROW_TYPE::kLeftUp;
    }
  }
  m_linePathTile->hadArrow = true;
}

bool RTSTiledMap::checkCanAddTile(UNITTYPE::E typeUnit, MapTile* tile)
{
  switch (typeUnit)
  {
  case UNITTYPE::KTERRESTRIAL:
    if (TERRAIN_TYPE::E::kGrass == tile->getType() || TERRAIN_TYPE::E::kMarsh == tile->getType())
    {
      if (tile->m_haveObtacle)
      {
        return false;
      }
      return true;
    }
    break;
  case UNITTYPE::KFLYING:
    return true;
    break;
  case UNITTYPE::KMARINE:
    if (TERRAIN_TYPE::E::kWater == tile->getType())
    {
      if (tile->m_haveObtacle)
      {
        return false;
      }
      return true;
    }
    break;
  case UNITTYPE::KNUMUNITTYPES:
    return true;
    break;
  default:
    return true;
    break;
  }
  return false;
}

void RTSTiledMap::BreadthFirstSearch()
{
  if (m_pathStack.empty())
  {
    m_bSearching = false;
    clearSearch();
    return;
  }

  
  actual->setcolor(m_serchingTileColor);
  actual = m_pathStack.front();       // Sacamos el estado a procesar
  actual->setcolor(m_lastSerchingTileColor);

  int actualIndX = actual->getIndexGridX();
  int actualIndY = actual->getIndexGridY();

  while (m_mapGridVisited[actualIndY *m_mapSize.x + actualIndX])
  {
    m_pathStack.pop_front();
    actual = m_pathStack.front();
    actualIndX = actual->getIndexGridX();
    actualIndY = actual->getIndexGridY();
  }
  m_mapSearchRegister.push_back(actual);
  m_pathStack.pop_front();                         // Sacamos ese estado de la cola

  if (isSearchFinish(actualIndX, actualIndY))
  {
    return;
  }

  m_mapGridVisited[actualIndY*m_mapSize.x + actualIndX] = true;    // Marcamos como visitada la casilla actual

  for (int i = 0; i < 8; i++) {
    int nuevox = actualIndX + movx[i];
    int nuevoy = actualIndY + movy[i];

    if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) {    // Revisamos que esté en los límites
      if (!m_mapGridVisited[nuevoy *m_mapSize.x + nuevox] && nullptr == m_mapGridCopy[nuevoy*m_mapSize.x + nuevox].getParent() &&
        TERRAIN_TYPE::kObstacle != m_mapGrid[nuevoy *m_mapSize.x + nuevox].getType()) {  // Revisamos que no esté visitado y que no sea pared
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX]);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndex(nuevoy *m_mapSize.x + nuevox);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndexOnGrid(nuevox, nuevoy);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
        m_pathStack.push_back(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);
      }
    }
  }
}

void RTSTiledMap::BreadthFirstSearchWithSteps(uint32 steps, UNITTYPE::E typeUnit)
{
  if (m_pathStack.empty())
  {
    m_bSearching = false;
    clearSearch();
    return;
  }

  
  actual->setcolor(m_serchingTileColor);
  actual = m_pathStack.front();       // Sacamos el estado a procesar
  actual->setcolor(m_lastSerchingTileColor);

  int actualIndX = actual->getIndexGridX();
  int actualIndY = actual->getIndexGridY();

  Vector2 actualIndex = { (float)actualIndX, (float)actualIndY };
  Vector2 finalIndex = { (float)m_tileFinishIndexX, (float)m_tileFinishIndexY };
  float distToactual = Vector2::distance(finalIndex, actualIndex);
  float distToCurrentNearlier = float(INT_MAX);
  if (nullptr != m_pNearlearTile)
  {
    Vector2 nearIndex = { (float)m_pNearlearTile->getIndexGridX(), (float)m_pNearlearTile->getIndexGridY() };
    distToCurrentNearlier = Vector2::distance(finalIndex, nearIndex);
  }
  if (nullptr == m_pNearlearTile || distToCurrentNearlier > distToactual)
  {
    m_pNearlearTile = actual;
  }
  while (m_mapGridVisited[actualIndY *m_mapSize.x + actualIndX])
  {
    m_pathStack.pop_front();
    actual = m_pathStack.front();
    actualIndX = actual->getIndexGridX();
    actualIndY = actual->getIndexGridY();
  }
  m_mapSearchRegister.push_back(actual);
  m_pathStack.pop_front();                         // Sacamos ese estado de la cola

  if (m_StepsDone == steps)
  {
    m_pCurrentTailNode = m_pNearlearTile;
    m_pCurrentTailNode->setcolor(m_PathTileColor);
    m_bSearching = false;
    m_bGetPath = true;
    return;
  }
  if (isSearchFinish(actualIndX, actualIndY))
  {
    return;
  }

  m_mapGridVisited[actualIndY*m_mapSize.x + actualIndX] = true;    // Marcamos como visitada la casilla actual

  for (int i = 0; i < 8; i++) {
    int nuevox = actualIndX + movx[i];
    int nuevoy = actualIndY + movy[i];

    if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) {    // Revisamos que esté en los límites
      if (!m_mapGridVisited[nuevoy *m_mapSize.x + nuevox] && 
          nullptr == m_mapGridCopy[nuevoy*m_mapSize.x + nuevox].getParent() &&
          checkCanAddTile(typeUnit, &m_mapGrid[nuevoy *m_mapSize.x + nuevox])) {  // Revisamos que no esté visitado y que no sea pared
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX]);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndex(nuevoy *m_mapSize.x + nuevox);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndexOnGrid(nuevox, nuevoy);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
        m_pathStack.push_back(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);
        ++m_StepsDone;
        if (m_StepsDone == steps)
        {
          break;
        }
      }
    }
  }
}

void
RTSTiledMap::DepthFirstSearch()
{

  if (m_pathStack.empty())
  {
    m_bSearching = false;
    clearSearch();
    return;
  }
  
  actual->setcolor(m_serchingTileColor);
  actual = m_pathStack.back();       // Sacamos el estado a procesar
  actual->setcolor(m_lastSerchingTileColor);

  int actualIndX = actual->getIndexGridX();
  int actualIndY = actual->getIndexGridY();

  m_mapSearchRegister.push_back(actual);
  m_pathStack.pop_back();                         // Sacamos ese estado de la cola

  if (isSearchFinish(actualIndX, actualIndY))
  {
    return;
  }

  m_mapGridVisited[actualIndY*m_mapSize.x + actualIndX] = true;    // Marcamos como visitada la casilla actual

  for (int i = 8-1; i > -1; i--) {           
    int nuevox = actualIndX + movx[i];    
    int nuevoy = actualIndY + movy[i];    

    if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) {    // Revisamos que esté en los límites
      if (!m_mapGridVisited[nuevoy *m_mapSize.x + nuevox]&&nullptr == m_mapGridCopy[nuevoy*m_mapSize.x + nuevox].getParent()&&
        TERRAIN_TYPE::kObstacle != m_mapGrid[nuevoy *m_mapSize.x + nuevox].getType()) {  // Revisamos que no esté visitado y que no sea pared
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX]);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndex(nuevoy *m_mapSize.x + nuevox);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndexOnGrid(nuevox, nuevoy);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
        m_pathStack.push_back(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);
      }
    }
  }
}

void 
RTSTiledMap::DepthFirstSearchWithSteps(uint32 steps, UNITTYPE::E typeUnit) {
  if (m_pathStack.empty())
  {
    m_bSearching = false;
    clearSearch();
    return;
  }

  actual->setcolor(m_serchingTileColor);
  actual = m_pathStack.back();       // Sacamos el estado a procesar
  actual->setcolor(m_lastSerchingTileColor);

  int actualIndX = actual->getIndexGridX();
  int actualIndY = actual->getIndexGridY();

  Vector2 actualIndex = { (float)actualIndX, (float)actualIndY };
  Vector2 finalIndex = { (float)m_tileFinishIndexX, (float)m_tileFinishIndexY };
  float distToactual = Vector2::distance(finalIndex, actualIndex);
  float distToCurrentNearlier = float(INT_MAX);
  if (nullptr != m_pNearlearTile)
  {
    Vector2 nearIndex = { (float)m_pNearlearTile->getIndexGridX(), (float)m_pNearlearTile->getIndexGridY() };
    distToCurrentNearlier = Vector2::distance(finalIndex, nearIndex);
  }
  if (nullptr == m_pNearlearTile || distToCurrentNearlier > distToactual)
  {
    m_pNearlearTile = actual;
  }

  m_mapSearchRegister.push_back(actual);
  m_pathStack.pop_back();                         // Sacamos ese estado de la cola

  if (m_StepsDone == steps)
  {
    m_pCurrentTailNode = m_pNearlearTile;
    m_pCurrentTailNode->setcolor(m_PathTileColor);
    m_bSearching = false;
    m_bGetPath = true;
    return;
  }

  if (isSearchFinish(actualIndX, actualIndY))
  {
    return;
  }

  m_mapGridVisited[actualIndY*m_mapSize.x + actualIndX] = true;    // Marcamos como visitada la casilla actual

  for (int i = 8 - 1; i > -1; i--) {
    int nuevox = actualIndX + movx[i];
    int nuevoy = actualIndY + movy[i];

    if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) {    // Revisamos que esté en los límites
      if (!m_mapGridVisited[nuevoy *m_mapSize.x + nuevox] && nullptr == m_mapGridCopy[nuevoy*m_mapSize.x + nuevox].getParent() &&
          checkCanAddTile(typeUnit, &m_mapGrid[nuevoy *m_mapSize.x + nuevox])) {  // Revisamos que no esté visitado y que no sea pared
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX]);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndex(nuevoy *m_mapSize.x + nuevox);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndexOnGrid(nuevox, nuevoy);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
        m_pathStack.push_back(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);
        ++m_StepsDone;
        if (m_StepsDone == steps)
        {
          break;
        }
      }
    }
  }
}

void RTSTiledMap::BestFirstSearch()
{
  if (m_pathStack.empty())
  {
    m_bSearching = false;
    clearSearch();
    return;
  }
  
  actual->setcolor(m_serchingTileColor);
  actual = m_pathStack.front();       // Sacamos el estado a procesar
  actual->setcolor(m_lastSerchingTileColor);
  
  int actualIndX = actual->getIndexGridX();
  int actualIndY = actual->getIndexGridY();

  m_mapSearchRegister.push_back(actual);
  m_pathStack.pop_front();                         // Sacamos ese estado de la cola

  if (isSearchFinish(actualIndX, actualIndY))
  {
    return;
  }

  m_mapGridVisited[actualIndY*m_mapSize.x + actualIndX] = true;    // Marcamos como visitada la casilla actual
  vector<MapTile*> Nodes;
  for (int i = 0; i < 8; i++) {
    int nuevox = actualIndX + movx[i];
    int nuevoy = actualIndY + movy[i];

    if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) {    // Revisamos que esté en los límites
      if (!m_mapGridVisited[nuevoy *m_mapSize.x + nuevox]&&nullptr==m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].getParent()&&
        TERRAIN_TYPE::kObstacle != m_mapGrid[nuevoy *m_mapSize.x + nuevox].getType()) {  // Revisamos que no esté visitado y que no sea pared
        float eu= Vector2::distance(Vector2(float(nuevox), float(nuevoy)), Vector2(float(m_tileFinishIndexX), float(m_tileFinishIndexY)));
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX]);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndex(nuevoy *m_mapSize.x + nuevox);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndexOnGrid(nuevox, nuevoy);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setEuristic(eu);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
        Nodes.push_back(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);
      }
    }
  }
  size_t n = Nodes.size();
  for (size_t i = 0; i < n; i++)
  {
    for (size_t j = 0; j < n; j++)
    {
      if (Nodes[i]->getEuristic() > Nodes[j]->getEuristic())
      {
        auto temp = Nodes[i];
        Nodes[i] = Nodes[j];
        Nodes[j] = temp;
      }
    }
  }
  for (size_t i = 0; i < n; i++)
  {
    m_pathStack.push_front(Nodes[i]);
  }
}

void 
RTSTiledMap::BestFirstSearchWithSteps(uint32 steps, UNITTYPE::E typeUnit) {
  if (m_pathStack.empty())
  {
    m_bSearching = false;
    clearSearch();
    return;
  }

  actual->setcolor(m_serchingTileColor);
  actual = m_pathStack.front();       // Sacamos el estado a procesar
  actual->setcolor(m_lastSerchingTileColor);

  int actualIndX = actual->getIndexGridX();
  int actualIndY = actual->getIndexGridY();

  Vector2 actualIndex = { (float)actualIndX, (float)actualIndY };
  Vector2 finalIndex = { (float)m_tileFinishIndexX, (float)m_tileFinishIndexY };
  float distToactual = Vector2::distance(finalIndex, actualIndex);
  float distToCurrentNearlier = float(INT_MAX);
  if (nullptr != m_pNearlearTile)
  {
    Vector2 nearIndex = { (float)m_pNearlearTile->getIndexGridX(), (float)m_pNearlearTile->getIndexGridY() };
    distToCurrentNearlier = Vector2::distance(finalIndex, nearIndex);
  }
  if (nullptr == m_pNearlearTile || distToCurrentNearlier > distToactual)
  {
    m_pNearlearTile = actual;
  }

  m_mapSearchRegister.push_back(actual);
  m_pathStack.pop_front();                         // Sacamos ese estado de la cola

  if (m_StepsDone == steps)
  {
    m_pCurrentTailNode = m_pNearlearTile;
    m_pCurrentTailNode->setcolor(m_PathTileColor);
    m_bSearching = false;
    m_bGetPath = true;
    return;
  }
  if (isSearchFinish(actualIndX, actualIndY))
  {
    return;
  }

  m_mapGridVisited[actualIndY*m_mapSize.x + actualIndX] = true;    // Marcamos como visitada la casilla actual
  vector<MapTile*> Nodes;
  for (int i = 0; i < 8; i++) {
    int nuevox = actualIndX + movx[i];
    int nuevoy = actualIndY + movy[i];

    if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) {    // Revisamos que esté en los límites
      if (!m_mapGridVisited[nuevoy *m_mapSize.x + nuevox] && nullptr == m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].getParent() &&
        checkCanAddTile(typeUnit, &m_mapGrid[nuevoy *m_mapSize.x + nuevox])) {  // Revisamos que no esté visitado y que no sea pared
        float eu = Vector2::distance(Vector2(float(nuevox), float(nuevoy)), Vector2(float(m_tileFinishIndexX), float(m_tileFinishIndexY)));
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX]);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndex(nuevoy *m_mapSize.x + nuevox);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndexOnGrid(nuevox, nuevoy);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setEuristic(eu);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
        Nodes.push_back(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);
        ++m_StepsDone;
        if (m_StepsDone == steps)
        {
          break;
        }
      }
    }
  }
  size_t n = Nodes.size();
  for (size_t i = 0; i < n; i++)
  {
    for (size_t j = 0; j < n; j++)
    {
      if (Nodes[i]->getEuristic() > Nodes[j]->getEuristic())
      {
        auto temp = Nodes[i];
        Nodes[i] = Nodes[j];
        Nodes[j] = temp;
      }
    }
  }
  for (size_t i = 0; i < n; i++)
  {
    m_pathStack.push_front(Nodes[i]);
  }
}

void RTSTiledMap::DijkstraSearch()
{
  m_pathStack.push_front(actual);
  int actualIndX = actual->getIndexGridX();
  int actualIndY = actual->getIndexGridY();

  m_mapSearchRegister.push_back(actual);
  //m_pathStack.pop_front();                         // Sacamos ese estado de la cola
  
  if (isSearchFinish(actualIndX, actualIndY))
  {
    return;
  }
  if (nullptr != actual)
  {
    m_mapGridVisited[actualIndY*m_mapSize.x + actualIndX] = true;    // Marcamos como visitada la casilla actual
    for (int i = 0; i < 8; i++) {
      int nuevox = actualIndX + movx[i];
      int nuevoy = actualIndY + movy[i];

      if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) {    // Revisamos que esté en los límites
        if (!m_mapGridVisited[nuevoy *m_mapSize.x + nuevox] &&
          nullptr == m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].getParent()) {  // Revisamos que no esté visitado y que no sea pared

          float nodeCost = m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].getTentativeCost();
          float tentativeCost =
            m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX].getTentativeCost() + m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].getCost();
          if (tentativeCost < nodeCost) {
            auto it = m_dijkstra.find(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);

            if (it != m_dijkstra.end())
            {
              m_dijkstra.erase(it);
            }
            m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setTentativeCost(tentativeCost);
            m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX]);
            m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndex(nuevoy *m_mapSize.x + nuevox);
            m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndexOnGrid(nuevox, nuevoy);
            m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
            //m_pathStack.push_front(nuevo);
            m_dijkstra.insert(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);
          }
        }
      }
    }
  }
  if (m_dijkstra.empty())
  {
    clearSearch();
    m_bSearching = false;
    return;
  }
  
  actual->setcolor(m_serchingTileColor);
  actual = *m_dijkstra.begin();       // Sacamos el estado a procesar
  actual->setcolor(m_lastSerchingTileColor); 
  m_dijkstra.erase(m_dijkstra.begin());
}

void RTSTiledMap::DijkstraSearchWithSteps(uint32 steps, UNITTYPE::E typeUnit)
{
  m_pathStack.push_front(actual);
  int actualIndX = actual->getIndexGridX();
  int actualIndY = actual->getIndexGridY();

  Vector2 actualIndex = { (float)actualIndX, (float)actualIndY };
  Vector2 finalIndex = { (float)m_tileFinishIndexX, (float)m_tileFinishIndexY };
  float distToactual = Vector2::distance(finalIndex, actualIndex);
  float distToCurrentNearlier = float(INT_MAX);
  if (nullptr != m_pNearlearTile)
  {
    Vector2 nearIndex = { (float)m_pNearlearTile->getIndexGridX(), (float)m_pNearlearTile->getIndexGridY() };
    distToCurrentNearlier = Vector2::distance(finalIndex, nearIndex);
  }
  if (nullptr == m_pNearlearTile || distToCurrentNearlier > distToactual)
  {
    m_pNearlearTile = actual;
  }

  m_mapSearchRegister.push_back(actual);
  //m_pathStack.pop_front();                         // Sacamos ese estado de la cola
  
  if (isSearchFinish(actualIndX, actualIndY))
  {
    return;
  }
  if (nullptr != actual && m_StepsDone < steps)
  {
    m_mapGridVisited[actualIndY*m_mapSize.x + actualIndX] = true;    // Marcamos como visitada la casilla actual
    for (int i = 0; i < 8; i++) {
      int nuevox = actualIndX + movx[i];
      int nuevoy = actualIndY + movy[i];

      if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) {    // Revisamos que esté en los límites
        if (!m_mapGridVisited[nuevoy *m_mapSize.x + nuevox] &&
          nullptr == m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].getParent()) {  // Revisamos que no esté visitado y que no sea pared
          if (!checkCanAddTile(typeUnit, &m_mapGrid[nuevoy *m_mapSize.x + nuevox]))
          {
            continue;
          }
          float nodeCost = m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].getTentativeCost();
          float tentativeCost =
            m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX].getTentativeCost() + m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].getCost();
          if (tentativeCost < nodeCost) {
            auto it = m_dijkstra.find(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);

            if (it != m_dijkstra.end())
            {
              m_dijkstra.erase(it);
            }
            m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setTentativeCost(tentativeCost);
            m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX]);
            m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndex(nuevoy *m_mapSize.x + nuevox);
            m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndexOnGrid(nuevox, nuevoy);
            m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
            //m_pathStack.push_front(nuevo);
            m_dijkstra.insert(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);
            m_StepsDone++;
            if (m_StepsDone == steps)
            {
              break;
            }
          }
        }
      }
    }
  }
  if (m_dijkstra.empty())
  {
    //clearSearch();
    m_pCurrentTailNode = m_pNearlearTile;
    m_pCurrentTailNode->setcolor(m_PathTileColor);
    m_bSearching = false;
    m_bGetPath = true;
    return;
  }
  
  actual->setcolor(m_serchingTileColor);
  actual = *m_dijkstra.begin();       // Sacamos el estado a procesar
  actual->setcolor(m_lastSerchingTileColor); 
  m_dijkstra.erase(m_dijkstra.begin());
}

void 
RTSTiledMap::DijkstraSearchForZones(uint32 steps)
{
  actual = &m_mapGrid[0];
  int actualIndX;
  int actualIndY;
  MapZone zone;
  int r = rand() % 100 + 100;
  int g = rand() % 100 + 100;
  int b = rand() % 100 + 100;
  int a = 255;
  zone.m_colorOfZone = sf::Color(r, g, b, a);
  zone.m_colorOfLimit = sf::Color(r*0.5f, g*0.5f, b*0.5f, a);
  actual->m_myZone = &zone;
  actual->setTentativeCost(0);
  actual->setCost(0);
  TERRAIN_TYPE::E currentType = (TERRAIN_TYPE::E)actual->getType();
  zone.m_terrainType = currentType;
  while (!m_bAllMapWithZones)
  {
    actualIndX = actual->getIndexGridX();
    actualIndY = actual->getIndexGridY();
    zone.m_myTiles.push_back(actual);
    //actual->m_myZone = &zone;
    if (nullptr != actual)
    {
      for (int i = 0; i < 8; i++) {
        int nuevox = actualIndX + movx[i];
        int nuevoy = actualIndY + movy[i];
        if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) 
        {  
          if ((nullptr == m_mapGrid[nuevoy *m_mapSize.x + nuevox].m_myZone || 
            NULL == m_mapGrid[nuevoy *m_mapSize.x + nuevox].m_myZone) &&
            currentType == m_mapGrid[nuevoy *m_mapSize.x + nuevox].getType() &&
            TERRAIN_TYPE::kObstacle != m_mapGrid[nuevoy *m_mapSize.x + nuevox].getType())
          {
            float nodeCost = m_mapGrid[nuevoy *m_mapSize.x + nuevox].getTentativeCost();
            float tentativeCost =
              m_mapGrid[actualIndY *m_mapSize.x + actualIndX].getTentativeCost() + m_mapGrid[nuevoy *m_mapSize.x + nuevox].getCost();
            if (tentativeCost < nodeCost) {
              auto it = m_dijkstra.find(&m_mapGrid[nuevoy *m_mapSize.x + nuevox]);
              if (it != m_dijkstra.end())
              {
                m_dijkstra.erase(it);
              }
              m_mapGrid[nuevoy *m_mapSize.x + nuevox].setTentativeCost(tentativeCost);
              m_mapGrid[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGrid[actualIndY *m_mapSize.x + actualIndX]);
              m_dijkstra.insert(&m_mapGrid[nuevoy *m_mapSize.x + nuevox]);
              ++m_dijkstraStepsDone;
            }
            if (m_dijkstraStepsDone ==steps )
            {
              m_dijkstraStepsDone = 0;
              m_mapZones.push_back(zone);
              for (int i = 0; i < m_mapZones[m_mapZones.size()-1].m_myTiles.size(); i++)
              {
                m_mapZones[m_mapZones.size() - 1].m_myTiles[i]->m_myZone = &m_mapZones[m_mapZones.size() - 1];
              }
              zone.m_myTiles.clear();
              r = rand() % 100 + 100;
              g = rand() % 100 + 100;
              b = rand() % 100 + 100;
              zone.m_colorOfLimit = sf::Color(r*0.5f, g*0.5f, b*0.5f, a);
              zone.m_colorOfZone = sf::Color( r,g,b, a);
              m_dijkstra.clear();
              for (int i = 0; i < m_mapGrid.size(); i++)
              {
                m_mapGrid[i].setTentativeCost(float(INT_MAX));
                setTileCost(i, m_mapGrid[i].getType());
              }
              for (int i = 0; i < m_mapGrid.size(); i++)
              {
                if (nullptr == m_mapGrid[i].m_myZone  && TERRAIN_TYPE::kObstacle != m_mapGrid[i].getType())
                {
                  m_dijkstra.insert(&m_mapGrid[i]);
                  m_mapGrid[i].setCost(0);
                  m_mapGrid[i].setTentativeCost(0);
                  currentType = (TERRAIN_TYPE::E)m_mapGrid[i].getType();
                  zone.m_terrainType = currentType;
                  break;
                }
              }
              break;
            }
          }
        }
      }
    }
    if (m_dijkstra.empty())
    {
      m_bAllMapWithZones = true;
      m_dijkstraStepsDone = 0;
      m_mapZones.push_back(zone);
      for (int i = 0; i < m_mapZones[m_mapZones.size() - 1].m_myTiles.size(); i++)
      {
        m_mapZones[m_mapZones.size() - 1].m_myTiles[i]->m_myZone = &m_mapZones[m_mapZones.size() - 1];
      }
      zone.m_myTiles.clear();
      r = rand() % 100 + 100;
      g = rand() % 100 + 100;
      b = rand() % 100 + 100;
      zone.m_colorOfLimit = sf::Color(r*0.5f, g*0.5f, b*0.5f, a);
      zone.m_colorOfZone = sf::Color(r, g, b, a);
      m_dijkstra.clear();
      for (int i = 0; i < m_mapGrid.size(); i++)
      {
        m_mapGrid[i].setTentativeCost(float(INT_MAX));
        setTileCost(i, m_mapGrid[i].getType());
      }
      for (int i = 0; i < m_mapGrid.size(); i++)
      {
        if (nullptr == m_mapGrid[i].m_myZone && TERRAIN_TYPE::kObstacle != m_mapGrid[i].getType())
        {
          m_dijkstra.insert(&m_mapGrid[i]);
          m_mapGrid[i].setCost(0);
          m_mapGrid[i].setTentativeCost(0);
          currentType = (TERRAIN_TYPE::E) m_mapGrid[i].getType();
          zone.m_terrainType = currentType;
          m_bAllMapWithZones = false;
          m_dijkstraStepsDone = 0;
          break;
        }
      }
      if (m_bAllMapWithZones)
      {
        clearSearch();
        m_bSearching = false;
        break;
      }
      //return;
    }
    actual = *m_dijkstra.begin();       // Sacamos el estado a procesar
    m_dijkstra.erase(m_dijkstra.begin());
  }
}

void 
RTSTiledMap::checkLimitOfzones() {
  for (int i = 0; i < m_mapZones.size(); ++i)
  {
    for (int j = 0; j < m_mapZones[i].m_myTiles.size(); ++j)
    {
      for (int n = 0; n < m_mapZones[i].m_myTiles[j]->m_connections.size(); ++n)
      {
        if (m_mapZones[i].m_myTiles[j]->m_connections[n]->m_myZone != &m_mapZones[i])
        {
          m_mapZones[i].m_myTiles[j]->m_imLimit= true;
          m_mapZones[i].m_myLimits.push_back(m_mapZones[i].m_myTiles[j]);
        }
      }
    }
  }
}

void 
RTSTiledMap::AstarSearch() { 
  m_pathStack.push_front(actual);
  int actualIndX = actual->getIndexGridX();
  int actualIndY = actual->getIndexGridY();
  int actualIndex = actualIndY * m_mapSize.x + actualIndX;

  m_mapSearchRegister.push_back(actual);
  //m_pathStack.pop_front();                         // Sacamos ese estado de la cola
  
  if (isSearchFinish(actualIndX,actualIndY))
  {
    return;
  }
  
  m_mapGridVisited[actualIndex] = true;    // Marcamos como visitada la casilla actual
  for (int i = 0; i < 8; i++) {
    int nuevox = actualIndX + movx[i];
    int nuevoy = actualIndY + movy[i];
    int nuevoIndex = nuevoy * m_mapSize.x + nuevox;

    if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) {    // Revisamos que esté en los límites
      if (!m_mapGridVisited[nuevoIndex] &&
          nullptr == m_mapGridCopy[nuevoIndex].getParent()) {  // Revisamos que no esté visitado y que no sea pared
        
        float nodeCost = m_mapGridCopy[nuevoIndex].getTentativeCost();
        float tentativeCost = 
        m_mapGridCopy[actualIndex].getTentativeCost() + m_mapGridCopy[nuevoIndex].getCost();
        
        if(tentativeCost < nodeCost){
          auto it = m_aStar.find(&m_mapGridCopy[nuevoIndex]);
  
          if (it != m_aStar.end())
          {
            m_aStar.erase(it);
          }
          float eu = Vector2::distance(Vector2(float(nuevox), float(nuevoy)), Vector2(float(m_tileFinishIndexX), float(m_tileFinishIndexY)));
          m_mapGridCopy[nuevoIndex].setTentativeCost(tentativeCost*m_costRelevance);
          m_mapGridCopy[nuevoIndex].setEuristic(eu*m_euristicRelevance);
          m_mapGridCopy[nuevoIndex].setParent(&m_mapGridCopy[actualIndex]);
          m_mapGridCopy[nuevoIndex].setIndex(nuevoIndex);
          m_mapGridCopy[nuevoIndex].setIndexOnGrid(nuevox, nuevoy);
          m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
          m_aStar.insert(&m_mapGridCopy[nuevoIndex]);
        }
      }
    }
  }
  if (m_aStar.empty())
  {
    m_bSearching = false;
    clearSearch();
    return;
  }
  
  actual->setcolor(m_serchingTileColor);
  actual = *m_aStar.begin();       // Sacamos el estado a procesar
  actual->setcolor(m_lastSerchingTileColor);  
  m_aStar.erase(m_aStar.begin());
}

void 
RTSTiledMap::AstarSearchWithSteps(uint32 steps, UNITTYPE::E typeUnit){
  m_pathStack.push_front(actual);
  int actualIndX = actual->getIndexGridX();
  int actualIndY = actual->getIndexGridY();
  int actualIndex = actualIndY * m_mapSize.x + actualIndX;

  Vector2 actualIndexvec = { (float)actualIndX, (float)actualIndY };
  Vector2 finalIndex = { (float)m_tileFinishIndexX, (float)m_tileFinishIndexY };
  float distToactual = Vector2::distance(finalIndex, actualIndexvec);
  float distToCurrentNearlier = float(INT_MAX);
  if (nullptr != m_pNearlearTile)
  {
    Vector2 nearIndex = { (float)m_pNearlearTile->getIndexGridX(), (float)m_pNearlearTile->getIndexGridY() };
    distToCurrentNearlier = Vector2::distance(finalIndex, nearIndex);
  }
  if (nullptr == m_pNearlearTile || distToCurrentNearlier > distToactual)
  {
    m_pNearlearTile = actual;
  }

  m_mapSearchRegister.push_back(actual);
  //m_pathStack.pop_front();                         // Sacamos ese estado de la cola
  if (m_StepsDone == steps)
  {
    m_pCurrentTailNode = m_pNearlearTile;
    m_pCurrentTailNode->setcolor(m_PathTileColor);
    m_bSearching = false;
    m_bGetPath = true;
    return;
  }
  if (isSearchFinish(actualIndX, actualIndY))
  {
    return;
  }

  m_mapGridVisited[actualIndex] = true;    // Marcamos como visitada la casilla actual
  for (int i = 0; i < 8; i++) {
    int nuevox = actualIndX + movx[i];
    int nuevoy = actualIndY + movy[i];
    int nuevoIndex = nuevoy * m_mapSize.x + nuevox;

    if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) {    // Revisamos que esté en los límites
      if (!m_mapGridVisited[nuevoIndex] &&
        nullptr == m_mapGridCopy[nuevoIndex].getParent() && 
        checkCanAddTile(typeUnit, &m_mapGrid[nuevoy *m_mapSize.x + nuevox])) {  // Revisamos que no esté visitado y que no sea pared

        float nodeCost = m_mapGridCopy[nuevoIndex].getTentativeCost();
        float tentativeCost =
          m_mapGridCopy[actualIndex].getTentativeCost() + m_mapGridCopy[nuevoIndex].getCost();

        if (tentativeCost < nodeCost) {
          auto it = m_aStar.find(&m_mapGridCopy[nuevoIndex]);

          if (it != m_aStar.end())
          {
            m_aStar.erase(it);
          }
          float eu = Vector2::distance(Vector2(float(nuevox), float(nuevoy)), Vector2(float(m_tileFinishIndexX), float(m_tileFinishIndexY)));
          m_mapGridCopy[nuevoIndex].setTentativeCost(tentativeCost*m_costRelevance);
          m_mapGridCopy[nuevoIndex].setEuristic(eu*m_euristicRelevance);
          m_mapGridCopy[nuevoIndex].setParent(&m_mapGridCopy[actualIndex]);
          m_mapGridCopy[nuevoIndex].setIndex(nuevoIndex);
          m_mapGridCopy[nuevoIndex].setIndexOnGrid(nuevox, nuevoy);
          m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
          m_aStar.insert(&m_mapGridCopy[nuevoIndex]);
          m_StepsDone++;
          if (m_StepsDone == steps)
          {
            break;
          }
        }
      }
    }
  }
  if (m_aStar.empty())
  {
    m_bSearching = false;
    clearSearch();
    return;
  }

  actual->setcolor(m_serchingTileColor);
  actual = *m_aStar.begin();       // Sacamos el estado a procesar
  actual->setcolor(m_lastSerchingTileColor);
  m_aStar.erase(m_aStar.begin());
}

bool RTSTiledMap::isSearchFinish(const int x, const int y)
{
  if (&m_mapGridCopy[y *m_mapSize.x + x] == m_tileFinish)
  {
    m_pCurrentTailNode = &m_mapGridCopy[m_tileFinishIndexY *m_mapSize.x + m_tileFinishIndexX];
    m_pCurrentTailNode->setcolor(m_PathTileColor);
    m_bSearching = false;
    m_bGetPath = true;
    return true;
  }
  return false;
}

void RTSTiledMap::returnPath()
{
  if (m_pCurrentTailNode == nullptr || m_pCurrentTailNode->getIndex()==m_tileStartIndexY*m_mapSize.x+m_tileStartIndexX)
  {
    //clearSearch();
    m_mapPathRegisterTailForBT = m_mapPathRegisterTail;
    m_bGetPathBT = true;
    m_bGetPath = false;
    return;
  }
  m_mapPathRegisterTail.push_back(m_pCurrentTailNode);
  m_pCurrentTailNode = m_pCurrentTailNode->getParent();
  m_pCurrentTailNode->setcolor(m_PathTileColor);
}

void RTSTiledMap::returnLinePath()
{
  m_PathLineTail.clear();
  if (nullptr== m_linePathTile &&nullptr!=actual)
  {
    m_linePathTile = actual;
    //return;
  }
  while (m_linePathTile != nullptr)
  {
    if (m_bTileDrawn[m_linePathTile->getIndex()])
    {
      sf::Vertex newVertes;
      Vector2I tempPixelPos;
      Vector2 tempScrnPos;
      COORDS::getTileCenterOnPixelCoords(m_linePathTile->getIndexGridX(),
                                         m_linePathTile->getIndexGridY(),
                                         tempPixelPos.x,
                                         tempPixelPos.y);
      COORDS::getPixelToScreenCoords(tempPixelPos.x,
                                     tempPixelPos.y,
                                     tempScrnPos.x,
                                     tempScrnPos.y);

      newVertes.position = { float(tempScrnPos.x), float(tempScrnPos.y) };
      m_PathLineTail.push_back(newVertes);
    }
    setArrow();
    m_linePathTile = m_linePathTile->getParent();
  }
  m_linePathTile = nullptr;
}

void RTSTiledMap::clearSearch()
{
  m_mapGridVisited.clear();           // Tiles visited
  //m_mapPathRegister.clear();
  m_mapPathRegisterTail.clear();
  m_mapSearchRegister.clear();
  while (!m_pathStack.empty())
  {
    m_pathStack.pop_front();
  }
  m_bSearching = false;
  m_bGetPath = false;
  m_mapGridCopy.clear();
  m_mapGridCopy = m_mapGrid;
  m_dijkstra.clear();
  m_aStar.clear();
  m_BTPathLine.clear();
  m_lastPendiente = { 0,0 };
  m_pCurrentBTTailNode = nullptr;
  m_BresenhammapPathRegisterTail.clear();
  m_BresenhamPathLineTail.clear();
}

void RTSTiledMap::backTracking()
{
  if (nullptr==m_pCurrentBTTailNode)
  {
    m_pCurrentBTTailNode = m_mapPathRegisterTailForBT.front();
    m_mapPathRegisterTailForBT.pop_front();
    //m_BTPathLine.push_back(m_pCurrentBTTailNode);
    m_pCurrentBTTailNode->setcolor(m_BTTileColor);
  }
  if (m_pCurrentBTTailNode->getIndex() == m_tileStartIndexY * m_mapSize.x + m_tileStartIndexX|| 
    m_mapPathRegisterTailForBT.size()<=0)
  {
    //clearSearch();
    for (int i = 0; i < m_BTmapPathRegisterTail.size(); i++)
    {
      m_BTmapPathRegisterTail[i]->setcolor(m_PathTileColor);
    }
    m_BTmapPathRegisterTail.clear();
    m_BTPathLine.push_back(m_pCurrentBTTailNode);
    Vector2I point;
    for (int i = 0; i < m_BTPathLine.size(); i++)
    {
      m_BTPathLine[i]->setcolor(m_BTTileColor);
      
    }
    m_BresenhammapPathRegisterTail = m_BTPathLine;
    if (nullptr != m_selectedUnit)
    {
      //m_archerUnit.m_pathToFollow.clear();
      m_selectedUnit->m_pathToFollow.clear();
      for (int i = m_BTPathLine.size() - 1; i > -1; --i)
      {
        Vector2I tempPoint;
        point = { m_BTPathLine[i]->getIndexGridX(),m_BTPathLine[i]->getIndexGridY() };
        COORDS::getTileCenterOnPixelCoords(point.x, point.y, tempPoint.x, tempPoint.y);
        //m_archerUnit.m_pathToFollow.push_back(Vector2(float(tempPoint.x), float(tempPoint.y)));
        m_selectedUnit->m_pathToFollow.push_back(Vector2(float(tempPoint.x), float(tempPoint.y)));
      }
      //m_archerUnit.m_bHaveObjetive = true;
      m_selectedUnit->m_bHaveObjetive = true;
    }
    m_bGetPathBT = false;
    m_bBresenhamLine = true;
    return;
  }
  Vector2I currentPendiente;
  if (nullptr != m_pCurrentBTTailNode->getParent())
  {
    currentPendiente.x = m_pCurrentBTTailNode->getIndexGridX() - m_pCurrentBTTailNode->getParent()->getIndexGridX();
    currentPendiente.y = m_pCurrentBTTailNode->getIndexGridY() - m_pCurrentBTTailNode->getParent()->getIndexGridY();
  }
  if (m_lastPendiente != currentPendiente)
  {
    for (int i = 0; i < m_BTmapPathRegisterTail.size(); i++)
    {
      m_BTmapPathRegisterTail[i]->setcolor(m_PathTileColor);
    }
    m_BTmapPathRegisterTail.clear();
    m_BTPathLine.push_back(m_pCurrentBTTailNode);
    for (int i = 0; i < m_BTPathLine.size(); i++)
    {
      m_BTPathLine[i]->setcolor(m_BTTileColor);
    }
  }
  m_lastPendiente = currentPendiente; //agregar cosas para detectar la pendiente y la pila de la linea recta
  m_BTmapPathRegisterTail.push_back(m_pCurrentBTTailNode);
  m_pCurrentBTTailNode = m_pCurrentBTTailNode->getParent();
  m_pCurrentBTTailNode->setcolor(m_BTTileColor);
}

void RTSTiledMap::returnBresenhamLinePath()
{
  if (m_BresenhammapPathRegisterTail.size() == 0)
  {
    return;
  }
  int lasTile = 0;
  sf::Vertex newVertes;
  Vector2I tempPixelPos;
  Vector2 tempScrnPos;
  m_finalPath.clear();
  int size = m_BresenhammapPathRegisterTail.size() - 1;
  m_BresenhamPathLine.clear();

  COORDS::getTileCenterOnPixelCoords(m_BresenhammapPathRegisterTail[0]->getIndexGridX(),
    m_BresenhammapPathRegisterTail[0]->getIndexGridY(),
    tempPixelPos.x,
    tempPixelPos.y);
  COORDS::getPixelToScreenCoords(tempPixelPos.x,
    tempPixelPos.y,
    tempScrnPos.x,
    tempScrnPos.y);
  newVertes.position = { float(tempScrnPos.x), float(tempScrnPos.y) };
  m_finalPath.push_back(newVertes);
  m_BresenhamPathLine.push_back(m_BresenhammapPathRegisterTail[0]);
  m_BresenhamPathLine[0]->setParent(nullptr);
  for (int i = size; i > lasTile; --i)
  {
    if (i==lasTile)
    {
      break;
    }
    if (checkForBresenhamLinePath(i, lasTile)) {
      COORDS::getTileCenterOnPixelCoords(m_BresenhammapPathRegisterTail[i]->getIndexGridX(),
        m_BresenhammapPathRegisterTail[i]->getIndexGridY(),
        tempPixelPos.x,
        tempPixelPos.y);
      COORDS::getPixelToScreenCoords(tempPixelPos.x,
        tempPixelPos.y,
        tempScrnPos.x,
        tempScrnPos.y);
      newVertes.position = { float(tempScrnPos.x), float(tempScrnPos.y) };
      m_finalPath.push_back(newVertes);
      m_BresenhamPathLine.push_back(m_BresenhammapPathRegisterTail[i]);
      m_BresenhamPathLine[m_BresenhamPathLine.size() - 1]->setParent(m_BresenhamPathLine[m_BresenhamPathLine.size() - 2]);
      if (i== size)
      {
        break;
      }
      lasTile = i;
      i = size;
    }
  }
  if (nullptr!= m_selectedUnit && m_bBresenhamLine)
  {
    //m_archerUnit.m_pathToFollow.clear();
    m_selectedUnit->m_pathToFollow.clear();
    Vector2I point;
    for (int i = m_BresenhamPathLine.size() - 1; i > -1; --i)
    {
      Vector2I tempPoint;
      point = { m_BresenhamPathLine[i]->getIndexGridX(),m_BresenhamPathLine[i]->getIndexGridY() };
      COORDS::getTileCenterOnPixelCoords(point.x, point.y, tempPoint.x, tempPoint.y);
      //m_archerUnit.m_pathToFollow.push_back(Vector2(float(tempPoint.x), float(tempPoint.y)));
      m_selectedUnit->m_pathToFollow.push_back(Vector2(float(tempPoint.x), float(tempPoint.y)));
    }
    //m_archerUnit.m_bHaveObjetive = true;
    m_selectedUnit->m_bHaveObjetive = true;
  }
  
  m_bBresenhamLine = false;
}

bool RTSTiledMap::checkForBresenhamLinePath(int IndexTileInit,int indexTileFinish )
{
  float m;
  Vector2 linea;
  int dx;
  int dy;
  int x;
  int x1;
  int y;
  int y1;

  
  MapTile* tmpTileA = m_BresenhammapPathRegisterTail[indexTileFinish];
  MapTile* tmpTileB = m_BresenhammapPathRegisterTail[IndexTileInit];
  linea = { float(tmpTileB->getIndexGridX()) - float(tmpTileA->getIndexGridX()),
            float(tmpTileB->getIndexGridY()) - float(tmpTileA->getIndexGridY()) };
  dx = linea.x;
  dy = linea.y;
  x = tmpTileA->getIndexGridX();
  x1 = tmpTileB->getIndexGridX();
  y = tmpTileA->getIndexGridY();
  y1 = tmpTileB->getIndexGridY();
  int p = 2 * dy - dx;
  m = linea.size();
  if (dx > dy)
  {
    if (x < x1)
    {
      if (y < y1)
      {
        p = 2 * dy - dx;
      }
      else
      {
        p = 2 * (-dy) - dx;
      }
      while (x < x1 || y != y1)
      {
        if (p >= 0)
        {
          if (y < y1)
          {
            y = y + 1;
            if (y > m_mapSize.y)
            {
              y > m_mapSize.y;
            }
            p = p + 2 * (dy)-2 * dx;
          }
          else
          {
            y = y - 1;
            if (y < 0)
            {
              y = 0;
            }
            p = p + 2 * (-dy) - 2 * dx;
          }
        }
        else
        {
          if (y < y1)
          {
            p = p + 2 * dy;
          }
          else
          {
            p = p + 2 * (-dy);
          }

        }
        x = x + 1;
        if (x > x1)
        {
          x = x1;
        }
        if (nullptr!= m_selectedUnit)
        {
          if (!checkCanAddTile(m_selectedUnit->m_unitType, &m_mapGrid[y*m_mapSize.x + x]))
          {
            return false;
          }
        }
      }
    }
    else
    {
      if (y < y1)
      {
        p = 2 * dy - dx;
      }
      else
      {
        p = 2 * (-dy) - dx;
      }
      while (x > x1 || y != y1)
      {
        if (p >= 0)
        {
          if (y < y1)
          {
            y = y + 1;
            if (y > m_mapSize.y)
            {
              y > m_mapSize.y;
            }
            p = p + 2 * (dy)+2 * dx;
          }
          else
          {
            y = y - 1;
            if (y < 0)
            {
              y = 0;
            }
            p = p + 2 * (-dy) - 2 * dx;
          }
        }
        else
        {
          if (y < y1)
          {
            p = p + 2 * dy;
          }
          else
          {
            p = p + 2 * (-dy);
          }
        }
        x = x - 1;
        if (x < x1)
        {
          x = x1;
        }
        if (nullptr != m_selectedUnit)
        {
          if (!checkCanAddTile(m_selectedUnit->m_unitType, &m_mapGrid[y*m_mapSize.x + x]))
          {
            return false;
          }
        }
      }
    }
  }
  else
  {

    if (y < y1)
    {
      if (x < x1)
      {
        p = 2 * dx - dy;
      }
      else
      {
        p = 2 * (-dx) - dy;
      }
      while (y < y1 || x != x1)
      {
        if (p >= 0)
        {
          if (x < x1)
          {
            x = x + 1;
            if (x > m_mapSize.x)
            {
              x > m_mapSize.x;
            }
            p = p + 2 * (dx)-2 * dy;
          }
          else
          {
            x = x - 1;
            if (x < 0)
            {
              x = 0;
            }
            p = p + 2 * (-dx) - 2 * dy;
          }
        }
        else
        {
          if (x < x1)
          {
            p = p + 2 * dx;
          }
          else
          {
            p = p + 2 * (-dx);
          }

        }
        y = y + 1;
        if (y > y1)
        {
          y = y1;
        }
        if (nullptr != m_selectedUnit)
        {
          if (!checkCanAddTile(m_selectedUnit->m_unitType, &m_mapGrid[y*m_mapSize.x + x]))
          {
            return false;
          }
        }
      }
    }
    else
    {
      if (x < x1)
      {
        p = 2 * dx - dy;
      }
      else
      {
        p = 2 * (-dx) - dy;
      }
      while (y > y1 || x != x1)
      {
        if (p >= 0)
        {
          if (x < x1)
          {
            x = x + 1;
            if (x > m_mapSize.x)
            {
              x > m_mapSize.x;
            }
            p = p + 2 * (dx)+2 * dy;
          }
          else
          {
            x = x - 1;
            if (x < 0)
            {
              x = 0;
            }
            p = p + 2 * (-dx) - 2 * dy;
          }
        }
        else
        {
          if (x < x1)
          {
            p = p + 2 * dx;
          }
          else
          {
            p = p + 2 * (-dx);
          }
        }
        y = y - 1;
        if (y < y1)
        {
          y = y1;
        }
        if (nullptr != m_selectedUnit)
        {
          if (!checkCanAddTile(m_selectedUnit->m_unitType, &m_mapGrid[y*m_mapSize.x + x]))
          {
            return false;
          }
        }
      }
    }
  }
  
  return true;
}



void RTSTiledMap::updateTileColor()
{
  if (nullptr!=m_selectedUnit)
  {
    m_tileStart = &m_mapGridCopy[m_selectedUnit->m_tileIndexY *m_mapSize.x + m_selectedUnit->m_tileIndexX];
    m_tileStartIndexX = m_selectedUnit->m_tileIndexX;
    m_tileStartIndexY = m_selectedUnit->m_tileIndexY;
  }
  if (nullptr != m_tileStart)
  {
    m_tileStart->setcolor(m_tileStartColor);
  }
  if (nullptr != m_tileFinish)
  {
    m_tileFinish->setcolor(m_tileFinishColor);
  }
}

void RTSTiledMap::deleteObjectInTile(Object * _object, int32 tilex, int32 tiley)
{
  if (tiley < 0 || tilex > m_mapSize.x || tiley < 0 || tiley > m_mapSize.x)
  {
    return;
  }
  MapTile* tmpTile = &m_mapGrid[tiley*m_mapSize.x + tilex];

  for (int i = 0; i < tmpTile->m_myObject.size(); i++)
  {
    if (tmpTile->m_myObject[i]==_object)
    {
      tmpTile->m_myObject.erase(tmpTile->m_myObject.begin() + i);
      return;
    }
  }
}

void RTSTiledMap::SetInfluenceInTile(int32 tilex, int32 tiley, const float& influence)
{
  if (tiley < 0 || tilex > m_mapSize.x || tiley < 0 || tiley > m_mapSize.x)
  {
    return;
  }
  m_mapGrid[tiley*m_mapSize.x + tilex].m_influence= influence;
}

void RTSTiledMap::insetObjectInTile(Object * _object, int32 tilex, int32 tiley)
{
  if (tiley < 0 || tilex > m_mapSize.x || tiley < 0 || tiley > m_mapSize.x)
  {
    return;
  }
  MapTile* tmpTile = &m_mapGrid[tiley*m_mapSize.x + tilex];
  tmpTile->addObject(_object);
}

void
RTSTiledMap::update(float deltaTime) {
  GE_UNREFERENCED_PARAMETER(deltaTime);
  m_currenttimeToNext += deltaTime;
  m_timeToUpdateIM += deltaTime;
  m_momentumPerSecond = m_momentum;
  //m_decayPerSecond = m_decay * deltaTime;
  updateTileColor();
  
  if (m_currenttimeToNext > m_timeToNext)
  {
    if (m_bSearching)
    {
      switch (m_ePathFinding)
      {
      case DEPTH:
        //DepthFirstSearch();
        if (nullptr == m_selectedUnit)
        {
          DepthFirstSearchWithSteps(m_stepToDo, UNITTYPE::KFLYING);
        }
        else
        {
          DepthFirstSearchWithSteps(m_stepToDo, m_selectedUnit->m_unitType);
        }
        break;
      case BREADTH:
        //BreadthFirstSearch();
        if (nullptr == m_selectedUnit)
        {
          BreadthFirstSearchWithSteps(m_stepToDo, UNITTYPE::KFLYING);
        }
        else
        {
          BreadthFirstSearchWithSteps(m_stepToDo, m_selectedUnit->m_unitType);
        }
        break;
      case BEST:
        //BestFirstSearch();
        if (nullptr == m_selectedUnit)
        {
          BestFirstSearchWithSteps(m_stepToDo, UNITTYPE::KFLYING);
        }
        else
        {
          BestFirstSearchWithSteps(m_stepToDo, m_selectedUnit->m_unitType);
        }
        break;
      case DIJKSTRA:
        //DijkstraSearch();
        if (nullptr == m_selectedUnit)
        {
          DijkstraSearchWithSteps(m_stepToDo,UNITTYPE::KFLYING);
        }
        else
        {
          DijkstraSearchWithSteps(m_stepToDo, m_selectedUnit->m_unitType);
        }
        break;
      case ASTAR:
        //AstarSearch();
        if (nullptr == m_selectedUnit)
        {
          AstarSearchWithSteps(m_stepToDo, UNITTYPE::KFLYING);
        }
        else
        {
          AstarSearchWithSteps(m_stepToDo, m_selectedUnit->m_unitType);
        }
        break;
      default:
        break;
      }
    }
    if (m_bGetPath)
    {
      //BreadthFirstSearchReturnPath();
      returnPath();
    }
    if (m_bGetPathBT)
    {
      backTracking();
    }
    //if (m_bBresenhamLine)
    //{
    //  returnBresenhamLinePath();
    //}
    m_currenttimeToNext = 0.0f;
  }
  returnLinePath();
  returnBresenhamLinePath();
  for (int i = 0; i < m_mapGrid.size(); ++i)
  {
    for (int j = 0; j < m_mapGrid[i].m_myObject.size(); ++j) {
      Object *tmpObject = m_mapGrid[i].m_myObject[j];
      RTSUnit *tmpUnit;
      RTSTree *tmpTree;
      switch (tmpObject->getType())
      {
      case TYPEOBJECT::UNIT:
        tmpUnit = static_cast<RTSUnit*>(tmpObject);
        tmpUnit->onUpdate(deltaTime,*this);
        break;
      default:
        break;
      }
    }
  }
  //m_archerUnit.onUpdate(deltaTime,*this);
#ifndef NDEBUG
  if (0.1<m_timeToUpdateIM)
  {
    for (int i = 0; i < m_mapGrid.size(); i++)
    {
      propagateInfluence(i);
    }
    m_timeToUpdateIM = 0;
  }
#endif
  //propagateInfluence(m_momentum,m_decay);
  
}

void
RTSTiledMap::render() {
  int32 tmpX = 0;
  int32 tmpY = 0;
  int32 tmpTypeTile = 0;
  Vector2I clipRect;

  int32 tileIniX = 0, tileIniY = 0;
  int32 tileFinX = 0, tileFinY = 0;

  if (GameOptions::s_MapIsIsometric)
  {
    int32 trashCoord = 0;
    getScreenToMapCoords(m_scrStart.x, m_scrStart.y, tileIniX, trashCoord);
    getScreenToMapCoords(m_scrEnd.x, m_scrEnd.y, tileFinX, trashCoord);

    getScreenToMapCoords(m_scrEnd.x, m_scrStart.y, trashCoord, tileIniY);
    getScreenToMapCoords(m_scrStart.x, m_scrEnd.y, trashCoord, tileFinY);
  }
  else
  {
    getScreenToMapCoords(m_scrStart.x, m_scrStart.y, tileIniX, tileIniY);
    getScreenToMapCoords(m_scrEnd.x, m_scrEnd.y, tileFinX, tileFinY);
  }
  sf::Vector2f mousePosition = getMousePosOnRenderTarget();
  m_tileSelectedIndex = -1;
  int32 tailSelectedY = -1;
  int32 iterTailSelectedY = -1;
  int32 tailSelectedX = -1;
  int32 iterTailSelectedX = -1;

  for (int32 iterX = tileIniX; iterX <= tileFinX; ++iterX) {
    for (int32 iterY = tileIniY; iterY <= tileFinY; ++iterY) {

      getMapToScreenCoords(iterX, iterY, tmpX, tmpY);
      if (tmpX > m_scrEnd.x ||
        tmpY > m_scrEnd.y ||
        (tmpX + GameOptions::s_TileSizeX) < m_scrStart.x ||
        (tmpY + GameOptions::s_TileSizeY) < m_scrStart.y) {
        continue;
      }
      int currentIndex= (iterY*m_mapSize.x) + iterX;
      //propagateInfluence(currentIndex);
      //m_bTileDrawn[currentIndex] = true;
      if (tmpX < mousePosition.x&&
         tmpX + GameOptions::s_TileSizeX > mousePosition.x&&
         tmpY  < mousePosition.y&&
         tmpY + GameOptions::s_TileSizeY > mousePosition.y)
       {
         m_tileSelectedIndexY = iterTailSelectedY = iterY;
         m_tileSelectedIndexX = iterTailSelectedX = iterX;
         tailSelectedY = tmpY + GameOptions::TILEHALFSIZE.y;
         tailSelectedX = tmpX + GameOptions::TILEHALFSIZE.x;
         m_tileSelectedIndex = currentIndex;
       }
       m_mapGridCopy[currentIndex].setPosition(
        static_cast<float>( tmpX ),
        static_cast<float>(tmpY));
       tmpTypeTile = m_mapGrid[currentIndex].getType();
       RTSTexture& refTexture = m_mapTextures[tmpTypeTile];

       clipRect.x = (iterX << GameOptions::BITSHFT_TILESIZE.x) % refTexture.getWidth();
       clipRect.y = (iterY << GameOptions::BITSHFT_TILESIZE.y) % refTexture.getHeight();
       
       refTexture.setPosition(tmpX, tmpY);
       sf::Color tmpColor;
       if (!m_bShowInfluenceMap)
       {
         if (m_bShowZones && TERRAIN_TYPE::kObstacle != m_mapGrid[currentIndex].getType())
         {
           if (m_mapGrid[currentIndex].m_imLimit)
           {
             tmpColor = m_mapGrid[currentIndex].m_myZone->m_colorOfLimit;
           }
           else
           {
             tmpColor = m_mapGrid[currentIndex].m_myZone->m_colorOfZone;
           }
           tmpColor.a = 255;
         }
         else
         {
           tmpColor = m_mapGridCopy[currentIndex].getColor();
         }
       }
       else
       {
         float influence = m_mapGrid[currentIndex].m_influence;
         if (influence>0.0000000000f)
         {
           tmpColor = sf::Color(0,0,(influence)*255 ,255);
         }
         else
         {
           influence *= -1;
           tmpColor = sf::Color((influence)*255,0,0 ,255);
         }
       }
       refTexture.setColor(tmpColor.r, tmpColor.g, tmpColor.b, tmpColor.a);
       refTexture.setSrcRect(clipRect.x, clipRect.y, GameOptions::s_TileSizeX, GameOptions::s_TileSizeY);
       //m_mapGridCopy[currentIndex].m_bDrawing = true;
       
       refTexture.draw();
       //for (int i = 0; i < m_mapGrid[currentIndex].m_myObject.size(); i++) {
       //  m_mapGrid[currentIndex].m_myObject[i]->drawObject(m_textureObjects, tmpX, tmpY);
       //}
    }
  }

  for (int32 iterX = tileIniX; iterX <= tileFinX; ++iterX) {
    for (int32 iterY = tileIniY; iterY <= tileFinY; ++iterY) {

      getMapToScreenCoords(iterX, iterY, tmpX, tmpY);
      if (tmpX > m_scrEnd.x ||
        tmpY > m_scrEnd.y ||
        (tmpX + GameOptions::s_TileSizeX) < m_scrStart.x ||
        (tmpY + GameOptions::s_TileSizeY) < m_scrStart.y) {
        continue;
      }
      int currentIndex = (iterY*m_mapSize.x) + iterX;
      for (int i = 0; i < m_mapGrid[currentIndex].m_myObject.size(); i++) {
        Object *tmpObject = m_mapGrid[currentIndex].m_myObject[i];
        RTSUnit *tmpUnit;
        RTSTree *tmpTree;
        switch (tmpObject->getType())
        {
        case TYPEOBJECT::UNIT:
          tmpUnit = static_cast<RTSUnit*>(tmpObject);
          tmpUnit->draw();
          break;
        case TYPEOBJECT::TREE:
          tmpTree = static_cast<RTSTree*>(tmpObject);
          tmpTree->draw();
          break;
        default:
          break;    
        }
      }
      if (m_mapGridCopy[currentIndex].hadArrow)
      {
        RTSTexture& refTexture = m_mapTextures[TERRAIN_TYPE::kArrow];

        clipRect.x = (iterX << GameOptions::BITSHFT_TILESIZE.x) % refTexture.getWidth();
        clipRect.y = (iterY << GameOptions::BITSHFT_TILESIZE.y) % refTexture.getHeight();

        refTexture.setPosition(tmpX+ GameOptions::TILEHALFSIZE.x, tmpY+ GameOptions::TILEHALFSIZE.y);
        refTexture.setOrigin(refTexture.getWidth()*0.5f, refTexture.getWidth()*0.5f);
        switch (m_mapGridCopy[currentIndex].m_directionArrow)    
        { 
        case ARROW_TYPE::kRight:
          refTexture.setRotation(Degree(0.0f));
          break;
        case ARROW_TYPE::kRightDown:
          refTexture.setRotation(Degree(45));
          break;
        case ARROW_TYPE::kDown:
          refTexture.setRotation(Degree(90));
          break;
        case ARROW_TYPE::kLeftDown:
          refTexture.setRotation(Degree(135));
          break;
        case ARROW_TYPE::kLeft:
          refTexture.setRotation(Degree(180));
          break;
        case ARROW_TYPE::kLeftUp:
          refTexture.setRotation(Degree(225));
          break;
        case ARROW_TYPE::kUp:
          refTexture.setRotation(Degree(270));
          break;
        case ARROW_TYPE::kRightUp:
          refTexture.setRotation(Degree(315));
          break;
        default:
          break;
        }
        refTexture.draw();
      }
    }
  }

  if (GameOptions::s_MapShowGrid) {
    FrameVector<sf::Vertex> gridLines;
    gridLines.reserve( ((tileFinX - tileIniX) + (tileFinY - tileIniY) + 4) << 1);
    
    sf::Color gridColor(255, 0, 0, 255);

    int32 tmpX2 = 0, tmpY2 = 0;
    for (int32 iterX = tileIniX; iterX <= tileFinX + 1; ++iterX) {
      getMapToScreenCoords(iterX, tileIniY, tmpX, tmpY);
      getMapToScreenCoords(iterX, tileFinY, tmpX2, tmpY2);
      if (GameOptions::s_MapIsIsometric)
      {
        gridLines.push_back(sf::Vertex(
          sf::Vector2f(static_cast<float>(tmpX + GameOptions::TILEHALFSIZE.x),
            static_cast<float>(tmpY)),
          gridColor));

        gridLines.push_back(sf::Vertex(
          sf::Vector2f(static_cast<float>(tmpX2),
            static_cast<float>(tmpY2 + GameOptions::TILEHALFSIZE.y)),
          gridColor));
      }
      else 
      {
        gridLines.push_back(sf::Vertex(
          sf::Vector2f(static_cast<float>(tmpX), static_cast<float>(tmpY)),
          gridColor));

        gridLines.push_back(sf::Vertex(sf::Vector2f(static_cast<float>(tmpX2),
                                                    static_cast<float>(tmpY2 + GameOptions::s_TileSizeY)),
                                                    gridColor));
      }
    }

    for (int32 iterY = tileIniY; iterY <= tileFinY + 1; ++iterY) {
      getMapToScreenCoords(tileIniX, iterY, tmpX, tmpY);
      getMapToScreenCoords(tileFinX, iterY, tmpX2, tmpY2);
      if (GameOptions::s_MapIsIsometric) 
      {
        gridLines.push_back(sf::Vertex(
          sf::Vector2f(static_cast<float>(tmpX + GameOptions::TILEHALFSIZE.x),
                       static_cast<float>(tmpY)),
                       gridColor));

        gridLines.push_back(sf::Vertex(
          sf::Vector2f(static_cast<float>(tmpX2 + GameOptions::s_TileSizeX),
                       static_cast<float>(tmpY2 + GameOptions::TILEHALFSIZE.y)),
                       gridColor));
      }
      else
      {
        gridLines.push_back(sf::Vertex(
          sf::Vector2f(static_cast<float>(tmpX), static_cast<float>(tmpY)),
                       gridColor));

        gridLines.push_back(sf::Vertex(sf::Vector2f(static_cast<float>(tmpX2 + GameOptions::s_TileSizeX),
                                       static_cast<float>(tmpY2)),
                                       gridColor));
      }
    }

    m_pTarget->draw(&gridLines[0], gridLines.size(), sf::Lines);
  }
  if (m_PathLineTail.size()>0)
  {
    //m_pTarget->draw(&m_PathLineTail[0], m_PathLineTail.size(), sf::LinesStrip);
  }
  if (m_BresenhamPathLineTail.size() > 0)
  {
    //m_pTarget->draw(&m_BresenhamPathLineTail[0], m_BresenhamPathLineTail.size(), sf::LinesStrip);
  }
  if (m_finalPath.size() > 0)
  {
    m_pTarget->draw(&m_finalPath[0], m_finalPath.size(), sf::LinesStrip);
  }
  
  {
    if (nullptr != m_selectedUnit)
    {
      drawTailOutline(m_tileSelectedIndexX,m_tileSelectedIndexY,m_selectingTileColor);
    }

    m_mouseInWindow.setPosition(mousePosition.x, mousePosition.y);
    m_pTarget->draw(m_mouseInWindow);
    if (m_tileSelectedIndex >= 0)
    {
      drawTailOutline(m_tileSelectedIndexX,m_tileSelectedIndexY,m_selectingTileColor);
    }
  }
}

RTSTiledMap::MapTile::MapTile() {
  m_idType = 1;
  m_cost = 1;
}

RTSTiledMap::MapTile::MapTile(const int8 idType, const int8 cost) {
  m_idType = idType;
  m_cost = cost;
}

RTSTiledMap::MapTile::MapTile(const MapTile& copy) {
  m_idType = copy.m_idType;
  m_cost = copy.m_cost;
}

RTSTiledMap::MapTile&
RTSTiledMap::MapTile::operator=(const MapTile& rhs) {
  m_idType = rhs.m_idType;
  m_cost = rhs.m_cost;
  return *this;
}

bool
RTSTiledMap::loadFromImageFile(sf::RenderTarget* pTarget, String fileName) {
  sf::Image image;
  m_bAllMapWithZones = false;
  for (int i = 0; i < m_mapZones.size(); ++i)
  {
    m_mapZones[i].m_myTiles.clear();
    m_mapZones[i].m_myLimits.clear();
  }
  m_mapZones.clear();
  if (!image.loadFromFile(fileName.c_str())) {
    LOGWRN("File not found: " + fileName);
    return false;
  }
  
  if (!init(pTarget, Vector2I(image.getSize().x, image.getSize().y))) {
    LOGERR("Failed initializing map data");
    return false;
  }

  //With this library, the pixels always are 4 bytes in length
  int32 lineBytes = image.getSize().x * 4;
  const uint8* pPixeles = reinterpret_cast<const uint8*>(image.getPixelsPtr());

  for (int32 tmpY = 0; tmpY < m_mapSize.y; ++tmpY) {
    for (int32 tmpX = 0; tmpX < m_mapSize.x; ++tmpX) {
      uint8 tipoTerreno = TERRAIN_TYPE::kObstacle;
      Color actualColor(
        pPixeles[(tmpY * lineBytes) + (tmpX * 4) + 0],
        pPixeles[(tmpY * lineBytes) + (tmpX * 4) + 1],
        pPixeles[(tmpY * lineBytes) + (tmpX * 4) + 2],
        pPixeles[(tmpY * lineBytes) + (tmpX * 4) + 3]);

      if (Color::Blue == actualColor) {
        tipoTerreno = TERRAIN_TYPE::kWater;
      }
      else if (Color::Green == actualColor) {
        tipoTerreno = TERRAIN_TYPE::kGrass;
      }
      else if (Color::Yellow == actualColor) {
        tipoTerreno = TERRAIN_TYPE::kMarsh;
      }

      setType(tmpX, tmpY, tipoTerreno);
    }
  }

  //setTileCost();
  for (int i = 0; i < m_mapZones.size(); ++i)
  {
    m_mapZones[i].m_myTiles.clear();
    m_mapZones[i].m_myLimits.clear();
  }
  m_mapZones.clear();
  for (int i = 0; i < m_mapGrid.size(); i++)
  {
    m_mapGrid[i].m_myZone = nullptr;
  }
  m_bAllMapWithZones = false;
  DijkstraSearchForZones(m_StepsToZone);
  for (int i = 0; i < m_mapZones.size(); i++)
  {
    for (int j = 0; j < m_mapZones[i].m_myTiles.size(); j++)
    {
      m_mapGrid[m_mapZones[i].m_myTiles[j]->getIndex()].m_myZone = &m_mapZones[i];
      m_mapZones[i].m_myTiles[j]->m_myZone = &m_mapZones[i];
    }
  }
  checkLimitOfzones();
  actual = nullptr;
  return true;
}

bool
RTSTiledMap::saveToImageFile(sf::RenderTarget*, String) {
  return false;
}

bool RTSTiledMap::CLessWeight::operator()(MapTile * lhs, MapTile * rhs) const
{
  return lhs->getTentativeCost() < rhs->getTentativeCost();;
}

bool RTSTiledMap::CLessF::operator()(MapTile * pA, MapTile * pB) const
{
  //////CGraph* pGraph = pA->GetGraph();
  float hA = pA->getEuristic();
  float hB = pB->getEuristic();
  return (hA + pA->getTentativeCost()) < (hB + pB->getTentativeCost());
}
