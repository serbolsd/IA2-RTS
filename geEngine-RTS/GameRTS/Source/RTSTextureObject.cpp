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
    auto pos = key.find("archer");
    if (std::string::npos != pos ) {
      addUnitFrame("archer", key, tempTexDara, directionFrame);
    }
    pos = key.find("castilian_knight");
    if (std::string::npos != pos)
    {
      unitType = RTSGame::UNITTYPE::KTERRESTRIAL;
    }
  }

  textureName = "Textures/Units/units.png";
  m_TexturesUnits.loadFromFile(pTarget, textureName);
  units.close();

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
}


void TextureObject::addUnitFrame(const std::string & unitName, const std::string & frameName, const TextureData & tempTexDara, RTSGame::DIRECTIONS::E _dir)
{
  string name;

 

   TextureData  tempTexDaraRotate= tempTexDara;
   tempTexDaraRotate.rotated = true;
  if ("archer" == unitName)
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

}
