/*****************************************************************************/
/**
 * @file    geTransformRTTI.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2018/04/28
 * @brief   RTTI Objects for geTransform.
 *
 * RTTI Objects for geTransform.
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
#include "geRTTIType.h"
#include "geTransform.h"

namespace geEngineSDK {
  class GE_UTILITY_EXPORT TransformRTTI
    : public RTTIType<Transform, IReflectable, TransformRTTI>
  {
   private:
    GE_BEGIN_RTTI_MEMBERS
      GE_RTTI_MEMBER_PLAIN(m_rotation, 0)
      GE_RTTI_MEMBER_PLAIN(m_translation, 1)
      GE_RTTI_MEMBER_PLAIN(m_scale3D, 2)
    GE_END_RTTI_MEMBERS

   public:
    const String&
    getRTTIName() override {
      static String name = "Transform";
      return name;
    }

    uint32
    getRTTIId() override {
      return TYPEID_UTILITY::kID_Transform;
    }

    SPtr<IReflectable>
    newRTTIObject() override {
      return ge_shared_ptr_new<Transform>();
    }
  };
}
