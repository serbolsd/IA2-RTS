#pragma once

//Define only if the map should be isometric
//#define GameOptions::s_MapIsIsometric

//#if GameOptions::s_MapIsIsometric
////Tiles pixel size
//# define GameOptions::s_TileSizeX 0
//# define GameOptions::s_TileSizeY 0
//#else
////Tiles pixel size
//# define GameOptions::s_TileSizeX 0
//# define GameOptions::s_TileSizeY 0
//#endif



#include <geModule.h>
#include <geVector2I.h>
#include <geVector2.h>
#include <geColor.h>

#include <lua.hpp>

using namespace geEngineSDK;

class GameOptions : public Module<GameOptions>
{
 public:
  //Application values
  static Vector2I s_Resolution;
  static Vector2 s_MapMovementSpeed;
  static Vector2I s_CameraPosition;

  static bool s_MapIsIsometric;

  static int s_TileSizeX;
  static int s_TileSizeY;

  static int s_MapSizeX;
  static int s_MapSizeY;

  static bool s_bFullScreen;
  static String s_windowName;
  //World Values

  //Map Values
  static bool s_MapShowGrid;
  static Color s_MapGridColor;

  //Global Constants
  static Vector2I TILEHALFSIZE;
  static Vector2I BITSFHT_TILEHALFSIZE;
  static Vector2I BITSHFT_TILESIZE;
};

GameOptions&
g_gameOptions();
