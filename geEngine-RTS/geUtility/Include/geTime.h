/*****************************************************************************/
/**
 * @file    geTime.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2015/02/18
 * @brief   Manages all time related functionality.
 *
 * Manages all time related functionality.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/
#pragma once

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "gePrerequisitesUtil.h"
#include "geModule.h"

namespace geEngineSDK {
  using std::atomic;

  /**
   * @brief Manages all time related functionality.
   * @note  Sim thread only unless where specified otherwise.
   */
  class GE_UTILITY_EXPORT Time : public Module<Time>
  {
   public:
    Time();
    ~Time();

    /**
     * @brief Gets the time elapsed since application start. Only gets updated once per frame.
     * @return  The time since application start, in seconds.
     */
    float
    getTime() const {
      return m_timeSinceStart;
    }

    /**
     * @copydoc Time::getTime()
     */
    uint64 getTimeMs() const {
      return m_timeSinceStartMs;
    }

    /**
     * @brief Gets the time since last frame was executed. Only gets updated once per frame.
     * @return  Time since last frame was executed, in seconds.
     */
    float
    getFrameDelta() const {
      return m_frameDelta;
    }

    /**
     * @brief Returns the sequential index of the current frame. First frame is 0.
     * @return  The current frame.
     * @note  Thread safe, but only counts sim thread frames.
     */
    uint64
    getFrameIdx() const {
      return m_currentFrame.load();
    }

    /**
     * @brief Returns the precise time since application start, in microseconds.
     *        Unlike other time methods this is not only updated every frame,
     *        but will return exact time at the moment it is called.
     * @return  Time in microseconds.
     * @note  You will generally only want to use this for performance measurements
     *        and similar. Use non-precise methods in majority of code as it is
     *        useful to keep the time value equal in all methods during a single frame.
     */
    uint64
    getTimePrecise() const;

    /**
     * @brief Gets the time at which the application was started, counting from system start.
     * @return  The time since system to application start, in milliseconds.
     */
    uint64
    getStartTimeMs() const {
      return m_appStartTime;
    }

    /**
     * @brief Called every frame. Should only be called by Application.
     */
    void
    _update();

    /**
     * @brief Multiply with time in microseconds to get a time in seconds.
     */
    static const double MICROSEC_TO_SEC;
   private:
    float m_frameDelta = 0.0f;      /**< Frame delta in seconds */
    float m_timeSinceStart = 0.0f;  /**< Time since start in seconds */
    uint64 m_timeSinceStartMs = 0u;
    uint64 m_appStartTime = 0u;     /**< Time the application started, in microseconds */
    uint64 m_lastFrameTime = 0u;    /**< Time since last runOneFrame call, In microseconds */
    atomic<uint32> m_currentFrame{0UL};
    Timer* m_timer;
  };

  /**
   * @brief Easier way to access the Time module.
   */
  GE_UTILITY_EXPORT Time& g_time();
}
