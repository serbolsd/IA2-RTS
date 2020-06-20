/*****************************************************************************/
/**
 * @file    geRTTIField.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/06/11
 * @brief   Structure that keeps meta-data concerning a single class field.
 *
 * Structure that keeps meta-data concerning a single class field. You can use
 * this data for setting and getting values for that field on a specific class
 * instance.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "geRTTIField.h"
#include "geException.h"

namespace geEngineSDK {
  void
  RTTIField::checkIsPlain(bool array) {
    if (!isPlainType()) {
      GE_EXCEPT(InternalErrorException,
                "Invalid field type. Needed: Plain type. Got: " +
                toString(m_isVectorType) + ", " +
                toString(isPlainType()) + ", " +
                toString(isReflectableType()) + ", " + 
                toString(isDataBlockType()) + ", " + 
                toString(isReflectablePtrType()));
    }
    checkIsArray(array);
  }

  void
  RTTIField::checkIsDataBlock() {
    if (!isDataBlockType()) {
      GE_EXCEPT(InternalErrorException,
                "Invalid field type. Needed: Data block. Got: " + 
                toString(m_isVectorType) + ", " +
                toString(isPlainType()) + ", " + 
                toString(isReflectableType()) + ", " + 
                toString(isDataBlockType()) + ", " + 
                toString(isReflectablePtrType()));
    }
  }

  void
  RTTIField::checkIsComplex(bool array) {
    if (!isReflectableType()) {
      GE_EXCEPT(InternalErrorException,
                "Invalid field type. Needed: Complex type. Got: " + 
                toString(m_isVectorType) + ", " +
                toString(isPlainType()) + ", " + 
                toString(isReflectableType()) + ", " + 
                toString(isDataBlockType()) + ", " + 
                toString(isReflectablePtrType()));
    }
    checkIsArray(array);
  }

  void
  RTTIField::checkIsComplexPtr(bool array) {
    if (!isReflectablePtrType()) {
      GE_EXCEPT(InternalErrorException,
                "Invalid field type. Needed: Complex ptr type. Got: " +
                toString(m_isVectorType) + ", " +
                toString(isPlainType()) + ", " +
                toString(isReflectableType()) + ", " +
                toString(isDataBlockType()) + ", " +
                toString(isReflectablePtrType()));
    }
    checkIsArray(array);
  }

  void
  RTTIField::checkIsArray(bool array) {
    if (array && !m_isVectorType) {
      GE_EXCEPT(InternalErrorException,
                "Invalid field type. Needed an array type but got a single type.");
    }

    if (!array && m_isVectorType) {
      GE_EXCEPT(InternalErrorException,
                "Invalid field type. Needed a single type but got an array type.");
    }
  }
}
