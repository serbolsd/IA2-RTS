/*****************************************************************************/
/**
 * @file    geFileSystem.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2016/09/19
 * @brief   Utility class for dealing with files.
 *
 * Shared functions among different operating systems
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
#include "geDebug.h"

namespace geEngineSDK {
  using std::tuple;
  using std::make_tuple;
  using std::get;

  void
  FileSystem::copy(const Path& oldPath, const Path& newPath, bool overwriteExisting) {
    Stack<tuple<Path, Path>> todo;
    todo.push(make_tuple(oldPath, newPath));

    while (!todo.empty()) {
      auto current = todo.top();
      todo.pop();

      Path sourcePath = get<0>(current);
      if (!FileSystem::exists(sourcePath)) {
        continue;
      }

      bool srcIsFile = FileSystem::isFile(sourcePath);
      Path destinationPath = get<1>(current);
      bool destExists = FileSystem::exists(destinationPath);

      if (destExists) {
        if (FileSystem::isFile(destinationPath)) {
          if (overwriteExisting) {
            FileSystem::remove(destinationPath);
          }
          else {
            LOGWRN("Copy operation failed because another file already exists" \
                   "at the new path: \"" + destinationPath.toString() + "\"");
            return;
          }
        }
      }

      if (srcIsFile) {
        FileSystem::copyFile(sourcePath, destinationPath);
      }
      else {
        if (!destExists) {
          FileSystem::createDir(destinationPath);
        }

        Vector<Path> files;
        Vector<Path> directories;
        getChildren(destinationPath, files, directories);

        for (auto& file : files) {
          Path fileDestPath = destinationPath;
          fileDestPath.append(file.getTail());
          todo.push(make_tuple(file, fileDestPath));
        }

        for (auto& dir : directories) {
          Path dirDestPath = destinationPath;
          dirDestPath.append(dir.getTail());
          todo.push(make_tuple(dir, dirDestPath));
        }
      }
    }
  }

  void
  FileSystem::remove(const Path& fullPath, bool recursively) {
    if (!FileSystem::exists(fullPath)) {
      return;
    }

    if (recursively) {
      Vector<Path> files;
      Vector<Path> directories;

      getChildren(fullPath, files, directories);

      for (auto& file : files) {
        remove(file, false);
      }

      for (auto& dir : directories) {
        remove(dir, true);
      }
    }

    FileSystem::removeFile(fullPath);
  }

  void
  FileSystem::move(const Path& oldPath, const Path& newPath, bool overwriteExisting) {
    if (FileSystem::exists(newPath)) {
      if (overwriteExisting) {
        FileSystem::remove(newPath);
      }
      else {
        LOGWRN("Move operation failed because another file already exists at" \
               "the new path: \"" + newPath.toString() + "\"");
        return;
      }
    }

    FileSystem::moveFile(oldPath, newPath);
  }

  Mutex FileScheduler::m_mutex;
}
