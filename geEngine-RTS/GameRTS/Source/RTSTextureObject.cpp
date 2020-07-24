#include "..\Include\RTSTextureObject.h"
#include <fstream>
#include "RTSUnitType.h"

void 
TextureObject::onInit(sf::RenderTarget* pTarget) {

  initAnimationData();
  std::ifstream i("trees.json");
  i >> m_treesJson;
  auto frames = m_treesJson["frames"];

  for (nlohmann::json::iterator it = m_treesJson["frames"].begin(); it != m_treesJson["frames"].end(); ++it) {
    TextureData tempTexDara;
    tempTexDara.name = it.key();
    tempTexDara.x= it.value()["frame"]["x"];
    tempTexDara.y= it.value()["frame"]["y"];
    tempTexDara.width= it.value()["frame"]["w"];
    tempTexDara.high= it.value()["frame"]["h"];
    tempTexDara.rotated= it.value()["rotated"];
    tempTexDara.trimmed = it.value()["trimmed"];
    m_treesData.push_back(tempTexDara);
  }

  String textureName = "Textures/Objects/trees.png";
  m_TexturesTree.loadFromFile(pTarget, textureName);
  i.close();

  std::ifstream units("feudalWarUnits.json");
  units >> m_unitsJson;
  frames = m_unitsJson["frames"];
  RTSGame::UNITTYPE::E unitType = RTSGame::UNITTYPE::KNUMUNITTYPES;
  RTSGame::DIRECTIONS::E dir = RTSGame::DIRECTIONS::kNUM_DIRECTIONS;

  for (nlohmann::json::iterator it = m_unitsJson["frames"].begin(); it != m_unitsJson["frames"].end(); ++it) {
    TextureData tempTexDara;
    tempTexDara.name = it.key();
    std::string key = it.key();
    
    tempTexDara.x = it.value()["frame"]["x"];
    tempTexDara.y = it.value()["frame"]["y"];
    tempTexDara.width = it.value()["frame"]["w"];
    tempTexDara.high = it.value()["frame"]["h"];
    tempTexDara.rotated = it.value()["rotated"];
    tempTexDara.trimmed = it.value()["trimmed"];

    RTSGame::DIRECTIONS::E directionFrame;

    auto dir = key.find("_N");
    dir = key.find("_N");
    if (std::string::npos != dir)
    {
      directionFrame = RTSGame::DIRECTIONS::kN;
    }
    dir = key.find("_NW");
    if (std::string::npos != dir)
    {
      directionFrame = RTSGame::DIRECTIONS::kNW;
    }
    dir = key.find("_S");
    if (std::string::npos != dir)
    {
      directionFrame = RTSGame::DIRECTIONS::kS;
    }
    dir = key.find("_SW");
    if (std::string::npos != dir)
    {
      directionFrame = RTSGame::DIRECTIONS::kSW;
    }
    dir = key.find("_W");
    if (std::string::npos != dir)
    {
      directionFrame = RTSGame::DIRECTIONS::kW;
    }
    auto pos = key.find("crossbow");
    if (std::string::npos != pos ) {
      addUnitFrame("crossbow", key, tempTexDara, directionFrame);
    }
    pos = key.find("heavy_pikeman");
    if (std::string::npos != pos)
    {
      addUnitFrame("heavy_pikeman", key, tempTexDara, directionFrame);
      //unitType = RTSGame::UNITTYPE::KTERRESTRIAL;
    }
    pos = key.find("mounted_hand_gunner");
    if (std::string::npos != pos)
    {
        addUnitFrame("mounted_hand_gunner", key, tempTexDara, directionFrame);
      //unitType = RTSGame::UNITTYPE::KTERRESTRIAL;
    }
  }

  textureName = "Textures/Units/units.png";
  m_TexturesUnits.loadFromFile(pTarget, textureName);
  units.close();

  textureName = "Textures/Units/units-2.png";
  m_TexturesUnitsB.loadFromFile(pTarget, textureName);

  m_ArcheriAnimations.push_back(m_archerRunN);
  m_ArcheriAnimations.push_back(m_archerRunS);
  m_ArcheriAnimations.push_back(m_archerRunNW);
  m_ArcheriAnimations.push_back(m_archerRunSW);
  m_ArcheriAnimations.push_back(m_archerRunW);
  m_ArcheriAnimations.push_back(m_archerRunE);
  m_ArcheriAnimations.push_back(m_archerRunNE);
  m_ArcheriAnimations.push_back(m_archerRunSE);
  m_ArcheriAnimations.push_back(m_archerIdleN);
  m_ArcheriAnimations.push_back(m_archerIdleS);
  m_ArcheriAnimations.push_back(m_archerIdleNW);
  m_ArcheriAnimations.push_back(m_archerIdleSW);
  m_ArcheriAnimations.push_back(m_archerIdleW);
  m_ArcheriAnimations.push_back(m_archerIdleE);
  m_ArcheriAnimations.push_back(m_archerIdleNE);
  m_ArcheriAnimations.push_back(m_archerIdleSE);

  m_waterAnimations.push_back(m_waterRunN);
  m_waterAnimations.push_back(m_waterRunS);
  m_waterAnimations.push_back(m_waterRunNW);
  m_waterAnimations.push_back(m_waterRunSW);
  m_waterAnimations.push_back(m_waterRunW);
  m_waterAnimations.push_back(m_waterRunE);
  m_waterAnimations.push_back(m_waterRunNE);
  m_waterAnimations.push_back(m_waterRunSE);
  m_waterAnimations.push_back(m_waterIdleN);
  m_waterAnimations.push_back(m_waterIdleS);
  m_waterAnimations.push_back(m_waterIdleNW);
  m_waterAnimations.push_back(m_waterIdleSW);
  m_waterAnimations.push_back(m_waterIdleW);
  m_waterAnimations.push_back(m_waterIdleE);
  m_waterAnimations.push_back(m_waterIdleNE);
  m_waterAnimations.push_back(m_waterIdleSE);

  m_flyAnimations.push_back(m_flyRunN);
  m_flyAnimations.push_back(m_flyRunS);
  m_flyAnimations.push_back(m_flyRunNW);
  m_flyAnimations.push_back(m_flyRunSW);
  m_flyAnimations.push_back(m_flyRunW);
  m_flyAnimations.push_back(m_flyRunE);
  m_flyAnimations.push_back(m_flyRunNE);
  m_flyAnimations.push_back(m_flyRunSE);
  m_flyAnimations.push_back(m_flyIdleN);
  m_flyAnimations.push_back(m_flyIdleS);
  m_flyAnimations.push_back(m_flyIdleNW);
  m_flyAnimations.push_back(m_flyIdleSW);
  m_flyAnimations.push_back(m_flyIdleW);
  m_flyAnimations.push_back(m_flyIdleE);
  m_flyAnimations.push_back(m_flyIdleNE);
  m_flyAnimations.push_back(m_flyIdleSE);

}

void TextureObject::initAnimationData()
{
  m_archerRunN.direction= RTSGame::DIRECTIONS::kN;
  m_archerRunS.direction = RTSGame::DIRECTIONS::kS;
  m_archerRunNW.direction = RTSGame::DIRECTIONS::kNW;
  m_archerRunSW.direction = RTSGame::DIRECTIONS::kSW;
  m_archerRunW.direction = RTSGame::DIRECTIONS::kW;
  m_archerRunE.direction = RTSGame::DIRECTIONS::kE;
  m_archerRunNE.direction = RTSGame::DIRECTIONS::kNE;
  m_archerRunSE.direction = RTSGame::DIRECTIONS::kNW;

  m_archerRunN.AnimationName ="run_N";
  m_archerRunS.AnimationName = "run_S";
  m_archerRunNW.AnimationName = "run_NW";
  m_archerRunSW.AnimationName = "run_SW";
  m_archerRunW.AnimationName = "run_W";
  m_archerRunE.AnimationName = "run_E";
  m_archerRunNE.AnimationName = "run_NE";
  m_archerRunSE.AnimationName = "run_SE";

  m_archerIdleN.direction = RTSGame::DIRECTIONS::kN;
  m_archerIdleS.direction = RTSGame::DIRECTIONS::kS;
  m_archerIdleNW.direction = RTSGame::DIRECTIONS::kNW;
  m_archerIdleSW.direction = RTSGame::DIRECTIONS::kSW;
  m_archerIdleW.direction = RTSGame::DIRECTIONS::kW;
  m_archerIdleE.direction = RTSGame::DIRECTIONS::kE;
  m_archerIdleNE.direction = RTSGame::DIRECTIONS::kNE;
  m_archerIdleSE.direction = RTSGame::DIRECTIONS::kNW;

  m_archerIdleN .AnimationName = "idle_N";
  m_archerIdleS .AnimationName = "idle_S";
  m_archerIdleNW.AnimationName = "idle_NW";
  m_archerIdleSW.AnimationName = "idle_SW";
  m_archerIdleW .AnimationName = "idle_W";
  m_archerIdleE .AnimationName = "idle_E";
  m_archerIdleNE.AnimationName = "idle_NE";
  m_archerIdleSE.AnimationName = "idle_SE";


  m_flyRunN.direction = RTSGame::DIRECTIONS::kN;
  m_flyRunS.direction = RTSGame::DIRECTIONS::kS;
  m_flyRunNW.direction = RTSGame::DIRECTIONS::kNW;
  m_flyRunSW.direction = RTSGame::DIRECTIONS::kSW;
  m_flyRunW.direction = RTSGame::DIRECTIONS::kW;
  m_flyRunE.direction = RTSGame::DIRECTIONS::kE;
  m_flyRunNE.direction = RTSGame::DIRECTIONS::kNE;
  m_flyRunSE.direction = RTSGame::DIRECTIONS::kNW;

  m_flyRunN.AnimationName = "run_N";
  m_flyRunS.AnimationName = "run_S";
  m_flyRunNW.AnimationName = "run_NW";
  m_flyRunSW.AnimationName = "run_SW";
  m_flyRunW.AnimationName = "run_W";
  m_flyRunE.AnimationName = "run_E";
  m_flyRunNE.AnimationName = "run_NE";
  m_flyRunSE.AnimationName = "run_SE";

  m_flyIdleN.direction = RTSGame::DIRECTIONS::kN;
  m_flyIdleS.direction = RTSGame::DIRECTIONS::kS;
  m_flyIdleNW.direction = RTSGame::DIRECTIONS::kNW;
  m_flyIdleSW.direction = RTSGame::DIRECTIONS::kSW;
  m_flyIdleW.direction = RTSGame::DIRECTIONS::kW;
  m_flyIdleE.direction = RTSGame::DIRECTIONS::kE;
  m_flyIdleNE.direction = RTSGame::DIRECTIONS::kNE;
  m_flyIdleSE.direction = RTSGame::DIRECTIONS::kNW;

  m_flyIdleN.AnimationName = "idle_N";
  m_flyIdleS.AnimationName = "idle_S";
  m_flyIdleNW.AnimationName = "idle_NW";
  m_flyIdleSW.AnimationName = "idle_SW";
  m_flyIdleW.AnimationName = "idle_W";
  m_flyIdleE.AnimationName = "idle_E";
  m_flyIdleNE.AnimationName = "idle_NE";
  m_flyIdleSE.AnimationName = "idle_SE";


  m_waterRunN.direction = RTSGame::DIRECTIONS::kN;
  m_waterRunS.direction = RTSGame::DIRECTIONS::kS;
  m_waterRunNW.direction = RTSGame::DIRECTIONS::kNW;
  m_waterRunSW.direction = RTSGame::DIRECTIONS::kSW;
  m_waterRunW.direction = RTSGame::DIRECTIONS::kW;
  m_waterRunE.direction = RTSGame::DIRECTIONS::kE;
  m_waterRunNE.direction = RTSGame::DIRECTIONS::kNE;
  m_waterRunSE.direction = RTSGame::DIRECTIONS::kNW;

  m_waterRunN.AnimationName = "run_N";
  m_waterRunS.AnimationName = "run_S";
  m_waterRunNW.AnimationName = "run_NW";
  m_waterRunSW.AnimationName = "run_SW";
  m_waterRunW.AnimationName = "run_W";
  m_waterRunE.AnimationName = "run_E";
  m_waterRunNE.AnimationName = "run_NE";
  m_waterRunSE.AnimationName = "run_SE";

  m_waterIdleN.direction = RTSGame::DIRECTIONS::kN;
  m_waterIdleS.direction = RTSGame::DIRECTIONS::kS;
  m_waterIdleNW.direction = RTSGame::DIRECTIONS::kNW;
  m_waterIdleSW.direction = RTSGame::DIRECTIONS::kSW;
  m_waterIdleW.direction = RTSGame::DIRECTIONS::kW;
  m_waterIdleE.direction = RTSGame::DIRECTIONS::kE;
  m_waterIdleNE.direction = RTSGame::DIRECTIONS::kNE;
  m_waterIdleSE.direction = RTSGame::DIRECTIONS::kNW;

  m_waterIdleN.AnimationName = "idle_N";
  m_waterIdleS.AnimationName = "idle_S";
  m_waterIdleNW.AnimationName = "idle_NW";
  m_waterIdleSW.AnimationName = "idle_SW";
  m_waterIdleW.AnimationName = "idle_W";
  m_waterIdleE.AnimationName = "idle_E";
  m_waterIdleNE.AnimationName = "idle_NE";
  m_waterIdleSE.AnimationName = "idle_SE";
}


void TextureObject::addUnitFrame(const std::string & unitName, const std::string & frameName, const TextureData & tempTexDara, RTSGame::DIRECTIONS::E _dir)
{
  string name;

 

   TextureData  tempTexDaraRotate= tempTexDara;
   tempTexDaraRotate.rotated = true;
  if ("crossbow" == unitName)
  {
    auto pos = frameName.find("idle");
    if (std::string::npos != pos) {
      switch (_dir)
      {
      case RTSGame::DIRECTIONS::kN:
        m_archerIdleN.frameData.push_back(tempTexDara);
        break;
      case RTSGame::DIRECTIONS::kNW:
        m_archerIdleNW.frameData.push_back(tempTexDara);
        m_archerIdleNE.frameData.push_back(tempTexDaraRotate);
        break;
      case RTSGame::DIRECTIONS::kS:
        m_archerIdleS.frameData.push_back(tempTexDara);
        break;
      case RTSGame::DIRECTIONS::kSW:
        m_archerIdleSW.frameData.push_back(tempTexDara);
        m_archerIdleSE.frameData.push_back(tempTexDaraRotate);
        break;
      case RTSGame::DIRECTIONS::kW:
        m_archerIdleW.frameData.push_back(tempTexDara);
        m_archerIdleE.frameData.push_back(tempTexDaraRotate);
        break;
      default:
        break;
      }
    }
    pos = frameName.find("run");
    if (std::string::npos != pos) {
      switch (_dir)
      {
      case RTSGame::DIRECTIONS::kN:
        m_archerRunN.frameData.push_back(tempTexDara);
        break;
      case RTSGame::DIRECTIONS::kNW:
        m_archerRunNW.frameData.push_back(tempTexDara);
        m_archerRunNE.frameData.push_back(tempTexDaraRotate);
        break;
      case RTSGame::DIRECTIONS::kS:
        m_archerRunS.frameData.push_back(tempTexDara);
        break;
      case RTSGame::DIRECTIONS::kSW:
        m_archerRunSW.frameData.push_back(tempTexDara);
        m_archerRunSE.frameData.push_back(tempTexDaraRotate);
        break;
      case RTSGame::DIRECTIONS::kW:
        m_archerRunW.frameData.push_back(tempTexDara);
        m_archerRunE.frameData.push_back(tempTexDaraRotate);
        break;
      default:
        break;
      }
    }
  }
  else if ("heavy_pikeman" == unitName)//water
  {
    auto pos = frameName.find("idle");
    if (std::string::npos != pos) {
      switch (_dir)
      {
      case RTSGame::DIRECTIONS::kN:
        m_waterIdleN.frameData.push_back(tempTexDara);
        break;
      case RTSGame::DIRECTIONS::kNW:
        m_waterIdleNW.frameData.push_back(tempTexDara);
        m_waterIdleNE.frameData.push_back(tempTexDaraRotate);
        break;
      case RTSGame::DIRECTIONS::kS:
        m_waterIdleS.frameData.push_back(tempTexDara);
        break;
      case RTSGame::DIRECTIONS::kSW:
        m_waterIdleSW.frameData.push_back(tempTexDara);
        m_waterIdleSE.frameData.push_back(tempTexDaraRotate);
        break;
      case RTSGame::DIRECTIONS::kW:
        m_waterIdleW.frameData.push_back(tempTexDara);
        m_waterIdleE.frameData.push_back(tempTexDaraRotate);
        break;
      default:
        break;
      }
    }
    pos = frameName.find("run");
    if (std::string::npos != pos) {
      switch (_dir)
      {
      case RTSGame::DIRECTIONS::kN:
        m_waterRunN.frameData.push_back(tempTexDara);
        break;
      case RTSGame::DIRECTIONS::kNW:
        m_waterRunNW.frameData.push_back(tempTexDara);
        m_waterRunNE.frameData.push_back(tempTexDaraRotate);
        break;
      case RTSGame::DIRECTIONS::kS:
        m_waterRunS.frameData.push_back(tempTexDara);
        break;
      case RTSGame::DIRECTIONS::kSW:
        m_waterRunSW.frameData.push_back(tempTexDara);
        m_waterRunSE.frameData.push_back(tempTexDaraRotate);
        break;
      case RTSGame::DIRECTIONS::kW:
        m_waterRunW.frameData.push_back(tempTexDara);
        m_waterRunE.frameData.push_back(tempTexDaraRotate);
        break;
      default:
        break;
      }
    }
  }
  else if ("mounted_hand_gunner" == unitName)
  {
    auto pos = frameName.find("idle");
    if (std::string::npos != pos) {
      switch (_dir)
      {
      case RTSGame::DIRECTIONS::kN:
        m_flyIdleN.frameData.push_back(tempTexDara);
        break;
      case RTSGame::DIRECTIONS::kNW:
        m_flyIdleNW.frameData.push_back(tempTexDara);
        m_flyIdleNE.frameData.push_back(tempTexDaraRotate);
        break;
      case RTSGame::DIRECTIONS::kS:
        m_flyIdleS.frameData.push_back(tempTexDara);
        break;
      case RTSGame::DIRECTIONS::kSW:
        m_flyIdleSW.frameData.push_back(tempTexDara);
        m_flyIdleSE.frameData.push_back(tempTexDaraRotate);
        break;
      case RTSGame::DIRECTIONS::kW:
        m_flyIdleW.frameData.push_back(tempTexDara);
        m_flyIdleE.frameData.push_back(tempTexDaraRotate);
        break;
      default:
        break;
      }
    }
    pos = frameName.find("run");
    if (std::string::npos != pos) {
      switch (_dir)
      {
      case RTSGame::DIRECTIONS::kN:
        m_flyRunN.frameData.push_back(tempTexDara);
        break;
      case RTSGame::DIRECTIONS::kNW:
        m_flyRunNW.frameData.push_back(tempTexDara);
        m_flyRunNE.frameData.push_back(tempTexDaraRotate);
        break;
      case RTSGame::DIRECTIONS::kS:
        m_flyRunS.frameData.push_back(tempTexDara);
        break;
      case RTSGame::DIRECTIONS::kSW:
        m_flyRunSW.frameData.push_back(tempTexDara);
        m_flyRunSE.frameData.push_back(tempTexDaraRotate);
        break;
      case RTSGame::DIRECTIONS::kW:
        m_flyRunW.frameData.push_back(tempTexDara);
        m_flyRunE.frameData.push_back(tempTexDaraRotate);
        break;
      default:
        break;
      }
    }
  }

}
