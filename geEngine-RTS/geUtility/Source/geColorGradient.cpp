/*****************************************************************************/
/**
 * @file    geColorGradient.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2018/06/06
 * @brief   Class to manage a multi-keys color gradient.
 *
 * Class to manage a multi-keys color gradient.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "geColorGradient.h"
#include "geDebug.h"
#include "geBitwise.h"

namespace geEngineSDK {
  LinearColor
  ColorGradient::evaluate(float t) const {
    if (0 == m_numKeys) {
      return LinearColor::Transparent;
    }

    if (1 == m_numKeys) {
      return m_colors[0];
    }

    float time = t;
    time = Math::clamp(time,
                       m_times[0],
                       m_times[m_numKeys - 1]);

    //TODO: Add a version of evaluate that supports caching?
    for (uint32 i = 1; i < m_numKeys; ++i) {
      const float curKeyTime = m_times[i];
      if (time > curKeyTime) {
        continue;
      }

      const float prevKeyTime = m_times[i - 1];
      const float fracColor = Math::invLerp(time, prevKeyTime, curKeyTime);
      return Math::lerp(m_colors[i - 1], m_colors[i], fracColor);
    }

    return m_colors[m_numKeys - 1];
  }

  void
  ColorGradient::setKeys(const Vector<ColorGradientKey>& keys,
                         float duration) {
    if (keys.size() > MAX_KEYS) {
      LOGWRN("Number of keys in ColorGradient exceeds the supported number (" +
             toString(MAX_KEYS) + "). Keys will be ignored.");
    }

    m_numKeys = 0;
    for (auto& key : keys) {
      if (MAX_KEYS <= m_numKeys) {
        break;
      }

      m_colors[m_numKeys] = key.color;
      m_times[m_numKeys] = key.time / duration;
      ++m_numKeys;
    }

    m_duration = duration;
  }

  void
  ColorGradient::setConstant(const LinearColor& color) {
    m_colors[0] = color;
    m_times[0] = 0;
    m_numKeys = 1;
    m_duration = 0.0f;
  }
}
