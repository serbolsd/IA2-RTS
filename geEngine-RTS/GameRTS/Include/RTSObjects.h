#pragma once
#include <gePlatformUtility.h>
#include <geVector2.h>
#include <geVector2I.h>

#include <SFML/Graphics.hpp>

#include "RTSConfig.h"
#include "RTSTextureObject.h"

enum TYPEOBJECT {
  TREE=0,
  UNIT,
  BUILD,
  UNKNOWOBJECTTYPE
};

class Object {
 public:
  Object() = default;
  virtual ~Object() {};
  
  virtual void
  setTile(void* tile) {
    m_myTile = tile;
  }

  virtual void *
  getTile() {
    return m_myTile;
  }

  virtual void
  setType(TYPEOBJECT type) {
    m_ytpe = type;
  }

  virtual TYPEOBJECT
  getType() {
    return m_ytpe;
  }

  virtual void
  setPosition(const Vector2& pos) {
    m_position = pos;
  }

  virtual Vector2
  getPosition() const {
    return m_position;
  }

  virtual void
  drawObject(TextureObject& texture, const int32& posX, const int32& posY);

  void
  setIDofData(int id) {
    m_ID = id;
  }
  int m_ID;
  void* m_myTile;
  Vector2 m_position;
  TYPEOBJECT m_ytpe=UNKNOWOBJECTTYPE;
 private:
};
