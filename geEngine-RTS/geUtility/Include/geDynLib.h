/*****************************************************************************/
/**
 * @file    geDynLib.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2016/09/25
 * @brief   Class that holds data about a dynamic library.
 *
 * Class that holds data about a dynamic library.
 *
 * @bug	    No known bugs.
 */
/*****************************************************************************/
#pragma once

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "gePrerequisitesUtil.h"

#if GE_PLATFORM == GE_PLATFORM_WIN32
  struct HINSTANCE__;
  using hInstance = struct HINSTANCE__*;
#endif

#if GE_PLATFORM == GE_PLATFORM_WIN32
# define DYNLIB_HANDLE hInstance
# define DYNLIB_LOAD(a) LoadLibraryEx(a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
# define DYNLIB_GETSYM(a, b) GetProcAddress(a, b)
# define DYNLIB_UNLOAD(a) !FreeLibrary(a)

#elif GE_PLATFORM == GE_PLATFORM_LINUX || GE_PLATFORM == GE_PLATFORM_OSX
# define DYNLIB_HANDLE void*
# define DYNLIB_LOAD(a) dlopen(a, RTLD_LAZY | RTLD_GLOBAL)
# define DYNLIB_GETSYM(a, b) dlsym(a, b)
# define DYNLIB_UNLOAD(a) dlclose(a)

#elif GE_PLATFORM == GE_PLATFORM_PS4
# define DYNLIB_HANDLE SceKernelModule
# define DYNLIB_LOAD(a, b, c, d, e, f) sceKernelLoadStartModule(a, b, c, d, e, f)
# define DYNLIB_GETSYM(a, b, c) sceKernelDlsym(a, b, c)
# define DYNLIB_UNLOAD(a, b, c, d, e, f) sceKernelStopUnloadModule(a, b, c, d, e, f)

#endif

namespace geEngineSDK {
  /**
   * @brief Class that holds data about a dynamic library.
   */
  class GE_UTILITY_EXPORT DynLib final
  {
   public:
#if GE_PLATFORM == GE_PLATFORM_LINUX
    static  const char* EXTENSION = "so";
    static constexpr const char* PREFIX = "lib";
#elif GE_PLATFORM == GE_PLATFORM_OSX
    static constexpr const char* EXTENSION = "dylib";
    static constexpr const char* PREFIX = "lib";
#elif GE_PLATFORM == GE_PLATFORM_WIN32
    static constexpr const char* EXTENSION = "dll";
    static constexpr const char* PREFIX = nullptr;
#elif GE_PLATFORM == GE_PLATFORM_PS4
    static constexpr const char* EXTENSION = "prx";
    static constexpr const char* PREFIX = "a";
#else
#  error Unhandled platform
#endif

    /**
     * @brief Constructs the dynamic library object and loads the library with
     *        the specified name.
     */
    DynLib(String name);
    ~DynLib() = default;

    /**
     * @brief Loads the library. Does nothing if library is already loaded.
     */
    void
    load();

    /**
     * @brief Unloads the library. Does nothing if library is not loaded.
     */
    void
    unload();

    /**
     * @brief Get the name of the library.
     */
    const String&
    getName() const {
      return m_name;
    }

    /**
     * @brief Returns the address of the given symbol from the loaded library.
     * @param[in] strName The name of the symbol to search for.
     * @return  If the function succeeds, the returned value is a handle to the symbol.
     *          Otherwise null.
     */
    void*
    getSymbol(const String& strName) const;

   protected:
    friend class DynLibManager;

    /**
     * @brief Gets the last loading error.
     */
    static String
    dynlibError();

   protected:
    const String m_name;
    DYNLIB_HANDLE m_hInst;  //Handle to the loaded library.
  };
}
