#pragma once

#include <gePlatformUtility.h>
#include <geVector2.h>
#include <geVector2I.h>

#include <SFML/Graphics.hpp>

#include "RTSConfig.h"
#include "RTSTextureObject.h"
#include "RTSObjects.h"
#include "RTSUnit.h"
#include "RTSBuildCreateUnits.h"
#include "StateMachine.h"
#include "RTSFunctionsCoords.h"

using namespace geEngineSDK;

class RTSTexture;

namespace TERRAIN_TYPE {
  enum E {
    kWater = 0,
    kGrass,
    kMarsh,
    kObstacle,
    kArrow,
    kNumObjects
  };
}
namespace ARROW_TYPE {
  enum E {
    kRight = 0,
    kRightDown,
    kDown,
    kLeftDown,
    kLeft,
    kLeftUp,
    kUp,
    kRightUp,
    kUnknow
  };
}
enum EURISTIC_TYPE
{
  EUCLIDEAN = 0,
  SQUAREDISTANCE,
  MANHATTAN
};

enum PATHFINDING
{
  DEPTH = 0,
  BREADTH,
  BEST,
  DIJKSTRA,
  ASTAR
};



class RTSTiledMap
{
  friend class PathFinder;
  class MapZone;
  class MapTile
  {
   public:
    MapTile();
    MapTile(const int8 idType, const int8 cost);
    MapTile(const MapTile& copy);

    MapTile&
    operator=(const MapTile& rhs);

    FORCEINLINE uint8
    getType() const {
      return m_idType;
    }

    void
    setType(const int8 idType) {
      m_idType = idType;
    }

    FORCEINLINE int8
    getCost() const {
      return m_cost;
    }

    void
    setCost(const int8 cost) {
      m_cost = cost;
    }

    void
    setPosition(const sf::Vector2f& position) {
      m_position = position;
    }

    void
      setPosition(const float& x, const float& y) {
      m_position = { x,y };
    }

    sf::Vector2f
    getPosition() {
      return m_position;
    }

    void
    setParent(MapTile* Parent) {
      m_pParent = Parent;
    };

    MapTile*
    getParent() {
      return m_pParent;
    }

    void
    setIndex(const int& index) {
      m_index = index;
    };

    void
    setIndexOnGrid(const int& x, const int& y) {
      m_indexX = x, m_indexY = y;
    };

    int
    getIndex() {
      return m_index;
    };

    int
    getIndexGridX() {
      return m_indexX;
    };

    int
    getIndexGridY() {
      return m_indexY;
    };

    void
    setEuristic(const float& euristic) {
      m_euristic = euristic;
    };

    float
    getEuristic() {
      return m_euristic;
    };

    void
    setTentativeCost(const float& tentativeCost) {
      m_TentativeCost = tentativeCost;
    };

    float
    getTentativeCost() {
      return m_TentativeCost;
    };

    void
    setcolor(const sf::Color& color) {
      m_color = color;
    };

    sf::Color
      getColor() {
      return m_color;
    };
    bool m_bDrawing = false;

    void
    addObject(Object* object) {
      for (int i = 0; i < m_myObject.size(); i++)
      {
        if (m_myObject[i]->getType() == object->getType())
        {
          return;
        }
      }
      m_myObject.push_back(object);
    }



    Vector<Object*> m_myObject;
    bool hadArrow = false;
    ARROW_TYPE::E m_directionArrow = ARROW_TYPE::E::kUnknow;

    float m_influence = 0;
    vector<MapTile*> m_connections;
    MapZone* m_myZone = nullptr;
    bool m_imLimit = false;
    bool m_haveObtacle = false;
   private:
    uint8 m_idType;
    int8 m_cost=0;
    sf::Vector2f m_position;
    sf::Color m_color = { 255,255,255,255 };
    MapTile* m_pParent = nullptr;
    int m_index = -1;
    int m_indexX = -1;
    int m_indexY = -1;
    float m_euristic = -1;
    float m_TentativeCost = float(INT_MAX);
  };

  class MapZone
  {
   public:
    vector<MapTile*> m_myTiles;
    vector<MapTile*> m_myLimits;
    sf::Color m_colorOfZone;
    sf::Color m_colorOfLimit;
    TERRAIN_TYPE::E m_terrainType;
  };

  class CLessWeight
  {
   public:
    bool operator()(MapTile* lhs, MapTile* rhs) const;
  };
  class CLessF
  {
   public:
    bool operator()(MapTile* pA, MapTile* pB) const;
  };

public:
  RTSTiledMap();
  RTSTiledMap(sf::RenderTarget* pTarget, const Vector2I& mapSize);
  ~RTSTiledMap();

public:
  bool
  init(sf::RenderTarget* pTarget, const Vector2I& mapSize);

  void
  destroy();

  void
  update(float deltaTime);

  void
  render();

  Vector2I
  getMapSize() const {
    return m_mapSize;
  }
  MapTile&
  getTile(int index) {
    if (index > 0 || index < m_mapSize.x * m_mapSize.y)
    {
      return m_mapGrid[index];
    }
  }
  
  bool
  loadFromImageFile(sf::RenderTarget* pTarget, String fileName);

  bool
  saveToImageFile(sf::RenderTarget* pTarget, String fileName);

  int8
  getCost(const int32 x, const int32 y) const;

  void
  setCost(const int32 x, const int32 y, const int8 cost);

  int8
  getType(const int32 x, const int32 y) const;

  void
  setType(const int32 x, const int32 y, const uint8 idtype);

  void
  setStart(const uint32 x, const uint32 y) {
    m_scrStart.x = x;
    m_scrStart.y = y;
    preCalc();
  }

  void
    getStart(uint32 &x, uint32 &y) const {
    x = m_scrStart.x;
    y = m_scrStart.y;
  }

  void
  setEnd(const uint32 x, const uint32 y) {
    m_scrEnd.x = x;
    m_scrEnd.y = y;
    preCalc();
  }

  void
  getEnd(uint32 &x, uint32 &y) const {
    x = m_scrEnd.x;
    y = m_scrEnd.y;
  }

  void
  preCalc() {
    m_PreCalc_MidResolution = (m_scrEnd - m_scrStart) / 2;
    COORDS::PreCalc_MidResolution = m_PreCalc_MidResolution;
    if (GameOptions::s_MapIsIsometric)
      m_PreCalc_MaxCameraCoord.x = m_mapSize.x * GameOptions::TILEHALFSIZE.x;
    else
      m_PreCalc_MaxCameraCoord.x = m_mapSize.x * GameOptions::s_TileSizeX;

    m_PreCalc_MaxCameraCoord.y = m_mapSize.y * GameOptions::s_TileSizeY;
  }

  void
  moveCamera(const float dx, const float dy);

  void
  setCameraStartPosition(const int32 x, const int32 y);

  void
  getCameraStartPosition(int32 &x, int32 &y) const {
    x = m_iCamera.x;
    y = m_iCamera.y;
  }

  void
  getScreenToMapCoords(const int32 scrX,
      const int32 scrY,
      int32 &mapX,
      int32 &mapY);

  void
  getMapToScreenCoords(const int32 mapX,
      const int32 mapY,
      int32 &scrX,
      int32 &scrY);

  sf::Vector2f
  getMousePosOnRenderTarget();

  void
  selecetStartTail();

  void
  selecetFinalTail();

  void
  startPathFinding();

  void
  setEuristic(const int& i);

  void
  setPathFinding(const int& i);

  void
  setTileType(const int& i);

  void
  setTileCost(int index,int type);

  void
  addTree();

  void
  addUnit(UNITTYPE::E _tipe, bool teamRed = false);

  RTSUnit*
  getUnitInTile(int indexX, int indexY);

  BuildCreateUnits*
  getBuildInTile(int indexX, int indexY);

  void
  selectUnit();

  void
  setTimeToPaintNextTail(const float& time) {
    m_timeToNext = time;
  }

  void
  setEuristicRelevance(const float& relevance) {
    if (m_bSearching)
      return;
    m_euristicRelevance = relevance;
  }

  void
  setCostRelevance(const float& relevance) {
    if (m_bSearching)
      return;
    m_costRelevance = relevance;
  }

  void
  deleteObjectInTile(Object* _object, int32 tilex, int32 tiley);

  void
  SetInfluenceInTile(int32 tilex, int32 tileym, const float& influence);

  void
  insetObjectInTile(Object* _object, int32 tilex, int32 tiley);

  void
  initZones();

  float m_momentum = 0.70f;
  float m_momentumPerSecond = 0.70f;
  float m_decay = 0.15f;
  float m_decayPerSecond = 0.15f;
  bool m_bShowInfluenceMap = false;
  bool m_bShowZones = false;
  bool m_bRedTeam = false;
  int32 m_stepToDo = 200;

  int m_tileSelectedIndex = -1;
  int m_tileSelectedIndexX = -1;
  int m_tileSelectedIndexY = -1;
  
  bool 
  checkCanAddUnitToTile(UNITTYPE::E typeUnit, int indexTile);

private:

  void
  propagateInfluence(int tileIndex);

  void
  drawTailOutline(const int & tailIndex, const int & tailIndexY, const sf::Color & outlineColor);

  void
  setArrow();

  bool 
  checkCanAddTile(UNITTYPE::E typeUnit, MapTile* tile);

  void
  BreadthFirstSearch();

  void
  BreadthFirstSearchWithSteps(uint32 steps, UNITTYPE::E typeUnit);

  void
  DepthFirstSearch();

  void
  DepthFirstSearchWithSteps(uint32 steps, UNITTYPE::E typeUnit);

  void
  BestFirstSearch();

  void
  BestFirstSearchWithSteps(uint32 steps, UNITTYPE::E typeUnit);

  void
  DijkstraSearch();

  void
  DijkstraSearchWithSteps(uint32 steps, UNITTYPE::E typeUnit);

  void
  DijkstraSearchForZones(uint32 steps);

  void
  DijkstraSearchForZonesPerFrame(uint32 steps);

  void
  checkLimitOfzones();

  void
  AstarSearch();

  void
  AstarSearchWithSteps(uint32 steps, UNITTYPE::E typeUnit);

  bool
  isSearchFinish(const int x, const int y);

  void
  returnPath();

  void
  returnLinePath();

  void
  returnBresenhamLinePath();

  bool
  checkForBresenhamLinePath(int IndexTileInit, int indexTileFinish);

  void
  clearSearch();

  void
  backTracking();

  void
  updateTileColor();
private:
  Vector2I m_mapSize;
  Vector<MapTile> m_mapGrid;
  Vector<MapZone> m_mapZones;
  Vector<RTSTexture> m_mapTextures;
  TextureObject m_textureObjects;
  TextureObject m_textureArrow;
  Vector<Object*> m_objects;

  Vector2I m_iCamera;
  Vector2 m_fCamera;

  Vector2I m_scrStart;
  Vector2I m_scrEnd;

  Vector2I m_PreCalc_MidResolution;
  Vector2I m_PreCalc_MaxCameraCoord;
  Vector2I m_PreCalc_ScreenDeface;

  sf::RenderTarget* m_pTarget;

  sf::CircleShape m_mouseInWindow;

  MapTile* m_tileSelected = nullptr;

  MapTile* m_tileStart = nullptr;
  int m_tileStartIndexX = -1;
  int m_tileStartIndexY = -1;
  MapTile* m_tileFinish = nullptr;
  int m_tileFinishIndexX = -1;
  int m_tileFinishIndexY = -1;

  const sf::Color m_selectingTileColor = { 0, 255, 125, 255 };
  const sf::Color m_tileStartColor = { 0,255,255,255 };
  const sf::Color m_tileFinishColor = { 255,120,0,255 };
  const sf::Color m_serchingTileColor = { 200, 200, 255, 255 };
  const sf::Color m_inStackTileColor = { 100, 100, 100, 255 };
  const sf::Color m_lastSerchingTileColor = { 0, 0, 0, 255 };
  const sf::Color m_PathTileColor = { 200, 0, 255, 255 };
  const sf::Color m_BTTileColor = { 255, 0, 0, 255 };
  const sf::Color m_NormalTileColor = { 255, 255, 255, 255 };

  Vector<bool> m_mapGridVisited;           // Tiles visited
  Vector<bool> m_bTileDrawn;           // Tiles drawned
  Vector<MapTile*> m_mapSearchRegister;
  const int movx[8] = { 1, 1, 0,-1,-1,-1,  0,  1 };    // Moves on X
  const int movy[8] = { 0,-1,-1,-1, 0, 1,  1,  1 };    // Moves on Y

  deque<MapTile*> m_pathStack;             // Cola con los estados del camino
  deque<MapTile*> m_mapPathRegisterTail;
  deque<MapTile*> m_mapPathRegisterTailForBT;
  Vector<MapTile*> m_BTmapPathRegisterTail;
  Vector<MapTile*> m_BTPathLine;
  Vector<MapTile*> m_BresenhammapPathRegisterTail;
  Vector<MapTile*> m_BresenhamPathLine;

  bool m_bSearching = false;
  bool m_bGetPath = false;
  bool m_bGetPathBT = false;
  bool m_bBresenhamLine = false;

  float m_currenttimeToNext = 0.0f;
  float m_timeToNext = 0.0f;
  float m_timeToUpdateIM = 0.0f;

  FrameVector<sf::Vertex> m_outLineTail;
  FrameVector<sf::Vertex> m_PathLineTail;
  FrameVector<sf::Vertex> m_BresenhamPathLineTail;
  FrameVector<sf::Vertex> m_finalPath;
  MapTile* actual = nullptr;

  Vector<MapTile> m_mapGridCopy;
  MapTile *m_pCurrentTailNode = nullptr;
  MapTile *m_linePathTile = nullptr;
  MapTile *m_pCurrentBTTailNode = nullptr;
  MapTile *m_pNearlearTile = nullptr;
  float m_euritic;
  EURISTIC_TYPE m_eEuristicType = EURISTIC_TYPE::EUCLIDEAN;
  PATHFINDING m_ePathFinding = PATHFINDING::BREADTH;
  std::multiset<MapTile*, CLessWeight> m_dijkstra;
  std::multiset<MapTile*, CLessF> m_aStar;

  Vector2I m_lastPendiente = { 0,0 };
  float m_euristicRelevance = 1;
  float m_costRelevance = 1;
  //RTSUnit m_archerUnit;
  RTSUnit* m_selectedUnit = nullptr;
  StateMachine m_stateMachine;
  uint32 m_dijkstraStepsDone=0;
  uint32 m_StepsDone=0;
  uint32 m_StepsToZone= 5000;
  bool m_bAllMapWithZones= false;
  bool m_loadingMap = false;

  MapZone m_tmpzone;
  int m_tmpr;
  int m_tmpg;
  int m_tmpb;
  int m_tmpa = 255;
  bool m_tmpChekingZones = false;
  TERRAIN_TYPE::E m_tmpCurrentType;
};
