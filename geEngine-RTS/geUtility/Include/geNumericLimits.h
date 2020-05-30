/*****************************************************************************/
/**
 * @file    geNumericLimits.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2018/05/20
 * @brief   Constants with the numeric limits for each numeric data type.
 *
 * Constants with the numeric limits for each numeric data type.
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
#include "gePlatformTypes.h"

#include <limits>

namespace geEngineSDK {
  using std::numeric_limits;

  struct GE_UTILITY_EXPORT NumLimit
  {
    static constexpr uint8  MIN_UINT8  = numeric_limits<uint8 >::min();
    static constexpr uint16 MIN_UINT16 = numeric_limits<uint16>::min();
    static constexpr uint32 MIN_UINT32 = numeric_limits<uint32>::min();

    static constexpr int8   MIN_INT8  = numeric_limits<int8  >::min();
    static constexpr int16  MIN_INT16 = numeric_limits<int16 >::min();
    static constexpr int32  MIN_INT32 = numeric_limits<int32 >::min();

    static constexpr uint8  MAX_UINT8 = numeric_limits<uint8 >::max();
    static constexpr uint16 MAX_UINT16 = numeric_limits<uint16>::max();
    static constexpr uint32 MAX_UINT32 = numeric_limits<uint32>::max();

    static constexpr int8   MAX_INT8 = numeric_limits<int8  >::max();
    static constexpr int16  MAX_INT16 = numeric_limits<int16 >::max();
    static constexpr int32  MAX_INT32 = numeric_limits<int32 >::max();

    static constexpr SIZE_T MIN_SIZET = numeric_limits<SIZE_T >::min();
    static constexpr SIZE_T MAX_SIZET = numeric_limits<SIZE_T >::max();

    static constexpr float  MIN_FLOAT = numeric_limits<float >::min();
    static constexpr float  MAX_FLOAT = numeric_limits<float >::max();

    static constexpr float  POS_INFINITY = numeric_limits<float>::infinity();
    static constexpr float  NEG_INFINITY = -numeric_limits<float>::infinity();

    static constexpr float FLOAT_EPSILON = numeric_limits<float >::epsilon();
    static constexpr double DOUBLE_EPSILON = numeric_limits<double>::epsilon();
  };
}
