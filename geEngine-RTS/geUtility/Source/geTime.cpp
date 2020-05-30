/*****************************************************************************/
/**
 * @file    geTime.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2015/02/18
 * @brief   Manages all time related functionality.
 *
 * Manages all time related functionality.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "geTime.h"
#include "geTimer.h"

namespace geEngineSDK {
  using std::memory_order_relaxed;

  const double Time::MICROSEC_TO_SEC = 1.0 / 1000000.0;

  Time::Time() {
    m_timer = ge_new<Timer>();
    m_appStartTime = m_timer->getStartMs();
    m_lastFrameTime = m_timer->getMicroseconds();
  }

  Time::~Time() {
    ge_delete(m_timer);
  }

  void
  Time::_update() {
    uint64 currentFrameTime = m_timer->getMicroseconds();
    m_frameDelta = static_cast<float>((currentFrameTime - m_lastFrameTime) * MICROSEC_TO_SEC);
    m_timeSinceStartMs = static_cast<uint64>(currentFrameTime / 1000);
    m_timeSinceStart = m_timeSinceStartMs / 1000.0f;
    m_lastFrameTime = currentFrameTime;
    m_currentFrame.fetch_add(1, memory_order_relaxed);
  }

  uint64
  Time::getTimePrecise() const {
    return m_timer->getMicroseconds();
  }

  Time&
  g_time() {
    return Time::instance();
  }
}
