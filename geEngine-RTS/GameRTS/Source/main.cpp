//#include <vld.h>

#if GE_PLATFORM == GE_PLATFORM_WIN32
# include <Win32/geWin32Windows.h>
#endif

#include "RTSApplication.h"

#include "lua.hpp"
#include <lauxlib.h>
#include <sol/sol.hpp>

using namespace geEngineSDK;

int main() {
  RTSApplication rtsApp;
  return rtsApp.run();
}
