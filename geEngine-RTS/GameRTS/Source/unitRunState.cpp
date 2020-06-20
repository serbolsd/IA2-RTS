#include "UnitsStates.h"
#include "RTSUnit.h"

void unitRunState::onEnter(RTSUnit& _unit) {
  _unit.m_animation = RTSGame::ANIMATIONS::kRUN;
  _unit.m_idexPath = 0;
}

UNITSSTATES::E unitRunState::onUpdate(RTSUnit& _unit) {
  if (!_unit.m_bHaveObjetive)
  {
    return UNITSSTATES::E::KIDLESTATE;
  }
  _unit.m_forces=_unit.FollowPath(10, _unit.m_idexPath, 10);

  return UNITSSTATES::E::KIRUNSTATE;
}

void unitRunState::onExit( RTSUnit& _unit) {
  _unit.m_pathToFollow.clear();
  _unit.m_bHaveObjetive = false;
}