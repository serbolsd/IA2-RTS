/*****************************************************************************/
/**
 * @file    geAny.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/06/01
 * @brief   Class capable of storing any general type, and safely extracting.
 *
 * Class capable of storing any general type, and safely extracting the proper
 * type from the internal data.
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
#include <algorithm>
#include <typeinfo>

#include "gePrerequisitesUtil.h"
#include "geDebug.h"

namespace geEngineSDK {
  /**
   * @brief Class capable of storing any general type, and safely extracting
   *        the proper type from the internal data.
   */
  class Any
  {
   private:
    class DataBase
    {
     public:
      virtual ~DataBase() = default;
      virtual DataBase*
      clone() const = 0;
    };

    template<typename ValueType>
    class Data : public DataBase
    {
     public:
      Data(const ValueType& value) : m_value(value) {}

      DataBase*
      clone() const override {
        return ge_new<Data>(Data(m_value));
      }

      ValueType m_value;
    };

   public:
    Any() = default;

    template<typename ValueType>
    Any(const ValueType& value) : m_data(ge_new<Data<ValueType>>(value)) {}
    Any(std::nullptr_t) : m_data(nullptr) {}
    Any(const Any& other) 
      : m_data(nullptr != other.m_data ? other.m_data->clone() : nullptr) {}

    ~Any() {
      if (nullptr != m_data) {
        ge_delete(m_data);
      }
    }

    /**
     * @brief Swaps the contents of this object with another.
     */
    Any&
    swap(Any& rhs) {
      std::swap(m_data, rhs.m_data);
      return *this;
    }

    template<typename ValueType>
    Any&
    operator=(const ValueType& rhs) {
      Any(rhs).swap(*this);
      return *this;
    }

    Any&
    operator=(const Any& rhs) {
      Any(rhs).swap(*this);
      return *this;
    }

    /**
     * @brief Returns true if no type is set.
     */
    bool
    empty() const {
      return nullptr == m_data;
    }

   private:
    template<typename ValueType>
    friend ValueType*
    any_cast(Any*);

    template<typename ValueType>
    friend ValueType*
    any_cast_unsafe(Any*);

   public:
    DataBase* m_data = nullptr;
  };

  /**
   * @brief Returns a pointer to the internal data of the specified type.
   * @note  Will return null if cast fails.
   */
  template<typename ValueType>
  ValueType*
  any_cast(Any* operand) {
    if (nullptr != operand) {
      return &static_cast<Any::Data<ValueType>*>(operand->m_data)->m_value;
    }
    return nullptr;
  }

  /**
   * @brief Returns a const pointer to the internal data of the specified type.
   * @note  Will return null if cast fails.
   */
  template<typename ValueType>
  const ValueType*
  any_cast(const Any* operand) {
    return any_cast<ValueType>(const_cast<Any*>(operand));
  }

  /**
   * @brief Returns a copy of the internal data of the specified type.
   * @note  Throws an exception if cast fails.
   */
  template<typename ValueType>
  ValueType
  any_cast(const Any& operand) {
    return *any_cast<ValueType>(const_cast<Any*>(&operand));
  }

  /**
   * @brief Returns a copy of the internal data of the specified type.
   * @note  Throws an exception if cast fails.
   */
  template<typename ValueType>
  ValueType
  any_cast(Any& operand) {
    return *any_cast<ValueType>(&operand);
  }

  /**
   * @brief Returns a reference to the internal data of the specified type.
   * @note  Throws an exception if cast fails.
   */
  template<typename ValueType>
  const ValueType&
  any_cast_ref(const Any & operand) {
    return *any_cast<ValueType>(const_cast<Any*>(&operand));
  }

  /**
   * @brief Returns a reference to the internal data of the specified type.
   * @note  Throws an exception if cast fails.
   */
  template<typename ValueType>
  ValueType&
  any_cast_ref(Any& operand) {
    return *any_cast<ValueType>(&operand);
  }

  /**
   * @brief Casts a type without performing any kind of checks.
   */
  template<typename ValueType>
  ValueType*
  any_cast_unsafe(Any* operand) {
    return &static_cast<Any::Data<ValueType>*>(operand->m_data)->m_value;
  }

  /**
   * @brief Casts a type without performing any kind of checks.
   */
  template<typename ValueType>
  const ValueType*
  any_cast_unsafe(const Any* operand) {
    return any_cast_unsafe<ValueType>(const_cast<Any*>(operand));
  }
}
