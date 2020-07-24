#pragma once

#include "RTSTiledMap.h"

class PathFinder
{
public:
  PathFinder();
  ~PathFinder();

  void
  update(float deltaTime);

  void
  render();

  void
  clearSearch();

private:

  class CLessWeight
  {
  public:
    bool 
    operator()(RTSTiledMap::MapTile* lhs, RTSTiledMap::MapTile* rhs) const {
      return lhs->getTentativeCost() < rhs->getTentativeCost();
    };
  };
  class CLessF
  {
  public:
    bool 
    operator()(RTSTiledMap::MapTile* pA, RTSTiledMap::MapTile* pB) const {
      float hA = pA->getEuristic();
      float hB = pB->getEuristic();
      return (hA + pA->getTentativeCost()) < (hB + pB->getTentativeCost());
    };
  };

  void
  BreadthFirstSearch();

  deque<RTSTiledMap::MapTile*> m_pathStack;             // Cola con los estados del camino
  deque<RTSTiledMap::MapTile*> m_mapPathRegisterTail;
  deque<RTSTiledMap::MapTile*> m_mapPathRegisterTailForBT;
  Vector<RTSTiledMap::MapTile*> m_BTmapPathRegisterTail;
  Vector<RTSTiledMap::MapTile*> m_BTPathLine;
  Vector<RTSTiledMap::MapTile*> m_BresenhammapPathRegisterTail;
  Vector<RTSTiledMap::MapTile*> m_BresenhamPathLine;
  Vector<RTSTiledMap::MapTile*> m_mapSearchRegister;
  RTSTiledMap::MapTile* m_pCurrentTailNode = nullptr;
  RTSTiledMap::MapTile* m_linePathTile = nullptr;
  RTSTiledMap::MapTile* m_pCurrentBTTailNode = nullptr;
  float m_euritic;
  EURISTIC_TYPE m_eEuristicType = EURISTIC_TYPE::EUCLIDEAN;
  PATHFINDING m_ePathFinding = PATHFINDING::BREADTH;
  std::multiset<RTSTiledMap::MapTile*, CLessWeight> m_dijkstra;
  std::multiset<RTSTiledMap::MapTile*, CLessF> m_aStar;


  bool m_bSearching = false;
  bool m_bGetPath = false;
  bool m_bGetPathBT = false;
  bool m_bBresenhamLine = false;

  float m_currenttimeToNext = 0.0f;
  float m_timeToNext = 0.0f;
  float m_timeToUpdateIM = 0.0f;

  Vector<bool> m_mapGridVisited;           // Tiles visited
  const int movx[8] = { 1, 1, 0,-1,-1,-1,  0,  1 };    // Moves on X
  const int movy[8] = { 0,-1,-1,-1, 0, 1,  1,  1 };    // Moves on Y

};
