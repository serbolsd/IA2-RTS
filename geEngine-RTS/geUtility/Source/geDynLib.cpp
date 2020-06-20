/*****************************************************************************/
/**
 * @file    geDynLib.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2016/09/25
 * @brief   Class that holds data about a dynamic library.
 *
 * Class that holds data about a dynamic library.
 *
 * @bug	    No known bugs.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "geDynLib.h"
#include "geException.h"

#if GE_PLATFORM == GE_PLATFORM_WIN32
#	include <Win32/geMinWindows.h>
#endif

#if GE_PLATFORM == GE_PLATFORM_OSX
# include <dlfcn.h>
#endif

namespace geEngineSDK {
  DynLib::DynLib(String name)
    : m_name(std::move(name)),
#if GE_PLATFORM == GE_PLATFORM_PS4
    m_hInst(0)
#else
    m_hInst(nullptr)
#endif
  {
    load();
  }

  void
  DynLib::load() {
    if (m_hInst) {
      return;
    }

#if GE_PLATFORM == GE_PLATFORM_PS4
    int startResult = 0;
    m_hInst = static_cast<DYNLIB_HANDLE>(DYNLIB_LOAD( m_name.c_str(),
                                                      0,
                                                      NULL,
                                                      0,
                                                      NULL,
                                                      &startResult));

    if (m_hInst < 0) {
      GE_EXCEPT(InternalErrorException,
                "Could not load dynamic library " +
                m_name +
                ".  System Error: " +
                dynlibError());
  }
#else
    m_hInst = static_cast<DYNLIB_HANDLE>(DYNLIB_LOAD(m_name.c_str()));

    if (!m_hInst) {
      GE_EXCEPT(InternalErrorException,
                "Could not load dynamic library " +
                m_name +
                ".  System Error: " +
                dynlibError());
    }
#endif

    
  }

  void
  DynLib::unload() {
    if (!m_hInst) {
      return;
    }

#if GE_PLATFORM == GE_PLATFORM_PS4
    if (DYNLIB_UNLOAD(m_hInst, 0, NULL, 0, NULL, NULL) != SCE_OK) {
      GE_EXCEPT(InternalErrorException,
                "Could not unload dynamic library " +
                m_name +
                ".  System Error: " +
                dynlibError());
    }
#else
    if (DYNLIB_UNLOAD(m_hInst)) {
      GE_EXCEPT(InternalErrorException,
                "Could not unload dynamic library " +
                m_name +
                ".  System Error: " +
                dynlibError());
    }
#endif
  }

  void*
  DynLib::getSymbol(const String& strName) const {
    if (!m_hInst) {
      return nullptr;
    }
#if GE_PLATFORM == GE_PLATFORM_PS4
    void* pAddressPtr = nullptr;
    DYNLIB_GETSYM(m_hInst, strName.c_str(), &pAddressPtr);
    return pAddressPtr;
#else
    return static_cast<void*>(DYNLIB_GETSYM(m_hInst, strName.c_str()));
#endif
  }

  String
  DynLib::dynlibError() {
#if GE_PLATFORM == GE_PLATFORM_WIN32
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM | 
                  FORMAT_MESSAGE_IGNORE_INSERTS, 
                  nullptr, 
                  GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  reinterpret_cast<LPTSTR>(&lpMsgBuf),
                  0,
                  nullptr);

    String ret = static_cast<char*>(lpMsgBuf);
    LocalFree(lpMsgBuf);  //Free the buffer.
    return ret;
#elif GE_PLATFORM == GE_PLATFORM_LINUX || GE_PLATFORM == GE_PLATFORM_OSX
    return String(dlerror());
#else
    return String("");
#endif
  }
}
