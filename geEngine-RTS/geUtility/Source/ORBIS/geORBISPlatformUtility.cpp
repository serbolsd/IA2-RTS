/*****************************************************************************/
/**
 * @file    geWin32PlatformUtility.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2016/03/11
 * @brief   Provides access to Windows specific utility functions
 *
 * Provides access to various Windows specific utility functions
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
#include "geColor.h"

#if GE_PLATFORM == GE_PLATFORM_PS4

namespace geEngineSDK {
  void
  PlatformUtility::terminate(bool force) {
    GE_UNREFERENCED_PARAMETER(force);
  }

  void
  PlatformUtility::copyToClipboard(const WString& string) {
    GE_UNREFERENCED_PARAMETER(string);
  }

  WString
  PlatformUtility::copyFromClipboard() {
    return StringUtil::WBLANK;
  }

  WString
  PlatformUtility::keyCodeToUnicode(uint32 keyCode) {
    GE_UNREFERENCED_PARAMETER(keyCode);
    return StringUtil::WBLANK;
  }

  bool
  PlatformUtility::getMACAddress(MACAddress& address) {
    GE_UNREFERENCED_PARAMETER(address);
    return false;
  }

  UUID
  PlatformUtility::generateUUID() {
    return UUID::EMPTY;
  }

  void
  PlatformUtility::open(const Path& path) {
    GE_UNREFERENCED_PARAMETER(path);
  }
}

#else
#   pragma error "Trying to compile ORBIS Code on Non-PS enviroment."
#endif // #if GE_PLATFORM == GE_PLATFORM_PS4
