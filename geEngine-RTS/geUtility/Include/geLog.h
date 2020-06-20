/*****************************************************************************/
/**
 * @file    geLog.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2016/09/19
 * @brief   Used for logging messages
 *
 * Can categorize messages according to channels, save the log to a
 * file and send out callbacks when a new message is added
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

namespace geEngineSDK {
  /**
   * @brief A single log entry, containing a message and a channel the message
   *        was recorded on.
   */
  class GE_UTILITY_EXPORT LogEntry
  {
   public:
    LogEntry() = default;
    LogEntry(String msg, uint32 channel)
      : m_msg(std::move(msg)),
        m_channel(channel)
    {}

    /**
     * @brief Channel the message was recorded on.
     */
    uint32
    getLogChannel() const {
      return m_channel;
    }

    /**
     * @brief Text of the message.
     */
    const String&
    getLogMessage() const {
      return m_msg;
    }

   private:
    String m_msg;
    uint32 m_channel;
  };

  /**
   * @brief Used for logging messages. Can categorize messages according to
   *        channels, save the log to a file and send out callbacks when a new
   *        message is added.
   * @note  Thread safe.
   */
  class GE_UTILITY_EXPORT Log
  {
   public:
    Log() = default;
    ~Log();

    /**
     * @brief Logs a new message.
     * @param[in] message	The message describing the log entry.
     * @param[in] channel Channel in which to store the log entry.
     */
    void
    logMsg(const String& message, uint32 channel);

    /**
     * @brief Removes all log entries.
     */
    void
    clear();

    /**
     * @brief Removes all log entries in a specific channel.
     */
    void
    clear(uint32 channel);

    /**
     * @brief Returns all existing log entries.
     */
    Vector<LogEntry>
    getEntries() const;

    /**
     * @brief Returns the latest unread entry from the log queue, and removes
     *        the entry from the unread entries list.
     * @param[out] entry Entry that was retrieved, or undefined if no entries exist.
     * @return true if an unread entry was retrieved, false otherwise.
     */
    bool
    getUnreadEntry(LogEntry& entry);

    /**
     * @brief Returns the last available log entry.
     * @param[out] entry Entry that was retrieved, or undefined if no entries exist.
     * @return true if an entry was retrieved, false otherwise.
     */
    bool
    getLastEntry(LogEntry& entry);

    /**
     * @brief Returns a hash value that is modified whenever entries in the log
     *        change. This can be used for checking for changes by external systems.
     */
    uint64
    getHash() const {
      return m_hash;
    }

   private:
    friend class Debug;

    /**
     * @brief Returns all log entries, including those marked as unread.
     */
    Vector<LogEntry>
    getAllEntries() const;

    Vector<LogEntry> m_entries;
    Queue<LogEntry> m_unreadEntries;
    uint64 m_hash = 0;
    mutable RecursiveMutex m_mutex;
  };
}
