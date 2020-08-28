#pragma once
#include <gePlatformUtility.h>
#include <geVector2.h>
#include <geVector2I.h>

#include <SFML/Graphics.hpp>

#include "RTSConfig.h"
namespace COORDS {
  static Vector2I PreCalc_MidResolution;
  static Vector2I PreCalc_ScreenDeface;
  FORCEINLINE void
  getScreenToMapCoords(const int32 scrX, const int32 scrY, int32 &mapX, int32 &mapY) {

    if (GameOptions::s_MapIsIsometric)
    {
      float fscrX = ((float)(scrX - PreCalc_ScreenDeface.x) / GameOptions::TILEHALFSIZE.x) - 1;
      float fscrY = ((float)(scrY - PreCalc_ScreenDeface.y) / GameOptions::TILEHALFSIZE.y);

      mapX = (Math::trunc(fscrX + fscrY)) >> 1;
      mapY = (Math::trunc(fscrY - fscrX)) >> 1;
    }
    else
    {
      mapX = (scrX - PreCalc_ScreenDeface.x) >> GameOptions::BITSHFT_TILESIZE.x;
      mapY = (scrY - PreCalc_ScreenDeface.y) >> GameOptions::BITSHFT_TILESIZE.y;
    }

    mapX = Math::clamp(mapX, 0, GameOptions::s_MapSizeX - 1);
    mapY = Math::clamp(mapY, 0, GameOptions::s_MapSizeY - 1);
  }

  //para imprimir convierte en una posicion para dibujar
  FORCEINLINE void
  getMapToScreenCoords(const int32 mapX, const int32 mapY, int32 &scrX, int32 &scrY) {
    GE_ASSERT(mapX >= 0 && mapX <= GameOptions::s_MapSizeX && mapY >= 0 && mapY <= GameOptions::s_MapSizeY);

    if (GameOptions::s_MapIsIsometric)
    {
      scrX = (mapX - mapY) << GameOptions::BITSFHT_TILEHALFSIZE.x;
      scrY = (mapX + mapY) << GameOptions::BITSFHT_TILEHALFSIZE.y;

      scrX += PreCalc_ScreenDeface.x;
      scrY += PreCalc_ScreenDeface.y;
    }
    else
    {
      scrX = (mapX << GameOptions::BITSHFT_TILESIZE.x);
      scrY = (mapY << GameOptions::BITSHFT_TILESIZE.y);
      scrX += PreCalc_ScreenDeface.x;
      scrY += PreCalc_ScreenDeface.y;
    }
  }

  FORCEINLINE void
  getMapToPixelCoords(const int32 mapX, const int32 mapY, int32 &pixX, int32 &pixY) {
    Vector2I pixelPos = { mapX * GameOptions::s_TileSizeX, mapY*GameOptions::s_TileSizeY};

    GE_ASSERT(mapX >= 0 && mapX <= GameOptions::s_MapSizeX && mapY >= 0 && mapY <= GameOptions::s_MapSizeY);
    pixX = pixelPos.x;
    pixY = pixelPos.y;
  }

  FORCEINLINE void
  getTileCenterOnPixelCoords(const int32 tileX, const int32 tileY, int32 &pixX, int32 &pixY) {
    Vector2I pixelPos;

    GE_ASSERT(tileX >= 0 && tileX <= GameOptions::s_MapSizeX && tileY >= 0 && tileY <= GameOptions::s_MapSizeY);
    if (GameOptions::s_MapIsIsometric)
    {
      Vector2I tilePixelPos = { tileX * GameOptions::s_TileSizeX, tileY * GameOptions::s_TileSizeY/2};
      tilePixelPos.x = (tileX - tileY) << GameOptions::BITSFHT_TILEHALFSIZE.x;
      tilePixelPos.y = (tileX + tileY) << GameOptions::BITSFHT_TILEHALFSIZE.y;
      pixelPos.x = tilePixelPos.x+GameOptions::TILEHALFSIZE.x;
      pixelPos.y = tilePixelPos.y+GameOptions::TILEHALFSIZE.y ;
      pixX = pixelPos.x;
      pixY = pixelPos.y;
      //pixX = pixelPos.x;
      //pixY = pixelPos.y;
     
    }
    else
    {
      pixelPos = { tileX * GameOptions::s_TileSizeX, tileY * GameOptions::s_TileSizeY };
      pixX = pixelPos.x + GameOptions::s_TileSizeX/2;
      pixY = pixelPos.y+ ((GameOptions::s_TileSizeY / 2));

    }
  }

  FORCEINLINE void
  getPixelToMapCoords(const int32 pixX, const int32 pixY, int32 &mapX, int32 &mapY) {
    Vector2I mapCoord = { pixX / GameOptions::s_TileSizeX, pixY /GameOptions::s_TileSizeY };

    //GE_ASSERT(mapCoord.x >= 0 && mapCoord.x <= GameOptions::s_MapSizeX && mapCoord.y >= 0 && mapCoord.y <= GameOptions::s_MapSizeY);
    mapX = mapCoord.x;
    mapY = mapCoord.y;
  }

  FORCEINLINE void
  getPixelToScreenCoords(const int32 pixX, const int32 pixY, float &scrX, float &scrY) {
    Vector2 pixCoord;
    if (GameOptions::s_MapIsIsometric)
    {
      //Vector2I tileCoord = { (pixX + GameOptions::TILEHALFSIZE.x) / GameOptions::s_TileSizeX,
      //                       (pixY + GameOptions::TILEHALFSIZE.y) / GameOptions::s_TileSizeY };
      //getPixelToMapCoords(pixX, pixY, tileCoord.x, tileCoord.y);
      //tileCoord.x += 1;
      //pixCoord.x = (tileCoord.x - tileCoord.y) << GameOptions::BITSFHT_TILEHALFSIZE.x;
      //pixCoord.y = (tileCoord.x + tileCoord.y) << GameOptions::BITSFHT_TILEHALFSIZE.y;
      pixCoord.x = (float)pixX ;
      pixCoord.y = (float)pixY ;

      scrX = pixCoord.x + PreCalc_ScreenDeface.x;
      scrY = pixCoord.y + PreCalc_ScreenDeface.y;
    }
    else
    { 
      pixCoord = { float(pixX),float(pixY) };
      scrX = (pixCoord.x) + PreCalc_ScreenDeface.x;
      scrY = (pixCoord.y) + PreCalc_ScreenDeface.y;
    }

  }

  FORCEINLINE void
  getScreenToPixelCoords(const int32 scrX, const int32 scrY, int32 &pixX, int32 &pixY) {

    if (GameOptions::s_MapIsIsometric)
    {
      pixX = (scrX)-PreCalc_ScreenDeface.x;
      pixY = (scrY)-PreCalc_ScreenDeface.y;
    }
    else
    {
      pixX = (scrX)-PreCalc_ScreenDeface.x;
      pixY = (scrY)-PreCalc_ScreenDeface.y;
    }
  }
}