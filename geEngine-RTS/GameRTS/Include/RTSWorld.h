#pragma once

#include <gePrerequisitesUtil.h>
#include <geVector2I.h>

#include <SFML/Graphics.hpp>
#include "RTSPlayer.h"
#include "RTSUnit.h"
#include "RTSBuildCreateUnits.h"
#include "RTSTextureObject.h"
#include <sol/sol.hpp>

using namespace geEngineSDK;

class RTSTiledMap;
class RTSMapGridWalker;
class RTSUnit;

class RTSWorld
{
 public:
  RTSWorld();
  ~RTSWorld();

 public:
  bool
  init(sf::RenderTarget* pTarget);

  void
  destroy();

  void
  update(float deltaTime);

  void
  render();

  RTSTiledMap*
  getTiledMap() {
    return m_pTiledMap;
  }

  void
  updateResolutionData();

  void
  setCurrentWalker(const int8 index);

  void 
  addUnit(UNITTYPE::E typeUnit);

  void
  selectUnit(int playerId);

  void
  selectBuild(int playerId);

  void
  addBuild(int playerID);

  bool
  m_bRedTeam = false;

  BuildCreateUnits* m_pBuildSelected = nullptr;
 private:
  void solCreateUnit(sol::state s);
  RTSUnit* createUnit(int tileX,int tileY,int typeUnit, unsigned int playerID = 0);
  void luaLoadUnitScript(sol::state& luaScripUnits);
 private:
  RTSTiledMap* m_pTiledMap;
  //List<RTSUnitType*> m_lstUnitTypes;
  //List<RTSUnit*> m_lstUnits;
  
  //Vector<RTSMapGridWalker*> m_walkersList;
  Vector<void*> m_walkersList;
  //RTSMapGridWalker* m_activeWalker;
  void* m_activeWalker;
  int8 m_activeWalkerIndex;

  StateMachine m_stateMachine;

  TextureObject m_textureObjects;

  sf::RenderTarget* m_pTarget;

  Vector<RTSUnit*> m_units;
  Vector<BuildCreateUnits*> m_Builds;
  Vector<RTSPlayer> m_players;

  unsigned int m_nextPlayerID = 1;
  unsigned int m_nextObjectID = 1;
  unsigned int m_nextUnitID = 1;

  sol::state m_luaStateInitUnits;
  sol::state m_luaScripUnits;
  friend class BuildCreateUnits;
  
};
