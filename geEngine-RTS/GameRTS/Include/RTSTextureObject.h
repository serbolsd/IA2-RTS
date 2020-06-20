#pragma once

#include "RTSTexture.h"
#include "Externals/json.hpp"
#include "RTSUnitType.h"

struct TextureData {
  String name;
  int x;
  int y;
  int width;
  int high;
  bool rotated;
  bool trimmed;
};

struct Animation {
  string AnimationName;
  RTSGame::DIRECTIONS::E direction;
  Vector<TextureData> frameData;
};

class TextureObject
{
 public:
  TextureObject() = default;
  ~TextureObject() {};
  void
  onInit(sf::RenderTarget* pTarget);

  void 
  onExit();
  

  
  RTSTexture &
  getTesturesTrees() {
    return m_TexturesTree;
  }

  Vector<TextureData> 
  getTesturesTreesData()const {
    return m_treesData;
  }
  RTSTexture &
    getTesturesUnit() {
    return m_TexturesUnits;
  }

  Vector<Animation>
    getArcherAnimations()const {
    return m_ArcheriAnimations;
  }
 private:

   void initAnimationData();

   void
   addUnitFrame(const std::string& unitName, 
                const std::string& frameName, 
                const TextureData& tempTexDara, 
                RTSGame::DIRECTIONS::E _dir);

  RTSTexture m_TexturesTree;
  Vector<TextureData> m_treesData;
  nlohmann::json  m_treesJson;
  String m_dataTree;

  RTSTexture m_TexturesUnits;
  Vector<TextureData> m_unitsData;
  nlohmann::json  m_unitsJson;
  String m_dataUnit;

  vector<TextureData> m_ArcherRunFrameData;
  Animation m_archerRunN;
  Animation m_archerRunS;
  Animation m_archerRunNW;
  Animation m_archerRunSW;
  Animation m_archerRunW;
  Animation m_archerRunE;
  Animation m_archerRunNE;
  Animation m_archerRunSE;

  Animation m_archerIdleN;
  Animation m_archerIdleS;
  Animation m_archerIdleNW;
  Animation m_archerIdleSW;
  Animation m_archerIdleW;
  Animation m_archerIdleE;
  Animation m_archerIdleNE;
  Animation m_archerIdleSE;
  
  Vector<Animation> m_ArcheriAnimations;
  //Vector<Animation> m_ArcheriAnimations;
};
