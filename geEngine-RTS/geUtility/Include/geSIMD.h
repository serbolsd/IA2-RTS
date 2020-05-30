/*****************************************************************************/
/**
 * @file    geSIMD.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2018/02/19
 * @brief   SIMD functions and objects declaration
 *
 * SIMD functions and objects declaration
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
#include "geVector4.h"
#include "geBox.h"
#include "geSphere.h"

#define SIMDPP_ARCH_X86_SSE4_1

#if GE_COMPILER == GE_COMPILER_MSVC
# pragma warning(disable: 4127)
# pragma warning(disable: 4244)
#endif

# include "Externals/simdpp/simd.h"

#if GE_COMPILER == GE_COMPILER_MSVC
# pragma warning(default: 4127)
# pragma warning(default: 4244)
#endif

namespace geEngineSDK {
  namespace simd {
    using namespace simdpp;

    /**
     * @brief Version of geEngineSDK::AABox suitable for SIMD use. Takes up a
     *        bit more memory than standard AABox and is always 16-byte aligned
     */
    struct AABox
    {
      AABox() {}

      /**
       * @brief Initializes bounds from an AABox.
       */
      AABox(const geEngineSDK::AABox& box) {
        Vector3 bC, bE;
        box.getCenterAndExtents(bC, bE);
        m_center = bC;
        m_extents = bE;
      }

      /**
       * @brief Initializes bounds from a Sphere.
       */
      AABox(const Sphere& sphere) {
        m_center = sphere.m_center;
        float radius = sphere.m_radius;
        m_extents = Vector4(radius, radius, radius, 0.0f);
      }

      /**
       * @brief Initializes bounds from a vector representing the center and
       *        equal extents in all directions.
       */
      AABox(const Vector3& center, float extent) {
        m_center = center;
        m_extents = Vector4(extent, extent, extent, 0.0f);
      }

      /**
       * @brief Returns true if the current bounds object intersects the
       *        provided object.
       */
      bool
      intersect(const AABox& other) const {
        auto myCenter = load<float32x4>(&m_center);
        auto otherCenter = load<float32x4>(&other.m_center);

        float32x4 diff = abs(sub(myCenter, otherCenter));

        auto myExtents = simd::load<float32x4>(&m_extents);
        auto otherExtents = simd::load<float32x4>(&other.m_extents);

        float32x4 extents = add(myExtents, otherExtents);

        return test_bits_any(bit_cast<uint32x4>(cmp_gt(diff, extents))) == false;
      }

      /**
       * @brief Center of the bounds, W component unused.
       */
      SIMDPP_ALIGN(16) Vector4 m_center;

      /**
       * @brief Extents (half-size) of the bounds, W component unused.
       */
      SIMDPP_ALIGN(16) Vector4 m_extents;
    };
  }
}
