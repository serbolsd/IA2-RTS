/*****************************************************************************/
/**
 * @file    geWin32FileSystem.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2016/09/19
 * @brief   Utility class for dealing with Win32 files.
 *
 * Utility class for dealing with Win32 files.
 *
 * @bug	    No known bugs.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "geFileSystem.h"
#include "geDataStream.h"

#if GE_PLATFORM == GE_PLATFORM_PS4

namespace geEngineSDK {
  SPtr<DataStream>
  FileSystem::openFile(const Path& fullPath, bool readOnly) {
    GE_UNREFERENCED_PARAMETER(fullPath);
    GE_UNREFERENCED_PARAMETER(readOnly);
    return nullptr;
  }

  SPtr<DataStream>
  FileSystem::createAndOpenFile(const Path& fullPath) {
    GE_UNREFERENCED_PARAMETER(fullPath);
    return nullptr;
  }

  uint64
  FileSystem::getFileSize(const Path& fullPath) {
    GE_UNREFERENCED_PARAMETER(fullPath);
    return 0;
  }
  
  bool
  FileSystem::exists(const Path& fullPath) {
    GE_UNREFERENCED_PARAMETER(fullPath);
    return false;
  }

  bool
  FileSystem::isFile(const Path& fullPath) {
    GE_UNREFERENCED_PARAMETER(fullPath);
    return false;
  }

  bool
  FileSystem::isDirectory(const Path& fullPath) {
    GE_UNREFERENCED_PARAMETER(fullPath);
    return false;
  }

  void
  FileSystem::createDir(const Path& fullPath) {
    GE_UNREFERENCED_PARAMETER(fullPath);
  }

  void
  FileSystem::getChildren(const Path& dirPath,
                          Vector<Path>& files,
                          Vector<Path>& directories) {
    GE_UNREFERENCED_PARAMETER(dirPath);
    GE_UNREFERENCED_PARAMETER(files);
    GE_UNREFERENCED_PARAMETER(directories);
  }

  bool
  FileSystem::iterate(const Path& dirPath,
                      const std::function<bool(const Path&)>& fileCallback,
                      const std::function<bool(const Path&)>& dirCallback,
                      bool recursive) {
    GE_UNREFERENCED_PARAMETER(dirPath);
    GE_UNREFERENCED_PARAMETER(fileCallback);
    GE_UNREFERENCED_PARAMETER(dirCallback);
    GE_UNREFERENCED_PARAMETER(recursive);
    return false;
  }

  std::time_t
  FileSystem::getLastModifiedTime(const Path& fullPath) {
    GE_UNREFERENCED_PARAMETER(fullPath);
    return std::time_t();
  }

  Path
  FileSystem::getWorkingDirectoryPath() {
    return Path();
  }

  Path
  FileSystem::getTempDirectoryPath() {
    return Path();
  }

  void
  FileSystem::copyFile(const Path& from, const Path& to) {
    GE_UNREFERENCED_PARAMETER(from);
    GE_UNREFERENCED_PARAMETER(to);
  }

  void
  FileSystem::removeFile(const Path& path) {
    GE_UNREFERENCED_PARAMETER(path);
  }

  void
  FileSystem::moveFile(const Path& oldPath, const Path& newPath) {
    GE_UNREFERENCED_PARAMETER(oldPath);
    GE_UNREFERENCED_PARAMETER(newPath);
  }
}

#else
#   pragma error "Trying to compile ORBIS Code on Non-PS enviroment."
#endif // #if GE_PLATFORM == GE_PLATFORM_PS4
