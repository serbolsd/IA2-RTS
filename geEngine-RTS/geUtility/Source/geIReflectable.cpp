/*****************************************************************************/
/**
 * @file    geIReflectable.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/11/03
 * @brief   Interface implemented by classes that provide RTTI.
 *
 * Provides an interface for accessing fields of a certain class.
 * Data can be easily accessed by getter and setter methods.
 *
 * Any class implementing this interface must implement the getRTTI() method,
 * as well as a static getRTTIStatic() method, returning the same value as
 * getRTTI(). Object returned by those methods is used for retrieving actual
 * RTTI data about the class.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "geIReflectable.h"
#include "geRTTIType.h"
#include "geException.h"
#include "geIReflectableRTTI.h"

namespace geEngineSDK {
  void
  IReflectable::_registerDerivedClass(RTTITypeBase* derivedClass) {
    if (_isTypeIdDuplicate(derivedClass->getRTTIId())) {
      GE_EXCEPT(InternalErrorException, "RTTI type \"" + derivedClass->getRTTIName() +
                "\" has a duplicate ID: " + toString(derivedClass->getRTTIId()));
    }

    getDerivedClasses().push_back(derivedClass);
  }

  SPtr<IReflectable>
  IReflectable::createInstanceFromTypeId(uint32 rttiTypeId) {
    RTTITypeBase* type = _getRTTIfromTypeId(rttiTypeId);

    SPtr<IReflectable> output;
    if (nullptr != type) {
      output = type->newRTTIObject();
      output->m_rttiData = nullptr;
    }

    return output;
  }

  RTTITypeBase*
  IReflectable::_getRTTIfromTypeId(uint32 rttiTypeId) {
    Stack<RTTITypeBase*> todo;

    for (const auto& item : getDerivedClasses()) {
      todo.push(item);
    }

    while (!todo.empty()) {
      RTTITypeBase* curType = todo.top();
      todo.pop();

      if (curType->getRTTIId() == rttiTypeId) {
        return curType;
      }

      for (const auto& item : curType->getDerivedClasses()) {
        todo.push(item);
      }
    }

    return nullptr;
  }

  bool
  IReflectable::_isTypeIdDuplicate(uint32 typeId) {
    if (TYPEID_UTILITY::kID_Abstract == typeId) {
      return false;
    }

    return IReflectable::_getRTTIfromTypeId(typeId) != nullptr;
  }

  bool
  IReflectable::isDerivedFrom(RTTITypeBase* base) {
    return getRTTI()->isDerivedFrom(base);
  }

  void
  IReflectable::_checkForCircularReferences() {
    Stack<RTTITypeBase*> todo;

    Vector<RTTITypeBase*> rootTypes = getDerivedClasses();
    for (auto& entry : rootTypes) {
      todo.push(entry);
    }

    while (!todo.empty()) {
      RTTITypeBase* myType = todo.top();
      todo.pop();

      uint32 myNumFields = myType->getNumFields();
      for (uint32 i = 0; i < myNumFields; ++i) {
        RTTIField* myField = myType->getField(i);

        if (!myField->isReflectablePtrType()) {
          continue;
        }

        RTTIReflectablePtrFieldBase*
        myReflectablePtrField = static_cast<RTTIReflectablePtrFieldBase*>(myField);

        RTTITypeBase* otherType = myReflectablePtrField->getType();
        uint32 otherNumFields = otherType->getNumFields();
        for (uint32 j = 0; j < otherNumFields; ++j) {
          RTTIField* otherField = otherType->getField(j);

          if (!otherField->isReflectablePtrType()) {
            continue;
          }

          RTTIReflectablePtrFieldBase*
          otherReflectablePtrField = static_cast<RTTIReflectablePtrFieldBase*>(otherField);

          if (myType->getRTTIId() == otherReflectablePtrField->getType()->getRTTIId() &&
              0 == (myReflectablePtrField->getFlags() & RTTI_FIELD_FLAG::kWeakRef) &&
              0 == (otherReflectablePtrField->getFlags() & RTTI_FIELD_FLAG::kWeakRef)) {
            GE_EXCEPT(InternalErrorException, "Found circular reference on RTTI type: " +
                      myType->getRTTIName() + " to type: " + otherType->getRTTIName() +
                      ". Either remove one of the references or mark it" +
                      " as a weak reference when defining the RTTI field.");
          }
        }
      }
    }
  }

  uint32
  IReflectable::getTypeId() const {
    return getRTTI()->getRTTIId();
  }

  const String&
  IReflectable::getTypeName() const {
    return getRTTI()->getRTTIName();
  }

  RTTITypeBase*
  IReflectable::getRTTIStatic() {
    return IReflectableRTTI::instance();
  }
}
