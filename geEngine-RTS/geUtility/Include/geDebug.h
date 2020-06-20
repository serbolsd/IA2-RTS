/*****************************************************************************/
/**
 * @file    geDebug.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2016/09/19
 * @brief   Utility class providing various debug functionality.
 *
 * Utility class providing various debug functionality.
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
#include "geLog.h"

namespace geEngineSDK {
  class Log;

  /**
   * @brief	Available types of channels that debug messages can be logged to.
   */
  namespace DEBUG_CHANNEL {
    enum E {
      kDebug,
      kWarning,
      kError,
      kCompilerWarning,
      kCompilerError
    };
  }

  /**
   * @brief Utility class providing various debug functionality.
   * @note  Thread safe.
   */
  class GE_UTILITY_EXPORT Debug
  {
   public:
     Debug() = default;

    /**
     * @brief Adds a log entry in the "Debug" channel.
     */
    void
    logDebug(const String& msg);

    /**
     * @brief Adds a log entry in the "Warning" channel.
     */
    void
    logWarning(const String& msg);

    /**
     * @brief Adds a log entry in the "Error" channel.
     */
    void
    logError(const String& msg);

    /**
     * @brief Adds a log entry in the specified channel.
              You may specify custom channels as needed.
     */
    void
    logMessage(const String& msg, uint32 channel);

    /**
     * @brief Retrieves the Log used by the Debug instance.
     */
    Log&
    getLog() {
      return m_log;
    }

    /**
     * @brief Converts raw pixels into a BMP image. See BitmapWriter for more information.
     */
    void
    writeAsBMP(uint8* rawPixels,
               uint32 bytesPerPixel,
               uint32 width,
               uint32 height,
               const Path& filePath,
               bool overwrite = true) const;

    /**
     * @brief Saves a log about the current state of the application to the
     *        specified location.
     * @param path  Absolute path to the log filename.
     */
    void
    saveLog(const Path& path) const;

    /**
     * @brief Triggered when a new entry in the log is added.
     * @note  Sim thread only.
     */
    Event<void(const LogEntry&)> onLogEntryAdded;

    /**
     * @brief Triggered whenever one or multiple log entries were added or removed.
     *        Triggers only once per frame.
     * @note  Sim thread only.
     */
    Event<void()> onLogModified;

    /**
     * @brief Triggers callbacks that notify external code that a log entry was added.
     * @note  Internal method. Sim thread only.
     */
    void
    _triggerCallbacks();

   private:
    uint64 m_logHash = 0;
    Log m_log;
  };

  /**
   * @brief A simpler way of accessing the Debug module.
   */
  GE_UTILITY_EXPORT Debug&
  g_Debug();

  /**
   * @brief Shortcut for logging a message in the debug channel.
   */
#define LOGDBG(x) geEngineSDK::g_Debug().logDebug((x) +                       \
                                                  String("\n\t\t in ") +      \
                                                  __PRETTY_FUNCTION__ +       \
                                                  " [" + __FILE__ + ":" +     \
                                                  toString(__LINE__) +\
                                                  "]\n");

  /**
   * @brief Shortcut for logging a message in the warning channel.
   */
#define LOGWRN(x) geEngineSDK::g_Debug().logWarning((x) +                     \
                                                  String("\n\t\t in ") +      \
                                                  __PRETTY_FUNCTION__ +       \
                                                  " [" + __FILE__ + ":" +     \
                                                  toString(__LINE__) +\
                                                  "]\n");

  /**
   * @brief Shortcut for logging a message in the error channel.
   */
#define LOGERR(x) geEngineSDK::g_Debug().logError((x) +                       \
                                                  String("\n\t\t in ") +      \
                                                  __PRETTY_FUNCTION__ +       \
                                                  " [" + __FILE__ + ":" +     \
                                                  toString(__LINE__) +\
                                                  "]\n");

  /**
   * @brief Shortcut for logging a verbose message in the debug channel.
   *        Verbose messages can be ignored unlike other log messages.
   */
#define LOGDBG_VERBOSE(x) ((void)0)

  /**
   * @brief Shortcut for logging a verbose message in the warning channel.
   *        Verbose messages can be ignored unlike other log messages.
   */
#define LOGWRN_VERBOSE(x) ((void)0)
}
