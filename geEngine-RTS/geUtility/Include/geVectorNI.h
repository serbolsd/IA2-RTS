/*****************************************************************************/
/**
 * @file    geVectorNI.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2015/04/14
 * @brief   A N dimensional vector with integer coordinates.
 *
 * A N dimensional vector with integer coordinates.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/
#pragma once

#ifndef _INC_VECTORNI_H_
# define _INC_VECTORNI_H_
#endif

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
namespace geEngineSDK {
  /**
   * @brief A N dimensional vector with integer coordinates.
   */
  template<int N>
  class VectorNI
  {
   public:
    VectorNI() {
      memset(v, 0, sizeof(v));
    }

    VectorNI(int32 val[N]) {
      memcpy(v, val, sizeof(v));
    }

    int32
    operator[](SIZE_T i) const {
      GE_ASSERT(i < N);
      return v[i];
    }

    int32&
    operator[](SIZE_T i) {
      GE_ASSERT(i < N);
      return v[i];
    }

    VectorNI&
    operator=(const VectorNI& rhs) {
      memcpy(v, rhs.v, sizeof(v));
      return *this;
    }

    bool
    operator==(const Vector2I& rhs) const {
      for (SIZE_T i = 0; i < N; ++i) {
        if (v[i] != rhs[i]) {
          return false;
        }
      }
      return true;
    }

    bool
    operator!=(const Vector2I& rhs) const {
      return !operator==(*this, rhs);
    }

    int32 v[N];
  };

  using Vector3I = VectorNI<3>;
  using Vector4I = VectorNI<4>;
}
