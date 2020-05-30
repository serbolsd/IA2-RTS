/*****************************************************************************/
/**
 * @file    geFloat10.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/06/03
 * @brief   Float 10 class according to OpenGL packed_float extension.
 *
 * Declaration of a class to use 10 bits floats with access to elements
 * represented by 5 mantissa bits and 5 exponent bits.
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
#include "geFloat32.h"

namespace geEngineSDK {
  /**
   * @brief Class to use 10 bits floats with elements access
   * @note 5 mantissa bits and 5 exponent bits.
   */
  class Float10
  {
   public:
    
    Float10() : encoded(0) {}
    Float10(const Float10& FP10Value) {
      encoded = FP10Value.encoded;
    }

    explicit Float10(float FP32Value) {
      set(FP32Value);
    }

    Float10&
    operator=(float FP32Value) {
      set(FP32Value);
      return *this;
    }

    Float10&
    operator=(const Float10& FP10Value) {
      encoded = FP10Value.encoded;
      return *this;
    }

    operator float() const {
      return getFloat();
    }

    void
    set(float FP32Value) {
      Float32 FP32(FP32Value);

      if (255 == FP32.components.exponent) {
        if (0 < FP32.components.mantissa) {
          encoded = 992 | ((
            (FP32.integerValue >> 18) |
            (FP32.integerValue >> 13) |
            (FP32.integerValue >> 3) |
            FP32.integerValue) & 31);
        }
        else if (FP32.components.sign) {  //Negative infinity clamped to 0
          encoded = 0;
        }
        else {  //Positive infinity
          encoded = 992;
        }
      }
      else if (FP32.components.sign) {  //Negative clamped to 0, no negatives allowed
        encoded = 0;
      }
      else if (0x477C0000 < FP32.integerValue) {  //Too large, clamp to max value
        encoded = 991;
      }
      else {
        uint32 val;
        if (0x38800000U > FP32.integerValue) {
          //Too small to be represented as a normalized float, convert to denormalized value
          uint32 shift = 113 - FP32.components.exponent;
          val = (0x800000U | FP32.components.mantissa) >> shift;
        }
        else {
          //Re bias exponent
          val = FP32.integerValue + 0xC8000000;
        }

        encoded = ((val + 0x1FFFFU + ((val >> 18) & 1)) >> 18) & 1023;
      }
    }

    /**
     * @brief Convert from Fp10 to Fp32.
     */
    float
    getFloat() const {
      Float32 Result;

      if (31 == components.exponent) { //INF or NAN
        Result.integerValue = 0x7f800000 | (components.mantissa << 17);
      }
      else {
        uint32 exponent;
        uint32 mantissa = components.mantissa;

        if (0 != components.exponent) {//The value is normalized
          exponent = components.exponent;
        }
        else if (mantissa != 0) { //The value is denormalized
          //Normalize the value in the resulting float
          exponent = 1;
          do {
            --exponent;
            mantissa <<= 1;
          } while (0 == (mantissa & 0x20));

          mantissa &= 0x1F;
        }
        else { //The value is zero
          exponent = static_cast<uint32>(-112);
        }

        Result.integerValue = ((exponent + 112) << 23) | (mantissa << 18);
      }

      return Result.floatValue;
    }

    union {
      uint32 encoded;
      struct
      {
#if GE_ENDIAN == GE_ENDIAN_LITTLE
        uint32 mantissa : 5;
        uint32 exponent : 5;
#else
        uint32 exponent : 5;
        uint32 mantissa : 5;
#endif
      } components;
    };
  };
}
