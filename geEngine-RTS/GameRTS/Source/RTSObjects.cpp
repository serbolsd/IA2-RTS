#include "..\Include\RTSObjects.h"
#include "RTSTiledMap.h"
void 
Object::drawObject(TextureObject & texture, const int32& posX, const int32& posY)
{
  RTSTexture& refTexture = texture.getTesturesTrees();

  refTexture.setSrcRect(texture.getTesturesTreesData()[m_ID].x, texture.getTesturesTreesData()[m_ID].y,
                        texture.getTesturesTreesData()[m_ID].width, texture.getTesturesTreesData()[m_ID].high);
  refTexture.setPosition(posX, posY);
  refTexture.setOrigin(texture.getTesturesTreesData()[m_ID].width*0.25f, (texture.getTesturesTreesData()[m_ID].high*0.70f));

  refTexture.draw();
}
