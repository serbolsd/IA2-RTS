#pragma once
#include "RTSObjects.h"
#include "RTSTextureObject.h"

class RTSTree : public Object
{
public:
  RTSTree() { m_ytpe = TYPEOBJECT::UNIT; };
  ~RTSTree() {};
  void onInit();
  void draw();

  int32 m_tileIndexX;
  int32 m_tileIndexY;

  void
  setTextures(RTSTexture* pText) {
    m_textures = pText;
  };

  void
  setTextureData(const TextureData& pText) {
    m_data = pText;
  };

  void
  setTile(int x, int y);

private:

  RTSTexture* m_textures;
  TextureData m_data;


};