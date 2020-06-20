/*****************************************************************************/
/**
 * @file    geRTTIManagedDataBlockField.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/06/11
 * @brief   Base class with common functionality for a managed data block field
 *
 * Base class containing common functionality for a managed data block class
 * field. Managed data blocks are just blocks of memory that may, or may not be
 * released automatically when they are no longer referenced. They are useful
 * when wanting to return some temporary data only for serialization purposes.
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

namespace geEngineSDK {
  using std::function;

  /**
   * @brief Base class containing common functionality for a managed data block class field.
   * @note  Managed data blocks are just blocks of memory that may, or may not be released
   *        automatically when they are no longer referenced. They are useful when wanting
   *        to return some temporary data only for serialization purposes.
   */
  struct RTTIManagedDataBlockFieldBase : public RTTIField
  {
    /**
     * @brief Retrieves a managed data block from the specified instance.
     */
    virtual SPtr<DataStream>
    getValue(void* object, uint32& size) = 0;

    /**
     * @brief Sets a managed data block on the specified instance.
     */
    virtual void
    setValue(void* object, const SPtr<DataStream>& data, uint32 size) = 0;
  };

  /**
   * @brief Class containing a managed data block field containing a specific type.
   */
  template<class DataType, class ObjectType>
  struct RTTIManagedDataBlockField : public RTTIManagedDataBlockFieldBase
  {
    /**
     * @brief Initializes a field that returns a block of bytes.
     *        Can be used for serializing pretty much anything.
     * @param[in] name      Name of the field.
     * @param[in] uniqueId  Unique identifier for this field. Although name is also a unique
     *            identifier we want a small data type that can be used for efficiently
     *            serializing data to disk and similar. It is primarily used for
     *            compatibility between different versions of serialized data.
     * @param[in] getter    The getter method for the field. Must be a specific signature:
     *            SerializableDataBlock(ObjectType*)
     * @param[in] setter    The setter method for the field. Must be a specific signature:
     *            void(ObjectType*, SerializableDataBlock)
     * @param[in] flags     Various flags you can use to specialize how systems handle this
     *            field. See RTTIFieldFlag.
     */
    void
    initSingle(const String& name, uint16 uniqueId, Any getter, Any setter, uint64 flags) {
      initAll(getter,
              setter,
              nullptr,
              nullptr,
              name,
              uniqueId,
              false,
              SERIALIZABLE_FIELD_TYPE::kDataBlock,
              flags);
    }

    /**
     * @copydoc RTTIField::getTypeSize
     */
    uint32
    getTypeSize() override {
      return 0; //Data block types don't store size the conventional way
    }

    /**
     * @copydoc RTTIField::hasDynamicSize
     */
    bool
    hasDynamicSize() override {
      return true;
    }

    /**
     * @copydoc RTTIField::getArraySize
     */
    uint32
    getArraySize(void* /*object*/) override {
      GE_EXCEPT(InternalErrorException, "Data block types don't support arrays.");
      //return 0; //Unreachable code
    }

    /**
     * @copydoc RTTIField::setArraySize
     */
    void
    setArraySize(void* /*object*/, uint32 /*size*/) override {
      GE_EXCEPT(InternalErrorException, "Data block types don't support arrays.");
    }

    /**
     * @copydoc RTTIManagedDataBlockFieldBase::getValue
     */
    SPtr<DataStream>
    getValue(void* object, uint32& size) override {
      ObjectType* castObj = static_cast<ObjectType*>(object);
      function<SPtr<DataStream>(ObjectType*, uint32&)> f =
        any_cast<function<SPtr<DataStream>(ObjectType*, uint32&)>>(m_valueGetter);
      return f(castObj, size);
    }

    /**
     * @copydoc RTTIManagedDataBlockFieldBase::setValue
     */
    void
    setValue(void* object, const SPtr<DataStream>& value, uint32 size) override {
      ObjectType* castObj = static_cast<ObjectType*>(object);
      function<void(ObjectType*, const SPtr<DataStream>&, uint32)> f =
        any_cast<function<void(ObjectType*, const SPtr<DataStream>&, uint32)>>(m_valueSetter);
      f(castObj, value, size);
    }
  };
}
