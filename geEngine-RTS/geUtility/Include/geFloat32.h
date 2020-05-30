/*****************************************************************************/
/**
 * @file    geFloat32.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2014/07/17
 * @brief   32 bits float with access to elements
 *
 * 32 bits float with access to individual elements
 * 23 bits Mantissa , 8 bits Exponent, 1 bit Sign
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
#include "gePlatformDefines.h"
#include "gePlatformTypes.h"

namespace geEngineSDK {
  /**
   * @brief 32 bit float components
   */
  class Float32
  {
   public:
    /**
     * @brief Constructor
     * @param InValue value of the float.
     * @return  What does this function returns.
     */
    explicit Float32(float InValue = 0.0f) : floatValue(InValue) {}

    union {
      struct
      {
#if GE_ENDIAN == GE_ENDIAN_LITTLE
        uint32 mantissa : 23;
        uint32 exponent : 8;
        uint32 sign : 1;
#else
        uint32 sign : 1;
        uint32 exponent : 8;
        uint32 mantissa : 23;
#endif
      } components;
      float	floatValue;
      uint32 integerValue;
    };
  };
}
