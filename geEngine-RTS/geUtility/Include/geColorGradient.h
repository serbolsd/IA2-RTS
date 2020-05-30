/*****************************************************************************/
/**
 * @file    geColorGradient.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2018/06/06
 * @brief   Class to manage a multi-keys color gradient.
 *
 * Class to manage a multi-keys color gradient.
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
#include "geColor.h"

namespace geEngineSDK {
  struct ColorGradientKey
  {
    LinearColor color;
    float time;
  };

  class GE_UTILITY_EXPORT ColorGradient
  {
    constexpr static uint32 MAX_KEYS = 8;
   public:
    LinearColor
    evaluate(float t) const;

    //TODO - Must be sorted, time must be normalized
    void
    setKeys(const Vector<ColorGradientKey>& keys, float duration = 1.0f);

    void
    setConstant(const LinearColor& color);

   private:
    LinearColor m_colors[MAX_KEYS];
    float m_times[MAX_KEYS];
    uint32 m_numKeys = 0;
    float m_duration = 0.0f;
  };
}
