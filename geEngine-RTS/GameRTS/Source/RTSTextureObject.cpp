#include "..\Include\RTSTextureObject.h"
#include <fstream>

void 
TextureObject::onInit(sf::RenderTarget* pTarget) {
  std::ifstream i("trees.json");
  i >> m_treesJson;
  auto frames = m_treesJson["frames"];
  int size = frames.size();

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
}
