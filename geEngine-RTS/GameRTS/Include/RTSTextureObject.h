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
  onExit() {};
  

  
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

  RTSTexture &
  getTesturesUnitB() {
    return m_TexturesUnitsB;
  }

  Vector<Animation>
  getArcherAnimations()const {
    return m_ArcheriAnimations;
  }

  Vector<Animation>
  getwaterAnimations()const {
    return m_waterAnimations;
  }

  Vector<Animation>
  getflyAnimations()const {
    return m_flyAnimations;
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
  RTSTexture m_TexturesUnitsB;
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
  
  Animation m_waterRunN;
  Animation m_waterRunS;
  Animation m_waterRunNW;
  Animation m_waterRunSW;
  Animation m_waterRunW;
  Animation m_waterRunE;
  Animation m_waterRunNE;
  Animation m_waterRunSE;

  Animation m_waterIdleN;
  Animation m_waterIdleS;
  Animation m_waterIdleNW;
  Animation m_waterIdleSW;
  Animation m_waterIdleW;
  Animation m_waterIdleE;
  Animation m_waterIdleNE;
  Animation m_waterIdleSE;

  Animation m_flyRunN;
  Animation m_flyRunS;
  Animation m_flyRunNW;
  Animation m_flyRunSW;
  Animation m_flyRunW;
  Animation m_flyRunE;
  Animation m_flyRunNE;
  Animation m_flyRunSE;

  Animation m_flyIdleN;
  Animation m_flyIdleS;
  Animation m_flyIdleNW;
  Animation m_flyIdleSW;
  Animation m_flyIdleW;
  Animation m_flyIdleE;
  Animation m_flyIdleNE;
  Animation m_flyIdleSE;

  Vector<Animation> m_ArcheriAnimations;
  Vector<Animation> m_waterAnimations;
  Vector<Animation> m_flyAnimations;
  //Vector<Animation> m_ArcheriAnimations;
};
