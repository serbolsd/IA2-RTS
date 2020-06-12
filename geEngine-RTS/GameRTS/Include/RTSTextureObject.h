#pragma once

#include "RTSTexture.h"
#include "Externals/json.hpp"

struct TextureData {
  String name;
  int x;
  int y;
  int width;
  int high;
  bool rotated;
  bool trimmed;
};

class TextureObject
{
 public:
  TextureObject() = default;
  ~TextureObject() {};
  void
  onInit(sf::RenderTarget* pTarget);

  RTSTexture &
  getTesturesTrees() {
    return m_TexturesTree;
  }

  Vector<TextureData> 
  getTesturesTreesData()const {
    return m_treesData;
  }

  nlohmann::json&
  getJsonTrees() {
    return m_treesJson;
  }
 private:
  RTSTexture m_TexturesTree;
  Vector<TextureData> m_treesData;
  nlohmann::json  m_treesJson;
  String m_dataTree;
};
