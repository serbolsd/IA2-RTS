/*****************************************************************************/
/**
 * @file    geBox.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2015/04/19
 * @brief   Implements an axis-aligned box.
 *
 * Boxes describe an axis-aligned extent in three dimensions. They are used for
 * many different things in the Engine and in games, such as bounding volumes,
 * collision detection and visibility calculation.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "gePrerequisitesUtil.h"
#include "geBox.h"
#include "geVector4.h"
#include "geMatrix4.h"

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
  using namespace simdpp;

  AABox::AABox(const Vector3* Points, SIZE_T Count)
    : m_min(0, 0, 0),
      m_max(0, 0, 0),
      m_isValid(0) {
    for (SIZE_T i = 0; i < Count; ++i) {
      *this += Points[i];
    }
  }

  AABox::AABox(const Vector<Vector3>& Points)
    : m_min(0, 0, 0),
      m_max(0, 0, 0),
      m_isValid(0) {
    for (auto& Point : Points) {
      *this += Point;
    }
  }

  AABox
  AABox::transformBy(const Matrix4& M) const {
    //If we are not valid, return another invalid box.
    if (!m_isValid) {
      return AABox(FORCE_INIT::kForceInit);
    }

    AABox NewBox;

    const Vector4 VecMin(m_min.x, m_min.y, m_min.z, 0.0f);
    const Vector4 VecMax(m_max.x, m_max.y, m_max.z, 0.0f);

    auto myExtents = load<float32x4>(&VecMin);

    const Vector4 m0(M.m[0][0], M.m[0][1], M.m[0][2], M.m[0][3]);
    const Vector4 m1(M.m[1][0], M.m[1][1], M.m[1][2], M.m[1][3]);
    const Vector4 m2(M.m[2][0], M.m[2][1], M.m[2][2], M.m[2][3]);
    const Vector4 m3(M.m[3][0], M.m[3][1], M.m[3][2], M.m[3][3]);

    const Vector4 Half(0.5f, 0.5f, 0.5f, 0.0f);
    const Vector4 Origin = (VecMax + VecMin) * Half;
    const Vector4 Extent = (VecMax - VecMin) * Half;

    Vector4 NewOrigin = Vector4(Origin.x, Origin.x, Origin.x, Origin.x) * m0;
    NewOrigin = (Vector4(Origin.y, Origin.y, Origin.y, Origin.y) * m1) + NewOrigin;
    NewOrigin = (Vector4(Origin.z, Origin.z, Origin.z, Origin.z) * m2) + NewOrigin;
    NewOrigin += m3;

    Vector4 NewExtent = (Vector4(Extent.x, Extent.x, Extent.x, Extent.x) * m0).vectorAbs();
    NewExtent = (Vector4(Extent.y, Extent.y, Extent.y, Extent.y) * m1).vectorAbs() + NewExtent;
    NewExtent = (Vector4(Extent.z, Extent.z, Extent.z, Extent.z) * m2).vectorAbs() + NewExtent;

    const Vector4 NewVecMin = NewOrigin - NewExtent;
    const Vector4 NewVecMax = NewOrigin + NewExtent;

    NewBox.m_min = NewVecMin;
    NewBox.m_max = NewVecMax;
    NewBox.m_isValid = 1;

    return NewBox;
  }

  AABox
  AABox::transformBy(const Transform& M) const {
    return transformBy(M.toMatrixWithScale());
  }

  AABox
  AABox::inverseTransformBy(const Transform& M) const {
    Vector3 Vertices[8] =
    {
      Vector3(m_min),
      Vector3(m_min.x, m_min.y, m_max.z),
      Vector3(m_min.x, m_max.y, m_min.z),
      Vector3(m_max.x, m_min.y, m_min.z),
      Vector3(m_max.x, m_max.y, m_min.z),
      Vector3(m_max.x, m_min.y, m_max.z),
      Vector3(m_min.x, m_max.y, m_max.z),
      Vector3(m_max)
    };

    AABox NewBox(FORCE_INIT::kForceInit);

    for (auto& Vertice : Vertices) {
      Vector4 ProjectedVertex = M.inverseTransformPosition(Vertice);
      NewBox += ProjectedVertex;
    }

    return NewBox;
  }

  AABox
  AABox::transformProjectBy(const Matrix4& ProjM) const {
    Vector3 Vertices[8] =
    {
      Vector3(m_min),
      Vector3(m_min.x, m_min.y, m_max.z),
      Vector3(m_min.x, m_max.y, m_min.z),
      Vector3(m_max.x, m_min.y, m_min.z),
      Vector3(m_max.x, m_max.y, m_min.z),
      Vector3(m_max.x, m_min.y, m_max.z),
      Vector3(m_min.x, m_max.y, m_max.z),
      Vector3(m_max)
    };

    AABox NewBox(FORCE_INIT::kForceInit);

    for (auto& Vertice : Vertices) {
      Vector4 ProjectedVertex = ProjM.transformPosition(Vertice);
      NewBox += (static_cast<Vector3>(ProjectedVertex)) / ProjectedVertex.w;
    }

    return NewBox;
  }

  AABox
  AABox::overlap(const AABox& Other) const {
    if (intersect(Other) == false) {
      static AABox EmptyBox(FORCE_INIT::kForceInit);
      return EmptyBox;
    }

    //Otherwise they overlap, so find overlapping box
    Vector3 MinVector, MaxVector;

    MinVector.x = Math::max(m_min.x, Other.m_min.x);
    MaxVector.x = Math::min(m_max.x, Other.m_max.x);

    MinVector.y = Math::max(m_min.y, Other.m_min.y);
    MaxVector.y = Math::min(m_max.y, Other.m_max.y);

    MinVector.z = Math::max(m_min.z, Other.m_min.z);
    MaxVector.z = Math::min(m_max.z, Other.m_max.z);

    return AABox(MinVector, MaxVector);
  }
}
