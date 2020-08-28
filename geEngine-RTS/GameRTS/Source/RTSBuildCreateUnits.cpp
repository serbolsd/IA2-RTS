#include "RTSBuildCreateUnits.h"
#include "RTSWorld.h"
#include "RTSTiledMap.h"

BuildCreateUnits::BuildCreateUnits()
{
}

BuildCreateUnits::~BuildCreateUnits()
{
}

void BuildCreateUnits::init(int32 tileX, int32 tileY, unsigned int playerID, RTSWorld* world)
{
  m_tileIndexX = tileX;
  m_tileIndexY = tileY;
  m_playerID = playerID;
  m_ytpe = TYPEOBJECT::BUILD;
  if (playerID == 1)
  {
    m_color = sf::Color::Blue;
    m_influence = 1.0f;
  }
  else
  {
    m_color = sf::Color::Red;
    m_influence = -1.0f;
  }
  m_pWorld = world;
  m_pWorld->getTiledMap()->SetInfluenceInTile(tileX, tileY,m_influence);
}

void BuildCreateUnits::update(const float & deltaTime)
{

  m_pWorld->getTiledMap()->SetInfluenceInTile(m_tileIndexX, m_tileIndexY, m_influence);
  if (m_bCreating)
  {
    m_elapseTime += deltaTime;
    if (m_timeToCreate<= m_elapseTime)
    {
      m_elapseTime = 0;
      m_pWorld->createUnit(m_tileToCreate.x, m_tileToCreate.y, m_typeUnitToCreate, m_playerID);
      m_bCreating = false;
    }
  }
}

void BuildCreateUnits::generateUnit(int typeUnit)
{
  if (m_bCreating)
  {
    return;
  }
  bool tileRedy = false;
  for (int x = 0; x < 8; x++)
  {
    for (int y = 0; y < 8; y++)
    {
      int tileX = m_tileIndexX + movx[x];
      int tileY = m_tileIndexY + movy[y];
      int tile = tileY * m_pWorld->getTiledMap()->getMapSize().x + tileX;
      if (tileX<=0 || tileX>= m_pWorld->getTiledMap()->getMapSize().x ||
        tileY <= 0 || tileY >= m_pWorld->getTiledMap()->getMapSize().y)
      {
        continue;
      }
      if (m_pWorld->getTiledMap()->getTile(tile).m_myObject.size()<=0)
      {
        if (m_pWorld->getTiledMap()->checkCanAddUnitToTile((UNITTYPE::E)typeUnit, tile))
        {
          m_tileToCreate = { tileX,tileY };
          tileRedy = true;
          break;
        }
      }
      
    }
    if (tileRedy)
    {
      break;
    }
  }
  if (!tileRedy)
  {
    return;
  }
  switch (typeUnit)
  {
  case 0:
    m_timeToCreate = timeToCreateWalker;
    break;
  case 1:
    m_timeToCreate = timeToCreateFlying;
    break;
  case 2:
    m_timeToCreate = timeToCreateWater;
    break;
  default:
    break;
  }
  m_typeUnitToCreate = typeUnit;
  m_bCreating = true;
}


