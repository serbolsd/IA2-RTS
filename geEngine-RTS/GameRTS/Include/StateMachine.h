#pragma once
#include "UnitsStates.h"
class RTSUnit;
class StateMachine
{
public:
  StateMachine();
  ~StateMachine();
  void init();
  void updateState(RTSUnit& _unit);
  void onDelete();
  unitState* getIdleState() { return m_pIdleState; };
private:
  unitState* m_pIdleState = nullptr;
  unitState* m_pRunState = nullptr;
  unitState* m_pAttackState = nullptr;
};