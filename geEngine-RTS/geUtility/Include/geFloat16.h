/*****************************************************************************/
/**
 * @file    geFloat16.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2014/07/17
 * @brief   IEEE float 16 class
 *
 * Declaration of a class to use 16 bits floats with access to elements
 * represented by 10 mantissa bits, 5 exponent bits and 1 sign bit.
 *
 * E=0, M=0     == 0.0
 * E=0, M!=0    == Denormalized value (M/2^10)*2^-14
 * 0<E<31, M=any== (1 + M / 2^10) * 2^(E-15)
 * E=32, M=0    == Infinite
 * E=31, M!=0   == NaN
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
#include "geMath.h"
#include "geFloat32.h"

namespace geEngineSDK {
  /**
   * @brief Class to use 16 bits floats with elements access
   * @note 10 mantissa bits, 5 exponent bits and 1 sign bit.
   */
  class Float16
  {
   public:
    
    Float16() : encoded(0) {}
    Float16(const Float16& FP16Value) {
      encoded = FP16Value.encoded;
    }

    explicit Float16(float FP32Value) {
      set(FP32Value);
    }

    Float16&
    operator=(float FP32Value) {
      set(FP32Value);
      return *this;
    }

    Float16&
    operator=(const Float16& FP16Value) {
      encoded = FP16Value.encoded;
      return *this;
    }

    operator float() const {
      return getFloat();
    }

    void
    set(float FP32Value) {
      Float32 FP32(FP32Value);
      components.sign = FP32.components.sign;	//Copy the sign bit

      //Check if this is a zero, de-normalized or too small value
      //Exponent Too small? (0+127-15)
      if (112 >= FP32.components.exponent) {
        //Set it to 0
        components.exponent = 0;
        components.mantissa = 0;
      }
      else if (143 <= FP32.components.exponent)	{ //Exponent Too big? (31+127-15)
        //Set it to 65504.0 (max value)
        components.exponent = 30;
        components.mantissa = 1023;
      }
      else {
        //Handle as normal number
        components.exponent = int32(FP32.components.exponent) - 127 + 15;
        components.mantissa = uint16(FP32.components.mantissa >> 13);
      }
    }

    /**
     * @brief Convert from Fp32 to Fp16 without doing any checks if the Fp32
     *        exponent is too large or too small. This is a faster alternative
     *        to set() when you know the values within the single precision
     *        float don't need the checks.
     * @param FP32Value Single precision float to be set as half precision.
     * @note  Make absolutely sure that you never pass in a single precision
     *        floating point value that may actually need the checks.
     *        If you are not 100% sure of that just use Set().
     */
    void
    setWithoutBoundsChecks(const float FP32Value) {
      const Float32 FP32(FP32Value);
      components.sign = FP32.components.sign;
      components.exponent = int32(FP32.components.exponent) - 127 + 15;
      components.mantissa = uint16(FP32.components.mantissa >> 13);
    }

    /**
     * @brief Convert from Fp16 to Fp32.
     */
    float
    getFloat() const {
      Float32	Result;

      Result.components.sign = components.sign;
      if (0 == components.exponent) {
        uint32 Mantissa = components.mantissa;
        if (0 == Mantissa) { //Zero
          Result.components.exponent = 0;
          Result.components.mantissa = 0;
        }
        else { //Denormal.
          uint32 MantissaShift = 10 - Math::trunc(Math::log2(static_cast<float>(Mantissa)));
          Result.components.exponent = 127 - (15 - 1) - MantissaShift;
          Result.components.mantissa = Mantissa << (MantissaShift + 23 - 10);
        }
      }
      else if (components.exponent == 31)	{// 2^5 - 1
        //Infinity or NaN. Set to 65504.0
        Result.components.exponent = 142;
        Result.components.mantissa = 8380416;
      }
      else { //Normal number.
        // Stored exponents are biased by half their range.
        Result.components.exponent = int32(components.exponent) - 15 + 127;
        Result.components.mantissa = uint32(components.mantissa) << 13;
      }

      return Result.floatValue;
    }

    union {
      struct
      {
#if GE_ENDIAN == GE_ENDIAN_LITTLE
        uint16 mantissa : 10;
        uint16 exponent : 5;
        uint16 sign : 1;
#else
        uint16 sign : 1;
        uint16 exponent : 5;
        uint16 mantissa : 10;
#endif
      } components;
      uint16 encoded;
    };
  };
}
