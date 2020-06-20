/*****************************************************************************/
/**
 * @file    geFlags.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/10/14
 * @brief   Wrapper that allows simple use of bitwise logic operations.
 *
 * Wrapper around an enum that allows simple use of bitwise logic operations.
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
#include "gePlatformTypes.h"
#include "geFwdDeclUtil.h"

namespace geEngineSDK {

  template<typename Enum, typename Storage = uint32>
  class Flags
  {
   public:
    using InternalType = Storage;

    Flags() = default;

    Flags(Enum value) : m_bits(static_cast<Storage>(value)) {}
    Flags(const Flags<Enum, Storage>& value) : m_bits(value.m_bits) {}
    explicit Flags(Storage bits) : m_bits(bits) {}

    /**
     * @brief Checks whether all of the provided bits are set
     */
    bool
    isSet(Enum value) const {
      return (m_bits & static_cast<Storage>(value)) == static_cast<Storage>(value);
    }

    /**
     * @brief Activates all of the provided bits.
     */
    Flags<Enum, Storage>&
    set(Enum value) {
      m_bits |= static_cast<Storage>(value);
      return *this;
    }

    /**
     * @brief Deactivates all of the provided bits.
     */
    void
    unset(Enum value) {
      m_bits &= ~static_cast<Storage>(value);
    }

    bool
    operator==(Enum rhs) const {
      return m_bits == static_cast<Storage>(rhs);
    }

    bool
    operator==(const Flags<Enum, Storage>& rhs) const {
      return m_bits == rhs.m_bits;
    }

    bool
    operator==(bool rhs) const {
      return (static_cast<bool>(*this)) == rhs;
    }

    bool
    operator!=(Enum rhs) const {
      return m_bits != static_cast<Storage>(rhs);
    }

    bool
    operator!=(const Flags<Enum, Storage>& rhs) const {
      return m_bits != rhs.m_bits;
    }

    Flags<Enum, Storage>&
    operator=(Enum rhs) {
      m_bits = static_cast<Storage>(rhs);
      return *this;
    }

    Flags<Enum, Storage>&
    operator=(const Flags<Enum, Storage>& rhs) {
      m_bits = rhs.m_bits;
      return *this;
    }

    Flags<Enum, Storage>&
    operator|=(Enum rhs) {
      m_bits |= static_cast<Storage>(rhs);
      return *this;
    }

    Flags<Enum, Storage>&
    operator|=(const Flags<Enum, Storage>& rhs) {
      m_bits |= rhs.m_bits;
      return *this;
    }

    Flags<Enum, Storage>
    operator|(Enum rhs) const {
      Flags<Enum, Storage> out(*this);
      out |= rhs;
      return out;
    }

    Flags<Enum, Storage>
    operator|(const Flags<Enum, Storage>& rhs) const {
      Flags<Enum, Storage> out(*this);
      out |= rhs;
      return out;
    }

    Flags<Enum, Storage>&
    operator&=(Enum rhs) {
      m_bits &= static_cast<Storage>(rhs);
      return *this;
    }

    Flags<Enum, Storage>&
    operator&=(const Flags<Enum, Storage>& rhs) {
      m_bits &= rhs.m_bits;
      return *this;
    }

    Flags<Enum, Storage>
    operator&(Enum rhs) const {
      Flags<Enum, Storage> out = *this;
      out.m_bits &= static_cast<Storage>(rhs);
      return out;
    }

    Flags<Enum, Storage>
    operator&(const Flags<Enum, Storage>& rhs) const {
      Flags<Enum, Storage> out = *this;
      out.m_bits &= rhs.m_bits;
      return out;
    }

    Flags<Enum, Storage>&
    operator^=(Enum rhs) {
      m_bits ^= static_cast<Storage>(rhs);
      return *this;
    }

    Flags<Enum, Storage>&
    operator^=(const Flags<Enum, Storage>& rhs) {
      m_bits ^= rhs.m_bits;
      return *this;
    }

    Flags<Enum, Storage>
    operator^(Enum rhs) const {
      Flags<Enum, Storage> out = *this;
      out.m_bits ^= static_cast<Storage>(rhs);
      return out;
    }

    Flags<Enum, Storage>
    operator^(const Flags<Enum, Storage>& rhs) const {
      Flags<Enum, Storage> out = *this;
      out.m_bits ^= rhs.m_bits;
      return out;
    }

    Flags<Enum, Storage>
    operator~() const {
      Flags<Enum, Storage> out;
      out.m_bits = (Storage)~m_bits;
      return out;
    }

    operator bool() const {
      return m_bits ? true : false;
    }

    operator uint8() const {
      return static_cast<uint8>(m_bits);
    }

    operator uint16() const {
      return static_cast<uint16>(m_bits);
    }

    operator uint32() const {
      return static_cast<uint32>(m_bits);
    }

    friend Flags<Enum, Storage>
    operator&(Enum a, Flags<Enum, Storage> &b) {
      Flags<Enum, Storage> out;
      out.m_bits = a & b.m_bits;
      return out;
    }

   private:
    Storage m_bits{0};
  };

  /**
  * Defines global operators for a Flags<Enum, Storage> implementation.
  */
# define GE_FLAGS_OPERATORS(Enum) GE_FLAGS_OPERATORS_EXT(Enum, uint32)

  /**
  * Defines global operators for a Flags<Enum, Storage> implementation.
  */
# define GE_FLAGS_OPERATORS_EXT(Enum, Storage)                                \
  inline Flags<Enum, Storage>                                                 \
  operator|(Enum a, Enum b) {                                                 \
    Flags<Enum, Storage> r(a); r |= b; return r;                              \
  }                                                                           \
                                                                              \
  inline Flags<Enum, Storage>                                                 \
  operator&(Enum a, Enum b) {                                                 \
    Flags<Enum, Storage> r(a); r &= b; return r;                              \
  }                                                                           \
                                                                              \
  inline Flags<Enum, Storage>                                                 \
  operator~(Enum a) {                                                         \
    return ~Flags<Enum, Storage>(a);                                          \
  }

  /**
   * @brief RTTIPlainType for Flags.
   * @see   RTTIPlainType
   */
  template<class Enum, class Storage>
  struct RTTIPlainType<Flags<Enum, Storage>>
  {
    enum { kID = TYPEID_UTILITY::kID_Flags }; enum { kHasDynamicSize = 0 };

    /**
     * @copydoc RTTIPlainType::toMemory
     */
    static void
    toMemory(const Flags<Enum, Storage>& data, char* memory) {
      Storage storageData = static_cast<Storage>(data);
      RTTIPlainType<Storage>::toMemory(storageData, memory);
    }

    /**
     * @copydoc RTTIPlainType::fromMemory
     */
    static uint32
    fromMemory(Flags<Enum, Storage>& data, char* memory) {
      Storage storageData;
      RTTIPlainType<Storage>::fromMemory(storageData, memory);

      data = Flags<Enum, Storage>(storageData);
      return static_cast<uint32>(sizeof(Flags<Enum, Storage>));
    }

    /**
     * @copydoc RTTIPlainType::getDynamicSize
     */
    static uint32
    getDynamicSize(const Flags<Enum, Storage>& /*data*/) {
      GE_ASSERT(false); //This should never be called, but just in case
      return static_cast<uint32>(sizeof(Flags<Enum, Storage>));
    }
  };
}

namespace std {
  /**
   * @brief Hash value generator for Flags<Enum, Storage>.
   */
  template<class Enum, class Storage>
  struct hash<geEngineSDK::Flags<Enum, Storage>>
  {
    size_t
    operator()(const geEngineSDK::Flags<Enum, Storage>& key) const {
      return static_cast<Storage>(key);
    }
  };
}
