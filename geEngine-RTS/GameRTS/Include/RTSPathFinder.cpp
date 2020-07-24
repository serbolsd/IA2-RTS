#include "RTSPathFinder.h"

PathFinder::PathFinder()
{
}

PathFinder::~PathFinder()
{
}

void 
PathFinder::clearSearch() {
  m_mapGridVisited.clear();
  m_mapPathRegisterTail.clear();
  m_mapSearchRegister.clear();
  while (!m_pathStack.empty())
  {
    m_pathStack.pop_front();
  }
  m_bSearching = false;
  m_bGetPath = false;
  //m_mapGridCopy.clear();
  //m_mapGridCopy = m_mapGrid;
  m_dijkstra.clear();
  m_aStar.clear();
  m_BTPathLine.clear();
  //m_lastPendiente = { 0,0 };
  m_pCurrentBTTailNode = nullptr;
  m_BresenhammapPathRegisterTail.clear();
  //m_BresenhamPathLineTail.clear();
}

void
PathFinder::BreadthFirstSearch() {
  //if (m_pathStack.empty())
  //{
  //  m_bSearching = false;
  //  clearSearch();
  //  return;
  //}
  //
  //
  //actual->setcolor(m_serchingTileColor);
  //actual = m_pathStack.front();       // Sacamos el estado a procesar
  //actual->setcolor(m_lastSerchingTileColor);
  //
  //int actualIndX = actual->getIndexGridX();
  //int actualIndY = actual->getIndexGridY();
  //
  //while (m_mapGridVisited[actualIndY *m_mapSize.x + actualIndX])
  //{
  //  m_pathStack.pop_front();
  //  actual = m_pathStack.front();
  //  actualIndX = actual->getIndexGridX();
  //  actualIndY = actual->getIndexGridY();
  //}
  //m_mapSearchRegister.push_back(actual);
  //m_pathStack.pop_front();                         // Sacamos ese estado de la cola
  //
  //if (isSearchFinish(actualIndX, actualIndY))
  //{
  //  return;
  //}
  //
  //m_mapGridVisited[actualIndY*m_mapSize.x + actualIndX] = true;    // Marcamos como visitada la casilla actual
  //
  //for (int i = 0; i < 8; i++) {
  //  int nuevox = actualIndX + movx[i];
  //  int nuevoy = actualIndY + movy[i];
  //
  //  if (nuevox >= 0 && nuevox < m_mapSize.x && nuevoy >= 0 && nuevoy < m_mapSize.y) {    // Revisamos que esté en los límites
  //    if (!m_mapGridVisited[nuevoy *m_mapSize.x + nuevox] && nullptr == m_mapGridCopy[nuevoy*m_mapSize.x + nuevox].getParent() &&
  //      TERRAIN_TYPE::kObstacle != m_mapGrid[nuevoy *m_mapSize.x + nuevox].getType()) {  // Revisamos que no esté visitado y que no sea pared
  //      m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setParent(&m_mapGridCopy[actualIndY *m_mapSize.x + actualIndX]);
  //      m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndex(nuevoy *m_mapSize.x + nuevox);
  //      m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setIndexOnGrid(nuevox, nuevoy);
  //      m_mapGridCopy[nuevoy *m_mapSize.x + nuevox].setcolor(m_inStackTileColor);
  //      m_pathStack.push_back(&m_mapGridCopy[nuevoy *m_mapSize.x + nuevox]);
  //    }
  //  }
  //}
}
