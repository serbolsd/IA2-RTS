#pragma once

#include <gePlatformUtility.h>
#include <geVector2.h>
#include <geVector2I.h>

#include <SFML/Graphics.hpp>

#include "RTSConfig.h"

using namespace geEngineSDK;

class RTSTexture;

namespace TERRAIN_TYPE {
  enum E {
    kWater = 0,
    kGrass,
    kMarsh,
    kObstacle,
    kNumObjects
  };
}

  enum EURISTIC_TYPE
  {
    EUCLIDEAN =0,
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
    setPosition(const sf::Vector2f& position){
      m_position = position;
    }

    void
    setPosition(const float& x, const float& y) {
      m_position = { x,y };
    }

    sf::Vector2f
    getPosition(){
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
      m_indexX = x, m_indexY=y;
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

   
  private:
    uint8 m_idType;
    int8 m_cost;
    sf::Vector2f m_position;
    MapTile* m_pParent=nullptr;
    int m_index=-1;
    int m_indexX=-1;
    int m_indexY=-1;
    float m_euristic=-1;
    float m_TentativeCost = INT_MAX;
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
#ifdef MAP_IS_ISOMETRIC
    m_PreCalc_MaxCameraCoord.x = m_mapSize.x * GameOptions::TILEHALFSIZE.x;
#else
    m_PreCalc_MaxCameraCoord.x = m_mapSize.x * TILESIZE_X;
#endif
    m_PreCalc_MaxCameraCoord.y = m_mapSize.y * TILESIZE_Y;
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

 private:
  void
  drawTailOutline(const int & tailIndex, const sf::Color & outlineColor);
  
  void
  BreadthFirstSearch();

  void
  DepthFirstSearch();

  void
  BestFirstSearch();
  
  void
  DijkstraSearch();

  void
  AstarSearch();

  void
  returnPath();
  
  void
  clearSearch();

 private:
  Vector2I m_mapSize;
  Vector<MapTile> m_mapGrid;
  Vector<RTSTexture> m_mapTextures;

  Vector2I m_iCamera;
  Vector2 m_fCamera;

  Vector2I m_scrStart;
  Vector2I m_scrEnd;

  Vector2I m_PreCalc_MidResolution;
  Vector2I m_PreCalc_MaxCameraCoord;
  Vector2I m_PreCalc_ScreenDeface;

  sf::RenderTarget* m_pTarget;

  sf::CircleShape m_mouseInWindow;

  int m_tileSelectedIndex = -1;
  int m_tileSelectedIndexX = -1;
  int m_tileSelectedIndexY = -1;
  MapTile* m_tileStart = nullptr;
  int m_tileStartIndexX = -1;
  int m_tileStartIndexY = -1;
  MapTile* m_tileFinish = nullptr;
  int m_tileFinishIndexX = -1;
  int m_tileFinishIndexY = -1;

  const sf::Color m_selectingTileColor = { 0, 255, 125, 255 };
  const sf::Color m_tileStartColor = {0,255,255,255};
  const sf::Color m_tileFinishColor = {255,255,0,255};
  const sf::Color m_serchingTileColor = { 255, 255, 255, 255 };
  const sf::Color m_lastSerchingTileColor = { 0, 0, 0, 255 };
  const sf::Color m_lastPathTileColor = { 255, 0, 255, 255 };

  class estado {
  public:
    int indexX;  // Coordenada en x
    int indexY;  // Coordenada en Y
    int d;  // Distancia recorrida
    float m_euristic;  // Distancia recorrida
    int8 m_cost;  // Distancia recorrida
    float m_TentativeCost;  // Distancia recorrida
    MapTile* m_pParent;
    estado(int tileIndexX, int tileIndexY, int8 cost=0,float tentativeCost=0,MapTile* parent=nullptr,const float&euristic=0) {     // Constructor
      indexX = tileIndexX, indexY = tileIndexY, m_cost=cost, m_TentativeCost=tentativeCost, m_pParent=parent,m_euristic=euristic;
    }
  };

  Vector<bool> m_mapGridVisited;           // Tiles visited
  Vector<MapTile*> m_mapSearchRegister;
  const int movx[4] = { 1, -1, 0, 0 };    // Moves on X
  const int movy[4] = { 0, 0, 1, -1 };    // Moves on Y

  deque<MapTile*> m_pathStack;             // Cola con los estados del camino
  Vector<MapTile*> m_mapPathRegisterTail;

  bool m_bSearching = false;
  bool m_bGetPath = false;

  float m_currenttimeToNext = 0.0f;
  float m_timeToNext = 0.0f;

  FrameVector<sf::Vertex> m_outLineTail;
  MapTile* actual;

  Vector<MapTile> m_mapGridCopy;
  MapTile *m_pCurrentTailNode=nullptr;
  float m_euritic;
  EURISTIC_TYPE m_eEuristicType = EURISTIC_TYPE::EUCLIDEAN;
  PATHFINDING m_ePathFinding = PATHFINDING::DEPTH;
  std::multiset<MapTile*, CLessWeight> m_dijkstra;
  std::multiset<MapTile*, CLessF> m_aStar;

  float m_euristicRelevance = 1;
  float m_costRelevance = 1;
};
