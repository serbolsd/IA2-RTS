/*****************************************************************************/
/**
 * @file    geIReflectableRTTI.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/06/11
 * @brief   Reflectable RTTI Interface.
 *
 * Reflectable RTTI Interface.
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

namespace geEngineSDK {
  class GE_UTILITY_EXPORT IReflectableRTTI
    : public RTTIType<IReflectable, IReflectable, IReflectableRTTI>
  {
   public:
    const String&
    getRTTIName() override {
      static String name = "IReflectable";
      return name;
    }

    uint32
    getRTTIId() override {
      return TYPEID_UTILITY::kID_IReflectable;
    }

    SPtr<IReflectable>
    newRTTIObject() override {
      return nullptr;
    }
  };
}
