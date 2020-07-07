#pragma once
namespace UNITSSTATES{
  enum E
  {
    KIDLESTATE = 0,
    KIRUNSTATE,
    KATTACKSTATE,
    KNUMSTATES
  };
}

class RTSUnit;

class unitState
{
public:
  unitState() {};
  virtual ~unitState() {};
  virtual void onEnter(RTSUnit&) {};
  virtual UNITSSTATES::E onUpdate(RTSUnit&) { return UNITSSTATES::E::KNUMSTATES; };
  virtual void onExit(RTSUnit&) {};
private:

};

class unitIdleState : public unitState
{
public:
  unitIdleState() {};
  ~unitIdleState() {};
  void onEnter(RTSUnit& _unit) override;
  UNITSSTATES::E onUpdate(RTSUnit& _unit) override;
  void onExit(RTSUnit& _unit) override;
private:

};

class unitRunState : public unitState
{
public:
  unitRunState() {};
  ~unitRunState() {};
  void onEnter(RTSUnit& _unit) override;
  UNITSSTATES::E onUpdate(RTSUnit& _unit) override;
  void onExit(RTSUnit& _unit) override;
private:

};

class unitAttackState : public unitState
{
public:
  unitAttackState() {};
  ~unitAttackState() {};
  void onEnter(RTSUnit& _unit) override;
  UNITSSTATES::E onUpdate(RTSUnit& _unit) override;
  void onExit(RTSUnit& _unit) override;
private:

};
