#pragma once
#include "RTSObjects.h"
#include "RTSTextureObject.h"
#include "UnitsStates.h"
#include "StateMachine.h"

class RTSUnit : public Object
{
public:
  RTSUnit() { m_ytpe = TYPEOBJECT::UNIT; };
  ~RTSUnit() {};
  void onInit();
  void onUpdate(const float& deltaTime);
  void draw(sf::RenderTarget* m_pTarget);
 

  UNITSSTATES::E
  getCurrentState() {
    return m_currentState;
  }

  UNITSSTATES::E
  getPastState() {
    return m_pastState;
  }

  unitState*
  getState() {
    return m_pState;
  };

  void
  setState(unitState* newState) {
    m_pState =newState;
  };

  void
  setStateMachine(StateMachine* _statemachine) {
    m_pStateMachine = _statemachine;
  };

  bool m_bHaveObjetive = false;
  bool m_bNearObjetive = false;
  vector<Vector2> m_pathToFollow;
  Vector2 m_objetivePos;


  Vector2 seek(Vector2 PosB, float impetu);
  Vector2 FollowPath(float impetu, int& indexPath, float Ratio);
  Vector2 flee(Vector2 PosB, float impetu);
  Vector2 arrive(Vector2 PosB, float impetu, float ratio);
  int m_idexPath=0;
  Vector2 m_forces;


  RTSGame::ANIMATIONS::E m_animation = RTSGame::ANIMATIONS::kIDLE;
  void SetAnimationData(RTSTexture* _rtsTextures, Vector<Animation> animations);
private:
  void
  UdpateAnimation();

  UNITSSTATES::E m_estate = UNITSSTATES::KIDLESTATE;

  UNITSSTATES::E m_currentState;
  UNITSSTATES::E m_pastState;
  Vector<Animation> m_animations;
  Animation m_currentAnimation;
  RTSTexture* m_textures;
  float m_timeElapsed=0.0f;
  int m_animationIdex=0;

  RTSGame::DIRECTIONS::E m_animationDirection;

  Vector2 m_direction = { 0,1 };
  Vector2 m_newDirection={0,1};
  Vector2 m_directionView ={0,1};
  float m_speed = 0.1f;

  StateMachine* m_pStateMachine = nullptr;
  unitState* m_pState=nullptr;
};
