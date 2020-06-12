#include "RTSTiledMap.h"
#include "RTSTexture.h"

#include <geDebug.h>
#include <geColor.h>

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
}

bool
RTSTiledMap::init(sf::RenderTarget* pTarget, const Vector2I& mapSize) {
  if (m_mapGrid.size()) {
    destroy();
  }

  m_pTarget = pTarget;

  m_mapGrid.resize(mapSize.x * mapSize.y);
  m_mapGridCopy = m_mapGrid;
  m_mapGridVisited.resize(mapSize.x * mapSize.y);
  m_mapSize = mapSize;
  setCameraStartPosition(0, 0);

  m_mapTextures.resize(TERRAIN_TYPE::kNumObjects);

  String textureName;
  for (uint32 i = 0; i < TERRAIN_TYPE::kNumObjects; ++i) {
#ifdef MAP_IS_ISOMETRIC
    textureName = "Textures/Terrain/iso_terrain_" + toString(i) + ".png";
#else
    textureName = "Textures/Terrain/terrain_" + toString(i) + ".png";
#endif
    m_mapTextures[i].loadFromFile(m_pTarget, textureName);
  }
  //To see a mouse position on render target
  m_mouseInWindow.setFillColor(sf::Color::Magenta);
  m_mouseInWindow.setRadius(4);
  m_mouseInWindow.setOrigin(4, 4);
  m_outLineTail.reserve(5);
 
  preCalc();

  return true;
}

void
RTSTiledMap::destroy() {
  m_mapGrid.clear();
  m_mapTextures.clear();

  m_mapSize = Vector2I::ZERO;
  setCameraStartPosition(0, 0);
  preCalc();
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
  m_iCamera = tmpPos;

#ifdef MAP_IS_ISOMETRIC
  m_PreCalc_ScreenDeface.x = m_scrStart.x + m_PreCalc_MidResolution.x -
                               (m_iCamera.x - m_iCamera.y);
  m_PreCalc_ScreenDeface.y = m_scrStart.y + m_PreCalc_MidResolution.y -
                               ((m_iCamera.x + m_iCamera.y) >> 1);
#else
  m_PreCalc_ScreenDeface = m_scrStart + m_PreCalc_MidResolution - m_iCamera;
#endif
}

void
RTSTiledMap::getScreenToMapCoords(const int32 scrX,
                                  const int32 scrY,
                                  int32 &mapX,
                                  int32 &mapY) {
#ifdef MAP_IS_ISOMETRIC
  float fscrX = ((float)(scrX - m_PreCalc_ScreenDeface.x) / GameOptions::TILEHALFSIZE.x) - 1;
  float fscrY = ((float)(scrY - m_PreCalc_ScreenDeface.y) / GameOptions::TILEHALFSIZE.y);

  mapX = (Math::trunc(fscrX + fscrY)) >> 1;
  mapY = (Math::trunc(fscrY - fscrX)) >> 1;
#else
  mapX = (scrX - m_PreCalc_ScreenDeface.x) >> GameOptions::BITSHFT_TILESIZE.x;
  mapY = (scrY - m_PreCalc_ScreenDeface.y) >> GameOptions::BITSHFT_TILESIZE.y;
#endif

  mapX = Math::clamp(mapX, 0, m_mapSize.x - 1);
  mapY = Math::clamp(mapY, 0, m_mapSize.y - 1);
}

void
RTSTiledMap::getMapToScreenCoords(const int32 mapX,
                                  const int32 mapY,
                                  int32 &scrX,
                                  int32 &scrY) {
  GE_ASSERT(mapX >= 0 && mapX <= m_mapSize.x && mapY >= 0 && mapY <= m_mapSize.y);

#ifdef MAP_IS_ISOMETRIC
  scrX = (mapX - mapY) << GameOptions::BITSFHT_TILEHALFSIZE.x;
  scrY = (mapX + mapY) << GameOptions::BITSFHT_TILEHALFSIZE.y;

  scrX += m_PreCalc_ScreenDeface.x;
  scrY += m_PreCalc_ScreenDeface.y;
#else
  scrX = (mapX << GameOptions::BITSHFT_TILESIZE.x) + m_PreCalc_ScreenDeface.x;
  scrY = (mapY << GameOptions::BITSHFT_TILESIZE.y) + m_PreCalc_ScreenDeface.y;
#endif
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
    m_tileStart = &m_mapGridCopy[m_tileSelectedIndex];
    m_tileStart->setcolor(m_tileStartColor);
    m_tileStartIndexX = m_tileSelectedIndexX;
    m_tileStartIndexY = m_tileSelectedIndexY;
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
  m_bSearching = true;

  int initIndex = m_tileStartIndexY * m_mapSize.x + m_tileStartIndexX;
  if (PATHFINDING::BEST==m_ePathFinding || PATHFINDING::ASTAR == m_ePathFinding)
  {
    switch (m_eEuristicType)
    {
    case EUCLIDEAN:
      m_euritic = Vector2::distance(Vector2(m_tileStartIndexX, m_tileStartIndexY),
        Vector2(m_tileFinishIndexX, m_tileFinishIndexY));
      break;
    case SQUAREDISTANCE:
      m_euritic = Vector2::distSquared(Vector2(m_tileStartIndexX, m_tileStartIndexY),
        Vector2(m_tileFinishIndexX, m_tileFinishIndexY));
      break;
    case MANHATTAN:
      break;
    }
    m_mapGridCopy[initIndex].setEuristic(m_euritic*m_euristicRelevance);
  }
 
  m_mapGridCopy[initIndex].setIndex(initIndex);
  m_mapGridCopy[initIndex].setIndexOnGrid(m_tileStartIndexX, m_tileStartIndexY);
  m_mapGridCopy[initIndex].setTentativeCost(0);
  m_mapGridCopy[initIndex].setCost(0);
  actual = &m_mapGridCopy[initIndex];
  if (PATHFINDING::DIJKSTRA != m_ePathFinding || PATHFINDING::ASTAR != m_ePathFinding)
  {
    m_pathStack.push_front(&m_mapGridCopy[initIndex]);
  }
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
    case 0:
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setType(0);
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setCost(3);
      break;
    case 1:
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setType(1);
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setCost(1);
      break;
    case 2:
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setType(2);
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setCost(5);
      break;
    case 3:
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setType(3);
      m_mapGrid[m_tileSelectedIndexY*m_mapSize.x + m_tileSelectedIndexX].setCost(20);
      break;
    default:
      break;
    }
    m_mapGridCopy.clear();
    m_mapGridCopy = m_mapGrid;
  }
}

void RTSTiledMap::drawTailOutline(const int & tailIndex,const sf::Color & outlineColor)
{
  sf::Vector2f tailPos = m_mapGridCopy[tailIndex].getPosition();
  if (m_outLineTail.size()<=0)
  {
    m_outLineTail.resize(5);
  }
#ifdef MAP_IS_ISOMETRIC
  m_outLineTail[0].position.x = tailPos.x - GameOptions::TILEHALFSIZE.x;
  m_outLineTail[0].position.y = tailPos.y;
  m_outLineTail[1].position.x = tailPos.x;
  m_outLineTail[1].position.y = tailPos.y - GameOptions::TILEHALFSIZE.y;
  m_outLineTail[2].position.x = tailPos.x + GameOptions::TILEHALFSIZE.x;
  m_outLineTail[2].position.y = tailPos.y;
  m_outLineTail[3].position.x = tailPos.x;
  m_outLineTail[3].position.y = tailPos.y + GameOptions::TILEHALFSIZE.y;
#else

  m_outLineTail[0].position.x = tailPos.x - GameOptions::TILEHALFSIZE.x;
  m_outLineTail[0].position.y = tailPos.y + GameOptions::TILEHALFSIZE.y;
  m_outLineTail[1].position.x = tailPos.x + GameOptions::TILEHALFSIZE.x;
  m_outLineTail[1].position.y = tailPos.y + GameOptions::TILEHALFSIZE.y;
  m_outLineTail[2].position.x = tailPos.x + GameOptions::TILEHALFSIZE.x;
  m_outLineTail[2].position.y = tailPos.y - GameOptions::TILEHALFSIZE.y;
  m_outLineTail[3].position.x = tailPos.x - GameOptions::TILEHALFSIZE.y;
  m_outLineTail[3].position.y = tailPos.y - GameOptions::TILEHALFSIZE.y;
#endif
  m_outLineTail[0].color = outlineColor;
  m_outLineTail[1].color = outlineColor;
  m_outLineTail[2].color = outlineColor;
  m_outLineTail[3].color = outlineColor;
  m_outLineTail[4] = m_outLineTail[0];
  m_pTarget->draw(&m_outLineTail[0], 5, sf::PrimitiveType::LinesStrip);
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
        TERRAIN_TYPE::kObstacle != m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].getType()) {  // Revisamos que no esté visitado y que no sea pared
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX]);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndex(nuevoy *m_mapSize.x + nuevox);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndexOnGrid(nuevox, nuevoy);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
        m_pathStack.push_back(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);
      }
    }
  }
}

void RTSTiledMap::DepthFirstSearch()
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
        TERRAIN_TYPE::kObstacle != m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].getType()) {  // Revisamos que no esté visitado y que no sea pared
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX]);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndex(nuevoy *m_mapSize.x + nuevox);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndexOnGrid(nuevox, nuevoy);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
        m_pathStack.push_back(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);
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
        TERRAIN_TYPE::kObstacle !=m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].getType()) {  // Revisamos que no esté visitado y que no sea pared
        float eu= Vector2::distance(Vector2(nuevox, nuevoy), Vector2(m_tileFinishIndexX, m_tileFinishIndexY));
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX]);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndex(nuevoy *m_mapSize.x + nuevox);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndexOnGrid(nuevox, nuevoy);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setEuristic(eu);
        m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
        Nodes.push_back(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);
      }
    }
  }
  int n = Nodes.size();
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      if (Nodes[i]->getEuristic() > Nodes[j]->getEuristic())
      {
        auto temp = Nodes[i];
        Nodes[i] = Nodes[j];
        Nodes[j] = temp;
      }
    }
  }
  for (int i = 0; i < n; i++)
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

void RTSTiledMap::AstarSearch()
{ 
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
          float eu = Vector2::distance(Vector2(nuevox, nuevoy), Vector2(m_tileFinishIndexX, m_tileFinishIndexY));
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
  if (m_pCurrentTailNode ==nullptr || m_pCurrentTailNode->getIndex()==m_tileStartIndexY*m_mapSize.x+m_tileStartIndexX)
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
    if (m_mapGridCopy[m_linePathTile->getIndex()].m_bDrawing = true)
    {
      sf::Vertex newVertes;
      newVertes.position = m_linePathTile->getPosition();
      m_PathLineTail.push_back(newVertes);
    }
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
}

void RTSTiledMap::backTracking()
{
  if (nullptr==m_pCurrentBTTailNode)
  {
    m_pCurrentBTTailNode = m_mapPathRegisterTailForBT.front();
    m_mapPathRegisterTailForBT.pop_front();
    m_BTPathLine.push_back(m_pCurrentBTTailNode);
    m_pCurrentBTTailNode->setcolor(m_BTTileColor);
  }
  if (m_pCurrentBTTailNode->getIndex() == m_tileStartIndexY * m_mapSize.x + m_tileStartIndexX|| m_mapPathRegisterTailForBT.size()<=0)
  {
    //clearSearch();
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
    m_bGetPathBT = false;
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

void RTSTiledMap::updateTileColor()
{
  //if (nullptr != m_tileSelected)
  //{
  //  m_tileSelected->setcolor(m_NormalTileColor.r, m_NormalTileColor.g, m_NormalTileColor.b, m_NormalTileColor.a);
  //}
  //if (m_tileSelectedIndex >= 0)
  //{
  //  m_tileSelected = &m_mapGridCopy[m_tileSelectedIndex];
  //  m_tileSelected->setcolor(m_selectingTileColor.r, m_selectingTileColor.g, 
  //                           m_selectingTileColor.b, m_selectingTileColor.a);
  //}
  if (nullptr != m_tileStart)
  {
    m_tileStart->setcolor(m_tileStartColor);
  }
  if (nullptr != m_tileFinish)
  {
    m_tileFinish->setcolor(m_tileFinishColor);
  }
}

void
RTSTiledMap::update(float deltaTime) {
  GE_UNREFERENCED_PARAMETER(deltaTime);
  m_currenttimeToNext += deltaTime;
  updateTileColor();
  if (m_currenttimeToNext > m_timeToNext)
  {
    if (m_bSearching)
    {
      switch (m_ePathFinding)
      {
      case DEPTH:
        DepthFirstSearch();
        break;
      case BREADTH:
        BreadthFirstSearch();
        break;
      case BEST:
        BestFirstSearch();
        break;
      case DIJKSTRA:
        DijkstraSearch();
        break;
      case ASTAR:
        AstarSearch();
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
    m_currenttimeToNext = 0.0f;
  }
  returnLinePath();
}

void
RTSTiledMap::render() {
  int32 tmpX = 0;
  int32 tmpY = 0;
  int32 tmpTypeTile = 0;
  Vector2I clipRect;

  int32 tileIniX = 0, tileIniY = 0;
  int32 tileFinX = 0, tileFinY = 0;

#ifdef MAP_IS_ISOMETRIC
  int32 trashCoord = 0;
  getScreenToMapCoords(m_scrStart.x, m_scrStart.y, tileIniX, trashCoord);
  getScreenToMapCoords(m_scrEnd.x, m_scrEnd.y, tileFinX, trashCoord);

  getScreenToMapCoords(m_scrEnd.x, m_scrStart.y, trashCoord, tileIniY);
  getScreenToMapCoords(m_scrStart.x, m_scrEnd.y, trashCoord, tileFinY);
#else
  getScreenToMapCoords(m_scrStart.x, m_scrStart.y, tileIniX, tileIniY);
  getScreenToMapCoords(m_scrEnd.x, m_scrEnd.y, tileFinX, tileFinY);
#endif
  sf::Vector2f mousePosition = getMousePosOnRenderTarget();
  m_tileSelectedIndex = -1;
  int32 tailSelectedY = -1;
  int32 iterTailSelectedY = -1;
  int32 tailSelectedX = -1;
  int32 iterTailSelectedX = -1;
  bool tileAreadySelect = false;

  for (int32 iterX = tileIniX; iterX <= tileFinX; ++iterX) {
    for (int32 iterY = tileIniY; iterY <= tileFinY; ++iterY) {

      getMapToScreenCoords(iterX, iterY, tmpX, tmpY);
      if (tmpX > m_scrEnd.x ||
        tmpY > m_scrEnd.y ||
        (tmpX + TILESIZE_X) < m_scrStart.x ||
        (tmpY + TILESIZE_Y) < m_scrStart.y) {
        m_mapGridCopy[(iterY*m_mapSize.x) + iterX].m_bDrawing = false;
        continue;
      }
     if (tmpX < mousePosition.x&&
        tmpX + TILESIZE_X > mousePosition.x&&
        tmpY  < mousePosition.y&&
        tmpY + TILESIZE_Y > mousePosition.y)
      {
        m_tileSelectedIndexY = iterTailSelectedY = iterY;
        m_tileSelectedIndexX = iterTailSelectedX = iterX;
        tailSelectedY = tmpY + GameOptions::TILEHALFSIZE.y;
        tailSelectedX = tmpX + GameOptions::TILEHALFSIZE.x;
        m_tileSelectedIndex = (iterY*m_mapSize.x) + iterX;
      }
      m_mapGridCopy[(iterY*m_mapSize.x) + iterX].setPosition(
        static_cast<float>( tmpX + GameOptions::TILEHALFSIZE.x),
        static_cast<float>(tmpY + GameOptions::TILEHALFSIZE.y));
      tmpTypeTile = m_mapGridCopy[(iterY*m_mapSize.x) + iterX].getType();
      RTSTexture& refTexture = m_mapTextures[tmpTypeTile];

      clipRect.x = (iterX << GameOptions::BITSHFT_TILESIZE.x) % refTexture.getWidth();
      clipRect.y = (iterY << GameOptions::BITSHFT_TILESIZE.y) % refTexture.getHeight();

      refTexture.setPosition(tmpX, tmpY);
      sf::Color tmpColor = m_mapGridCopy[(iterY*m_mapSize.x) + iterX].getColor();
      refTexture.setColor(tmpColor.r, tmpColor.g, tmpColor.b, tmpColor.a);
      refTexture.setSrcRect(clipRect.x, clipRect.y, TILESIZE_X, TILESIZE_Y);
      m_mapGridCopy[(iterY*m_mapSize.x) + iterX].m_bDrawing = true;
      
      refTexture.draw();
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
#ifdef MAP_IS_ISOMETRIC
      gridLines.push_back(sf::Vertex(
        sf::Vector2f(static_cast<float>(tmpX + GameOptions::TILEHALFSIZE.x),
                     static_cast<float>(tmpY)),
        gridColor));
      
      gridLines.push_back(sf::Vertex(
        sf::Vector2f(static_cast<float>(tmpX2),
                     static_cast<float>(tmpY2 + GameOptions::TILEHALFSIZE.y)),
        gridColor));
#else
      gridLines.push_back(sf::Vertex(
        sf::Vector2f(static_cast<float>(tmpX), static_cast<float>(tmpY)),
        gridColor));

      gridLines.push_back(sf::Vertex(sf::Vector2f(static_cast<float>(tmpX2),
                                                  static_cast<float>(tmpY2 + TILESIZE_Y)),
                                     gridColor));
#endif
    }

    for (int32 iterY = tileIniY; iterY <= tileFinY + 1; ++iterY) {
      getMapToScreenCoords(tileIniX, iterY, tmpX, tmpY);
      getMapToScreenCoords(tileFinX, iterY, tmpX2, tmpY2);
#ifdef MAP_IS_ISOMETRIC
      gridLines.push_back(sf::Vertex(
        sf::Vector2f(static_cast<float>(tmpX + GameOptions::TILEHALFSIZE.x),
                     static_cast<float>(tmpY)),
        gridColor));

      gridLines.push_back(sf::Vertex(
        sf::Vector2f(static_cast<float>(tmpX2 + TILESIZE_X),
                     static_cast<float>(tmpY2 + GameOptions::TILEHALFSIZE.y)),
        gridColor));
#else
      gridLines.push_back(sf::Vertex(
        sf::Vector2f(static_cast<float>(tmpX), static_cast<float>(tmpY)),
        gridColor));

      gridLines.push_back(sf::Vertex(sf::Vector2f(static_cast<float>(tmpX2 + TILESIZE_X),
                                                  static_cast<float>(tmpY2)),
                                     gridColor));
#endif
    }

    m_pTarget->draw(&gridLines[0], gridLines.size(), sf::Lines);
  }
  if (m_PathLineTail.size()>0)
  {
    m_pTarget->draw(&m_PathLineTail[0], m_PathLineTail.size(), sf::LinesStrip);
  }
  {
    m_mouseInWindow.setPosition(mousePosition.x, mousePosition.y);
    m_pTarget->draw(m_mouseInWindow);
    if (m_tileSelectedIndex >= 0)
    {
      drawTailOutline(m_tileSelectedIndex,m_selectingTileColor);
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
