#include "..\Include\RTSObjects.h"

void 
Object::drawObject(TextureObject & texture, const int32& posX, const int32& posY)
{
  RTSTexture& refTexture = texture.getTesturesTrees();

  refTexture.setPosition(posX, posY);
  refTexture.setSrcRect(texture.getTesturesTreesData()[m_ID].x, texture.getTesturesTreesData()[m_ID].y,
                        texture.getTesturesTreesData()[m_ID].width, texture.getTesturesTreesData()[m_ID].high);
  refTexture.setOrigin(texture.getTesturesTreesData()[m_ID].width*.25, texture.getTesturesTreesData()[m_ID].high*0.5f);

  refTexture.draw();
}
