#include "RTSTree.h"
#include "RTSFunctionsCoords.h"

void RTSTree::draw()
{
  RTSTexture& refTexture = *m_textures;

  refTexture.setSrcRect(m_data.x, m_data.y,
    m_data.width, m_data.high);

  Vector2I posI;
  COORDS::getMapToPixelCoords(m_tileIndexX, m_tileIndexY, posI.x, posI.y);
  m_position.x = posI.x;
  m_position.y = posI.y;
  Vector2 posScreen;
  COORDS::getPixelToScreenCoords(static_cast<int32>(m_position.x), static_cast<int32>(m_position.y), posScreen.x, posScreen.y);
  refTexture.setPosition(posScreen.x, posScreen.y);
  refTexture.setOrigin(m_data.width*0.25f, (m_data.high*0.70f));

  refTexture.draw();
}

void RTSTree::setTile(int x, int y)
{
  Vector2I posI;
  COORDS::getMapToPixelCoords(x,y, posI.x, posI.y);
  m_position.x = posI.x;
  m_position.y = posI.y;
  m_tileIndexX = x;
  m_tileIndexY = y;
}
