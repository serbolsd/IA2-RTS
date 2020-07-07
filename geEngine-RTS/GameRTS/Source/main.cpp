//#include <vld.h>

#if GE_PLATFORM == GE_PLATFORM_WIN32
# include <Win32/geWin32Windows.h>
#endif

#include "RTSApplication.h"

#include "lua.hpp"
using namespace geEngineSDK;

lua_State* g_luaState = nullptr;

bool initScipringSystem()
{
  g_luaState = luaL_newstate();
  if (nullptr == g_luaState)
  {
    return false;
  }
  luaL_openlibs(g_luaState);
  luaL_dofile(g_luaState,"config.lua");
  lua_getglobal(g_luaState,"GameOptions::s_MapIsIsometric");
  bool isIso = lua_toboolean(g_luaState,-1);
  if (isIso)
  {
    std::cout << "is isometric";
  }
  else
  {
    std::cout << "is normal";
  }

  //lua_Number X = lua_tonumber(g_luaState,1);
  //int xx = (int )X;
  //X = lua_tonumber(g_luaState, 2);
  //xx = (int)X;
  lua_close(g_luaState);
  return true;
}


int main() {
  //if (!initScipringSystem())
  //{
  //  return 0;
  //}
  RTSApplication rtsApp;
  return rtsApp.run();
}
