#include "RTSWorld.h"
#include "RTSTiledMap.h"

#include "RTSUnitType.h"
#include "RTSUnit.h"

struct LuaUnit
{
  float deltaTime = 2.0f;
  float speed = 2.0f;
  Vector2 position = { 1,2 };
  Vector2 direction = { 1,0 };
  Vector2 force = { 1,0 };

  void setDeltaTime(float time) {
    deltaTime = time;
  }

  float getDeltaTime() const {
    return deltaTime;
  }

  void setSpeed(float spd) {
    speed = spd;
  }

  float getSpeed() const {
    return speed;
  }

  void setPosition(Vector2 pos) {
    position = pos;
  }

  Vector2 getPosition() const {
    return position;
  }

  void setDirection(Vector2 dir) {
    direction = dir;
  }

  Vector2 getDirection() const {
    return direction;
  }

  void setForce(Vector2 F) {
    force = F;
  }

  Vector2 getForce() const {
    return force;
  }
};

struct LuaVector
{
  Vector2 vec = { 0,0 };
  Vector2
    operator-(Vector2 vec2) {
    //LuaVector result;
    //result.vec = vec - vec2.vec;
    return  vec - vec2;
  }
  void fun()
  {
    vec = { 10,10 };
  }
  void setData(Vector2 data) {
    vec = data;
  }

  Vector2 getData() const {
    return vec;
  }
};

Vector2 luaSubstracVector(Vector2 vec1, Vector2 vec2)
{
  return vec1 - vec2;
}

Vector2 luaNormalizeVector(Vector2 vec)
{
  return vec.getSafeNormal();
}

Vector2 luaMultiplyVecForMagnitude(Vector2 vec, float magnitude)
{
  return vec * magnitude;
}

Vector2 luaAddVectors(Vector2 vec1, Vector2 vec2)
{
  return vec1 + vec2;
}

bool luaCheckVectorZero(Vector2 vec) {
  if (vec == Vector2(0, 0))
  {
    return true;
  }
  return false;
}


RTSWorld::RTSWorld() {
  m_pTiledMap = nullptr;
  m_activeWalkerIndex = -1;	//-1 = Invalid index
}

RTSWorld::~RTSWorld() {
  destroy();
}

bool
RTSWorld::init(sf::RenderTarget* pTarget) {
  GE_ASSERT(nullptr == m_pTiledMap && "World was already initialized");
  destroy();

  m_pTarget = pTarget;

  //Initialize the map (right now it's an empty map)
  m_pTiledMap = ge_new<RTSTiledMap>();
  GE_ASSERT(m_pTiledMap);
  m_pTiledMap->init(m_pTarget, Vector2I(GameOptions::s_MapSizeX, GameOptions::s_MapSizeY));
  m_textureObjects.onInit(pTarget);
  m_stateMachine.init();
  //m_pTiledMap->init(m_pTarget, Vector2I(10, 10));

  //Create the path finding classes and push them to the walker list
  //m_walkersList.push_back(ge_new<RTSBreadthFirstSearchMapGridWalker>(m_pTiledMap));

  //Init the walker objects
/*
  for (SIZE_T it = 0; it < m_walkersList.size(); ++it) {
    m_walkersList[it]->init();
  }

  //Set the first walker as the active walker
  setCurrentWalker(m_walkersList.size() > 0 ? 0 : -1);
*/

  RTSGame::RTSUnitType unitTypes;
  unitTypes.loadAnimationData(m_pTarget, 1);
  RTSPlayer newPlayer;
  newPlayer.setID(m_nextPlayerID);
  newPlayer.setName("Blue");
  m_players.push_back(newPlayer);
  ++m_nextPlayerID;
  newPlayer.setID(m_nextPlayerID);
  newPlayer.setName("Red");
  m_players.push_back(newPlayer);

  m_luaStateInitUnits.open_libraries(sol::lib::base);
  m_luaStateInitUnits.set_function("createUnit", &RTSWorld::createUnit, this);
 // m_luaStateInitUnits.script_file("lua/InitUnits.lua");
  //luaLoadUnitScript();
  return true;
}

void
RTSWorld::destroy() {
  for (int i = 0; i < m_units.size(); ++i)
  {
    if (nullptr != m_units[i])
    {
      delete m_units[i];
    }
  }
  m_units.clear();

  for (int i = 0; i < m_Builds.size(); ++i)
  {
    if (nullptr != m_Builds[i])
    {
      delete m_Builds[i];
    }
  }
  m_Builds.clear();
  m_textureObjects.onExit();
  m_stateMachine.onDelete();
  //Destroy all the walkers
  while (m_walkersList.size() > 0) {
    ge_delete(m_walkersList.back());
    m_walkersList.pop_back();
  }

  //As the last step, destroy the full map
  if (nullptr != m_pTiledMap) {
    ge_delete(m_pTiledMap);
    m_pTiledMap = nullptr;
  }
}

void
RTSWorld::update(float deltaTime) {
  for (int i = 0; i < m_units.size(); ++i)
  {
    m_units[i]->onUpdate(deltaTime, *m_pTiledMap);
  }
  for (int i = 0; i < m_Builds.size(); i++)
  {
    m_Builds[i]->update(deltaTime);
  }
  m_pTiledMap->update(deltaTime);
}

void
RTSWorld::render() {
  m_pTiledMap->render();
}

void
RTSWorld::updateResolutionData() {
  if (nullptr != m_pTiledMap) {
    Vector2I appResolution = g_gameOptions().s_Resolution;
    
    m_pTiledMap->setStart(0, 0);
    m_pTiledMap->setEnd(appResolution.x, appResolution.y - 175); //Cut the map in y on the window view
    //m_pTiledMap->setEnd(appResolution.x, appResolution.y);  //All the map can be seen
    
    //This ensures a clamp if necessary
    m_pTiledMap->moveCamera(0, 0);
  }
}

void
RTSWorld::setCurrentWalker(const int8 index) {
  //Revisamos que el walker exista (en modo de debug)
  GE_ASSERT(m_walkersList.size() > static_cast<SIZE_T>(index));

  m_activeWalker = m_walkersList[index];
  m_activeWalkerIndex = index;
}

void 
RTSWorld::addUnit(UNITTYPE::E typeUnit) {
  RTSUnit* unit = nullptr;
  if (m_bRedTeam)
  {
    unit = createUnit(m_pTiledMap->m_tileSelectedIndexX, m_pTiledMap->m_tileSelectedIndexY,typeUnit, 2);
  }
  else
  {
    unit = createUnit(m_pTiledMap->m_tileSelectedIndexX, m_pTiledMap->m_tileSelectedIndexY, typeUnit, 1);
  }
  //if (nullptr != unit)
  //{
  //  m_units.push_back(unit);
  //}
}

void 
RTSWorld::selectUnit(int playerId) {
  RTSUnit* unit = m_pTiledMap->getUnitInTile(m_pTiledMap->m_tileSelectedIndexX, m_pTiledMap->m_tileSelectedIndexY);
  if (nullptr == unit)
  {
    return;
  }
  if (unit->m_playerID == playerId)
  {
    m_pTiledMap->selectUnit();
  }
}

void RTSWorld::selectBuild(int playerId)
{
  BuildCreateUnits* build = m_pTiledMap->getBuildInTile(m_pTiledMap->m_tileSelectedIndexX, m_pTiledMap->m_tileSelectedIndexY);
  if (nullptr == build)
  {
    m_pBuildSelected =nullptr;
    return;
  }
  if (build->m_playerID == playerId)
  {
    m_pBuildSelected = build;
  }
}

void 
RTSWorld::addBuild(int playerID) {
  //RTSUnit newUnit;
  auto size = m_pTiledMap->getMapSize();
  int tileY = m_pTiledMap->m_tileSelectedIndexY;
  int tileX = m_pTiledMap->m_tileSelectedIndexX;
  int tile = tileY * size.x + tileX;
  if (((tileX >= 0 && tileX < size.x) || (tileY >= 0 && tileY < size.y)) &&
    m_pTiledMap->getTile(tile).m_myObject.size()<=0)
  {
    BuildCreateUnits* newBuild = new BuildCreateUnits;
    newBuild->init(tileX, tileY, playerID,this);
    m_pTiledMap->getTile(tileY * m_pTiledMap->getMapSize().x + tileX).addObject(newBuild);
    m_Builds.push_back(newBuild);
  }
}

void RTSWorld::solCreateUnit(sol::state s)
{
  //createUnit();
}

RTSUnit*
RTSWorld::createUnit(int tileX, int tileY, int typeUnit, unsigned int playerID) {
  //RTSUnit newUnit;
  auto size = m_pTiledMap->getMapSize();
  int tile = tileY * size.x + tileX;
  if (((tileX >= 0 && tileX < size.x)||(tileY >= 0 && tileY < size.y)) &&
      m_pTiledMap->checkCanAddUnitToTile((UNITTYPE::E)typeUnit, tile))
  {
    RTSUnit* newUnit = new RTSUnit;
    newUnit->m_tileIndexX = tileX;
    newUnit->m_tileIndexY = tileY;

    newUnit->setType(TYPEOBJECT::UNIT);
    newUnit->m_unitType = (UNITTYPE::E)typeUnit;
    newUnit->setStateMachine(&m_stateMachine);
    RTSTexture* textureUnits=nullptr;
    Vector<Animation> anmations;
    switch (typeUnit)
    {
    case UNITTYPE::KTERRESTRIAL:
        anmations = m_textureObjects.getArcherAnimations();
      break;
    case UNITTYPE::KFLYING:
      anmations = m_textureObjects.getflyAnimations();
      break;
    case UNITTYPE::KMARINE:
        anmations = m_textureObjects.getwaterAnimations();
        break;
    default:
      return nullptr;
      break;
    }
    if (playerID == 2)
    {
      textureUnits = &m_textureObjects.getTesturesUnitB();
      newUnit->setInfluence(-1.0f);
    }
    else
    {
      newUnit->setInfluence(1.0f);
      textureUnits = &m_textureObjects.getTesturesUnit();
    }
    newUnit->SetAnimationData(textureUnits, anmations);
    newUnit->m_playerID = playerID;

    Vector2I pixelPos;
    COORDS::getTileCenterOnPixelCoords(tileX, 
                                       tileY, 
                                       pixelPos.x, 
                                       pixelPos.y);
    newUnit->m_position = { float(pixelPos.x), float(pixelPos.y) };
    
    newUnit->m_lasTile = { tileX ,tileY };
    newUnit->m_bOnMap = true;
    newUnit->onInit();
    //newUnit->m_luaScripUnits = &m_luaScripUnits;
    luaLoadUnitScript(newUnit->m_luaScripUnits);
    m_pTiledMap->getTile(tileY * m_pTiledMap->getMapSize().x +tileX).addObject(newUnit);
    m_units.push_back(newUnit);
    return newUnit;
  }
  return nullptr;
}

void RTSWorld::luaLoadUnitScript(sol::state& luaScripUnits)
{
  luaScripUnits.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math, sol::lib::jit);
  //lua.script_file("lua/vector.lua");
  sol::usertype<LuaUnit> playerType = luaScripUnits.new_usertype<LuaUnit>("player", sol::constructors<LuaUnit()>());
  sol::usertype<LuaVector> vectorType = luaScripUnits.new_usertype<LuaVector>("vector", sol::constructors<LuaVector()>());
  playerType["deltaTime"] = sol::property(&LuaUnit::getDeltaTime, &LuaUnit::setDeltaTime);
  playerType["position"] = sol::property(&LuaUnit::getPosition, &LuaUnit::setPosition);
  playerType["speed"] = sol::property(&LuaUnit::getSpeed, &LuaUnit::setSpeed);
  playerType["direction"] = sol::property(&LuaUnit::getDirection, &LuaUnit::setDirection);
  playerType["force"] = sol::property(&LuaUnit::getForce, &LuaUnit::setForce);
  vectorType["vec"] = sol::property(&LuaVector::getData, &LuaVector::setData);
  //lua.set_function("fun",&LuaVector::fun);
  luaScripUnits.set_function("SubstracVectors", luaSubstracVector);
  luaScripUnits.set_function("AddVectors", luaAddVectors);
  luaScripUnits.set_function("NormalizeVector", luaNormalizeVector);
  luaScripUnits.set_function("MultiplyVecForMagnitude", luaMultiplyVecForMagnitude);
  luaScripUnits.set_function("CheckVectorZero", luaCheckVectorZero);
  //execute
  luaScripUnits.script_file("lua/scriptTest.lua");
}
