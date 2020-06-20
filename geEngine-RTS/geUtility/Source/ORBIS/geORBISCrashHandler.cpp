/*****************************************************************************/
/**
 * @file    geWin32CrashHandler.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2016/03/10
 * @brief   Saves crash data and notifies when a crash occurs
 *
 * Saves crash data and notifies the user when a crash occurs
 *
 * @bug	    No known bugs.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "gePrerequisitesUtil.h"

#if GE_PLATFORM == GE_PLATFORM_PS4

namespace geEngineSDK {
  //static const char* s_MiniDumpName = "MiniDump.dmp";

  CrashHandler::CrashHandler() {}

  CrashHandler::~CrashHandler() {}

  void
  CrashHandler::reportCrash(const String& type,
                            const String& strDescription,
                            const String& strFunction,
                            const String& strFile,
                            uint32 nLine) const {
    logErrorAndStackTrace(type, strDescription, strFunction, strFile, nLine);
    saveCrashLog();
  }

  String
  CrashHandler::getCrashTimestamp() {
    return StringUtil::BLANK;
  }

  String
  CrashHandler::getStackTrace() {
    return StringUtil::BLANK;
  }
}

#else
#   pragma error "Trying to compile ORBIS Code on Non-PS enviroment."
#endif // #if GE_PLATFORM == GE_PLATFORM_PS4
