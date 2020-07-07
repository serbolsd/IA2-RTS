#include "RTSConfig.h"



Vector2I GameOptions::s_Resolution = Vector2I(0, 0);
Vector2 GameOptions::s_MapMovementSpeed = Vector2(0, 0);
Vector2I GameOptions::s_CameraPosition = Vector2I(0, 0);
bool GameOptions::s_MapIsIsometric = false;

int GameOptions::s_TileSizeX = 64;
int GameOptions::s_TileSizeY = 64;

int GameOptions::s_MapSizeX = 5;
int GameOptions::s_MapSizeY = 5;

bool GameOptions::s_bFullScreen = false;;
String GameOptions::s_windowName ="";


bool GameOptions::s_MapShowGrid = false;
Color GameOptions::s_MapGridColor = Color(0, 0, 0, 0);

Vector2I
GameOptions::TILEHALFSIZE = Vector2I(GameOptions::s_TileSizeX >> 1, GameOptions::s_TileSizeY >> 1);

Vector2I
GameOptions::BITSHFT_TILESIZE = Vector2I(
  Math::countTrailingZeros(GameOptions::s_TileSizeX),
  Math::countTrailingZeros(GameOptions::s_TileSizeY)
);

Vector2I
GameOptions::BITSFHT_TILEHALFSIZE = Vector2I(GameOptions::BITSHFT_TILESIZE.x - 1,
                                             GameOptions::BITSHFT_TILESIZE.y - 1);


GameOptions&
g_gameOptions() {
  return GameOptions::instance();
}