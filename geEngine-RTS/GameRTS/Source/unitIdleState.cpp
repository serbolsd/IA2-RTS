#include "UnitsStates.h"
#include "RTSUnit.h"

void unitIdleState::onEnter( RTSUnit& _unit) {
  _unit.m_animation = RTSGame::ANIMATIONS::kIDLE;
}

UNITSSTATES::E unitIdleState::onUpdate(RTSUnit& _unit) {
  if (_unit.m_bHaveObjetive)
  {
    float dist = Vector2::distance(_unit.m_objetivePos, _unit.m_position);
    if (TILESIZE_X*0.10 < dist)
    {
      return UNITSSTATES::E::KIRUNSTATE;
    }
  }
  _unit.m_forces = { 0,0 };
  return UNITSSTATES::E::KIDLESTATE;
}

void unitIdleState::onExit( RTSUnit& _unit) {

}