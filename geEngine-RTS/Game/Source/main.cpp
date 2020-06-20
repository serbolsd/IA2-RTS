//#include <vld.h>
#include <iostream>

#if GE_PLATFORM == GE_PLATFORM_WIN32
#include <Win32/geWin32Windows.h>
#endif

#include <gePrerequisitesUtil.h>
#include <geRTTIPlainField.h>
#include <geException.h>

#include <geCrashHandler.h>
#include <geDynLibManager.h>
#include <geTime.h>

using namespace geEngineSDK;

int
main() {
  CrashHandler::startUp();
  DynLibManager::startUp();
  Time::startUp();

  __try {
    //EditorApplication::startUp(EditorRenderAPI::DX11);
    //EditorApplication::instance().runMainLoop();
    //EditorApplication::shutDown();
    
  }
  __except(g_crashHandler().reportCrash(GetExceptionInformation())) {
    PlatformUtility::terminate(true);
  }
  
  Time::shutDown();
  DynLibManager::shutDown();
  CrashHandler::shutDown();

  return 0;
}
