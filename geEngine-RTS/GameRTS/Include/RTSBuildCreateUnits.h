#pragma once
#include "RTSObjects.h"
#include "RTSTextureObject.h"
#include "RTSUnit.h"

class RTSWorld;
class BuildCreateUnits : public Object
{
public:
  BuildCreateUnits();
  ~BuildCreateUnits();
  void init(int32 tileX, int32 tileY, unsigned int playerID, RTSWorld* world);
  void update(const float& deltaTime);
  void generateUnit(int typeUnit);
  sf::Color m_color;
  float m_elapseTime = 0;
  float m_timeToCreate = 0;
  bool m_bCreating = false;
private:
  RTSWorld* m_pWorld = nullptr;
  const float timeToCreateWalker = 2;
  const float timeToCreateFlying = 3;
  const float timeToCreateWater = 2.5;
  int m_typeUnitToCreate;
  Vector2I m_tileToCreate;
  int32 m_tileIndexX;
  int32 m_tileIndexY;
  float m_influence = 0.0f;
  const int movx[8] = { 1, 1, 0,-1,-1,-1,  0,  1 };    // Moves on X
  const int movy[8] = { 0,-1,-1,-1, 0, 1,  1,  1 };    // Moves on Y
};
