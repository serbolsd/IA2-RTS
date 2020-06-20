#include "StateMachine.h"
#include "RTSUnit.h"

StateMachine::StateMachine()
{
}

StateMachine::~StateMachine()
{
}

void StateMachine::init()
{
  m_pIdleState = new unitIdleState();
  m_pRunState = new unitRunState();
  m_pAttackState = new unitAttackState();;
}

void StateMachine::updateState(RTSUnit& _unit)
{
  UNITSSTATES::E currentState = _unit.getCurrentState();
  UNITSSTATES::E pastState = _unit.getPastState();
  bool ChangingState = false;
  if (currentState != pastState)
  {
    pastState = currentState;
    ChangingState = true;
    _unit.getState()->onExit(_unit);
  }
  switch (_unit.getCurrentState())
  {
  case UNITSSTATES::E::KIDLESTATE:
    _unit.setState(m_pIdleState);
    break;
  case UNITSSTATES::E::KIRUNSTATE:
    _unit.setState(m_pRunState);
    break;
  case UNITSSTATES::E::KATTACKSTATE:
    _unit.setState(m_pAttackState);
    break;
  default:
    break;
  }
  if (ChangingState)
  {
    _unit.getState()->onEnter(_unit);
  }

}

void StateMachine::onDelete()
{
  if (m_pIdleState != nullptr)
  {
    delete m_pIdleState;
  }
  if (m_pRunState != nullptr)
  {
    delete m_pRunState;
  }
  if (m_pAttackState != nullptr)
  {
    delete m_pAttackState;
  }

}
