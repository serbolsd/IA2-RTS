/*****************************************************************************/
/**
 * @file    geRTTIReflectableField.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/11/03
 * @brief   Base class containing functionality for a reflectable class field.
 *
 * Reflectable fields are fields containing complex types deriving from
 * IReflectable. They are serialized recursively and you may add/remove fields
 * from them without breaking the serialized data.
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
#include "geRTTIField.h"
#include "geIReflectable.h"

namespace geEngineSDK {
  using std::function;

  struct RTTIReflectableFieldBase : public RTTIField
  {
    /**
     * @brief Retrieves the IReflectable value from the provided instance.
     * @note  Field type must not be an array.
     */
    virtual IReflectable&
    getValue(void* object) = 0;

    /**
     * @brief Retrieves the IReflectable value from an array on the provided
     *        instance and index.
     * @note  Field type must be an array.
     */
    virtual IReflectable&
    getArrayValue(void* object, uint32 index) = 0;

    /**
     * @brief Sets the IReflectable value in the provided instance.
     * @note  Field type must not be an array.
     */
    virtual void
    setValue(void* object, IReflectable& value) = 0;

    /**
     * @brief Sets the IReflectable value in an array on the provided instance
     *        and index.
     * @note  Field type must be an array.
     */
    virtual void
    setArrayValue(void* object, uint32 index, IReflectable& value) = 0;

    /**
     * @brief Creates a new object of the field type.
     */
    virtual SPtr<IReflectable>
    newObject() = 0;

    /**
     * @copydoc RTTIField::hasDynamicSize
     */
    bool hasDynamicSize() override {
      return true;
    }

    /**
     * @brief Retrieves the RTTI object for the type the field contains.
     */
    virtual RTTITypeBase*
    getType() = 0;
  };

  /**
   * @brief Reflectable field containing a specific type with RTTI implemented.
   */
  template <class DataType, class ObjectType>
  struct RTTIReflectableField : public RTTIReflectableFieldBase
  {
    /**
     * @brief Initializes a field containing a single data type implementing
     *        IReflectable interface.
     * @param[in] name      Name of the field.
     * @param[in] uniqueId  Unique identifier for this field. Although name is
     *            also a unique identifier we want a small data type that can
     *            be used for efficiently serializing data to disk and similar.
     *            It is primarily used for compatibility between different
     *            versions of serialized data.
     * @param[in] getter    The getter method for the field. Must be a specific
     *                      signature: DataType&(ObjectType*)
     * @param[in] setter    The setter method for the field. Must be a specific
     *                      signature: void(ObjectType*, DataType)
     * @param[in] flags     Various flags you can use to specialize how systems
     *                      handle this field. See "RTTIFieldFlag".
     */
    void
    initSingle(const String& name,
               uint16 uniqueId,
               Any getter,
               Any setter,
               uint64 flags) {
      initAll(getter,
              setter,
              nullptr,
              nullptr,
              name,
              uniqueId,
              false,
              SERIALIZABLE_FIELD_TYPE::kReflectable,
              flags);
    }

    /**
     * @brief Initializes a field containing an array of data types
     *        implementing IReflectable interface.
     * @param[in] name      Name of the field.
     * @param[in] uniqueId  Unique identifier for this field. Although name is
     *            also a unique identifier we want a small data type that can
     *            be used for efficiently serializing data to disk and similar.
     *            It is primarily used for compatibility between different
     *            versions of serialized data.
     * @param[in] getter    The getter method for the field. Must be a specific
     *            signature: DataType&(ObjectType*, uint32)
     * @param[in] getSize   Getter method that returns the size of an array.
     *            Must be a specific signature: uint32(ObjectType*)
     * @param[in] setter    The setter method for the field. Must be a specific
     *            signature: void(ObjectType*, uint32, DataType)
     * @param[in] setSize   Setter method that allows you to resize an array.
     *            Must be a specific signature: void(ObjectType*, uint32)
     * @param[in] flags     Various flags you can use to specialize how systems
     *            handle this field. See "RTTIFieldFlag".
     */
    void
    initArray(const String& name,
              uint16 uniqueId,
              Any getter,
              Any getSize,
              Any setter,
              Any setSize,
              uint64 flags) {
      initAll(getter,
              setter,
              getSize,
              setSize,
              name,
              uniqueId,
              true,
              SERIALIZABLE_FIELD_TYPE::kReflectable,
              flags);
    }

    /**
     * @copydoc RTTIField::getTypeSize
     */
    uint32
    getTypeSize() override {
      return 0; //Complex types don't store size the conventional way
    }

    /**
     * @copydoc RTTIReflectableFieldBase::getValue
     */
    IReflectable&
    getValue(void* object) override {
      checkIsArray(false);

      ObjectType* castObjType = static_cast<ObjectType*>(object);
      function<DataType&(ObjectType*)>
      f = any_cast<function<DataType&(ObjectType*)>>(m_valueGetter);
      IReflectable& castDataType = f(castObjType);

      return castDataType;
    }

    /**
     * @copydoc RTTIReflectableFieldBase::getArrayValue
     */
    IReflectable&
    getArrayValue(void* object, uint32 index) override {
      checkIsArray(true);

      ObjectType* castObjType = static_cast<ObjectType*>(object);
      function<DataType&(ObjectType*, uint32)>
      f = any_cast<function<DataType&(ObjectType*, uint32)>>(m_valueGetter);
      IReflectable& castDataType = f(castObjType, index);

      return castDataType;
    }

    /**
     * @copydoc RTTIReflectableFieldBase::setValue
     */
    void
    setValue(void* object, IReflectable& value) override {
      checkIsArray(false);

      if (m_valueSetter.empty()) {
        GE_EXCEPT(InternalErrorException, "Specified field (" + m_name + ") has no setter.");
      }

      ObjectType* castObjType = static_cast<ObjectType*>(object);
      DataType& castDataObj = static_cast<DataType&>(value);
      function<void(ObjectType*, DataType&)>
      f = any_cast<function<void(ObjectType*, DataType&)>>(m_valueSetter);
      f(castObjType, castDataObj);
    }

    /**
     * @copydoc RTTIReflectableFieldBase::setArrayValue
     */
    void
    setArrayValue(void* object, uint32 index, IReflectable& value) override {
      checkIsArray(true);

      if (m_valueSetter.empty()) {
        GE_EXCEPT(InternalErrorException, "Specified field (" + m_name + ") has no setter.");
      }

      ObjectType* castObjType = static_cast<ObjectType*>(object);
      DataType& castDataObj = static_cast<DataType&>(value);
      function<void(ObjectType*, uint32, DataType&)>
      f = any_cast<std::function<void(ObjectType*, uint32, DataType&)>>(m_valueSetter);
      f(castObjType, index, castDataObj);
    }

    /**
     * @copydoc RTTIField::getArraySize
     */
    uint32
    getArraySize(void* object) override {
      checkIsArray(true);

      function<uint32(ObjectType*)>
      f = any_cast<function<uint32(ObjectType*)>>(m_arraySizeGetter);
      ObjectType* castObject = static_cast<ObjectType*>(object);
      return f(castObject);
    }

    /**
     * @copydoc RTTIField::setArraySize
     */
    void
    setArraySize(void* object, uint32 size) override {
      checkIsArray(true);

      if (m_arraySizeSetter.empty()) {
        GE_EXCEPT(InternalErrorException,
                  "Specified field (" + m_name + ") has no array size setter.");
      }

      function<void(ObjectType*, uint32)>
      f = any_cast<function<void(ObjectType*, uint32)>>(m_arraySizeSetter);
      ObjectType* castObject = static_cast<ObjectType*>(object);
      f(castObject, size);
    }

    /**
     * @copydoc RTTIReflectableFieldBase::newObject
     */
    SPtr<IReflectable>
    newObject() override {
      return DataType::getRTTIStatic()->newRTTIObject();
    }

    /**
     * @copydoc RTTIReflectableFieldBase::getType
     */
    RTTITypeBase*
    getType() override {
      return DataType::getRTTIStatic();
    }
  };
}
