#include "RTSUnit.h"
#include "RTSTiledMap.h"
#include "RTSFunctionsCoords.h"

void RTSUnit::onInit() {
  m_pState = m_pStateMachine->getIdleState();
  string nameAnimation="idle_S";
  for (int i = 0; i < m_animations.size(); i++)
  {

    if (m_animations[i].AnimationName == nameAnimation)
    {
      m_currentAnimation = m_animations[i];
      return;
    }
  }
}

void RTSUnit::onUpdate(const float& deltaTime, RTSTiledMap& tileMap) {
  if (!m_bOnMap)
  {
    return;
  }
  m_timeElapsed += deltaTime;
  COORDS::getPixelToMapCoords(m_position.x, m_position.y, m_lasTile.x, m_lasTile.y);
  if (m_currentTile!= m_lasTile)
  {
    tileMap.deleteObjectInTile(this, m_currentTile.x, m_currentTile.y);
    m_currentTile = m_lasTile;
    tileMap.insetObjectInTile(this, m_currentTile.x, m_currentTile.y);
    m_tileIndexX = m_currentTile.x;
    m_tileIndexY = m_currentTile.y;
  }
  tileMap.SetInfluenceInTile(m_currentTile.x, m_currentTile.y,m_influenceOfUnit);
  if (m_forces == Vector2(0, 0))
  {
    m_newDirection = { 0,0 };
  }
  else
  {
    m_newDirection = m_forces;
  }
  //Vector2 pointToSeek = m_newDirection + m_position;
  m_pastState = m_currentState;
  m_currentState = m_pState->onUpdate(*this);
  m_pStateMachine->updateState(*this);

  if (m_newDirection != Vector2(0, 0))
  {
    m_direction= m_newDirection;
    m_direction.normalize();
    m_direction *= m_speed*deltaTime;
    m_directionView = m_direction.getSafeNormal();
  }
  else
  {
    m_direction = Vector2(0, 0);
  }
  UdpateAnimation();
  m_position += m_direction;
  return;
}

void RTSUnit::draw() {
  if (!m_bOnMap)
  {
    return;
  }
  RTSTexture& refTexture = *m_textures;
  Vector2 screenPos;
  COORDS::getPixelToScreenCoords(static_cast<int32>(m_position.x), static_cast<int32>(m_position.y), screenPos.x, screenPos.y);

  refTexture.setPosition(screenPos.x, screenPos.y);

  if (m_timeElapsed > 0.1) {
    ++m_animationIdex;
    m_timeElapsed = 0.0f;
    if (m_currentAnimation.frameData.size() == m_animationIdex)
      m_animationIdex = 0;
  }
  if (m_currentAnimation.frameData[m_animationIdex].rotated)
  {
    refTexture.setScale(-1, 1);
  }
  refTexture.setSrcRect(m_currentAnimation.frameData[m_animationIdex].x,
    m_currentAnimation.frameData[m_animationIdex].y,
    m_currentAnimation.frameData[m_animationIdex].width,
    m_currentAnimation.frameData[m_animationIdex].high);
  refTexture.setOrigin(m_currentAnimation.frameData[m_animationIdex].width/2, m_currentAnimation.frameData[m_animationIdex].high);
  refTexture.draw();
  if (m_currentAnimation.frameData[m_animationIdex].rotated)
  {
    refTexture.setScale(-1, 1);
  }
}

Vector2 RTSUnit::seek(Vector2 PosB, float impetu)
{
  Vector2 Dir = PosB - m_position;
  Dir.normalize();
  Vector2 F = Dir * impetu;
  return F;
}

Vector2 RTSUnit::FollowPath(float impetu, int & indexPath, float Ratio)
{
  Vector2 v1 = m_position - m_pathToFollow[indexPath];
  Vector2 v2;
  Vector2 nextPoint;
  int index = indexPath;
  int nextindex = index + 1;;
  if (indexPath == m_pathToFollow.size() - 1)
  {
    v2 = m_pathToFollow[0] - m_pathToFollow[index];
    float distance = v1.size();
    float compare = GameOptions::s_TileSizeX * .20f;
    if (distance<= compare)
    {
      m_bHaveObjetive = false;
      m_position = m_pathToFollow[index];
      return Vector2();
    }
  }
  else
  {
    nextPoint = m_pathToFollow[nextindex];
    v2 = m_pathToFollow[nextindex] - m_pathToFollow[index];
  }
  Vector2 dist = m_position - nextPoint;
  float distance = dist.size();
  if (distance <= Ratio)
  {
    indexPath++;
    if (indexPath == m_pathToFollow.size())
    {
      indexPath = 0;
    }
  }
  float proyection = Vector2::dot(v1, v2);
  proyection /= v2.size();
  if (proyection < 0)
  {
    proyection *= -1;
  }
  Vector2 pathPoint = (v2*proyection) + m_pathToFollow[indexPath];
  Vector2 F = seek(pathPoint, impetu);
  F += seek(nextPoint, impetu);
  return F;
}

Vector2 RTSUnit::flee(Vector2 PosB, float impetu)
{
  Vector2 Dir = m_position - PosB;
  Dir.normalize();
  Vector2 F = Dir * impetu;
  return F;
}

Vector2 RTSUnit::arrive(Vector2 PosB, float impetu, float ratio)
{
  //Vector2 Dir = PosB - m_position;
  //float tempImpetu = impetu;
  //if (Dir.length() < ratio)
  //{
  //  tempImpetu = Dir.length() / ratio;
  //}
  //Dir.normalize();
  //Vector2 F = m_direction;
  //Vector2 newDir = Dir;
  //newDir = truncar(newDir, impetu);
  //F = Inercia(newDir);
  //F *= impetu * tempImpetu;
  //return F;
  return Vector2();
}

void RTSUnit::SetAnimationData(RTSTexture * _rtsTextures, Vector<Animation> animations)
{
  m_textures = _rtsTextures;
  m_animations = animations;
}

void RTSUnit::UdpateAnimation()
{
  //if (m_pastState==m_currentState)
  //{
  //  return;
  //}
  
  string nameAnimation;
  switch (m_currentState)
  {
  case UNITSSTATES::KIDLESTATE:
    nameAnimation = "idle";
    break;
  case UNITSSTATES::KIRUNSTATE:
    nameAnimation = "run";
    break;
  case UNITSSTATES::KATTACKSTATE:
    nameAnimation = "attack";
    break;
  default:
    break;
  }

  if (m_directionView.y>-0.2&&m_directionView.y<0.2)
  {
    if (m_directionView.x<0)
    {
      nameAnimation += "_W";
    }
    else
    {
      nameAnimation += "_E";
    }
  }
  else if(m_directionView.x > -0.2&&m_directionView.x < 0.2)
  {
    if (m_directionView.y < 0)
    {
      nameAnimation += "_N";
    }
    else
    {
      nameAnimation += "_S";
    }
  }
  else
  {
    if (m_directionView.x > 0 && m_directionView.y > 0.2) {
      nameAnimation += "_SE";
    }
    else if (m_directionView.x > 0 && m_directionView.y < -0.2)
    {
      nameAnimation += "_NE";
    }
    else if (m_directionView.x < 0 && m_directionView.y > 0.2)
    {
      nameAnimation += "_SW";
    }
    else
    {
      nameAnimation += "_NW";
    }
  }
  for (int i = 0; i < m_animations.size(); i++)
  {
    
    if (m_animations[i].AnimationName == nameAnimation&&m_currentAnimation.AnimationName!= nameAnimation)
    {
      m_animationIdex = 0;
      m_timeElapsed = 0.0f;
      m_currentAnimation = m_animations[i];
      return;
    }
  }
}
