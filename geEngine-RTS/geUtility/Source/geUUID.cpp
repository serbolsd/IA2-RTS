/*****************************************************************************/
/**
 * @file    geUUID.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/10/29
 * @brief   Class that represents a universally unique identifier.
 *
 * Class that represents a universally unique identifier.
 *
 * @bug	    No known bugs.
 */
/*****************************************************************************/
#pragma once

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "gePrerequisitesUtil.h"
#include "geUUID.h"
#include "gePlatformUtility.h"

namespace {
  constexpr const char HEX_TO_LITERAL[16] = {
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'a',
    'b',
    'c',
    'd',
    'e',
    'f'
  };

  constexpr const geEngineSDK::uint8 LITERAL_TO_HEX[256] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    //0 through 9 translate to 0 though 9
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    //A through F translate to 10 though 15
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    //a through f translate to 10 though 15
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
}

namespace geEngineSDK {
  UUID UUID::EMPTY;

  UUID::UUID(const String& uuid) {
    memset(m_data, 0, sizeof(m_data));

    if (uuid.size() < 36) {
      return;
    }

    uint32 idx = 0;

    //First group: 8 digits
    for (int32 i = 7; i >= 0; --i) {
      char charVal = uuid[idx++];
      uint8 hexVal = LITERAL_TO_HEX[static_cast<int>(charVal)];
      m_data[0] |= hexVal << (i * 4);
    }

    ++idx;

    //Second group: 4 digits
    for (int32 i = 7; i >= 4; --i) {
      char charVal = uuid[idx++];
      uint8 hexVal = LITERAL_TO_HEX[static_cast<int>(charVal)];
      m_data[1] |= hexVal << (i * 4);
    }

    ++idx;

    //Third group: 4 digits
    for (int32 i = 3; i >= 0; --i)
    {
      char charVal = uuid[idx++];
      uint8 hexVal = LITERAL_TO_HEX[static_cast<int>(charVal)];
      m_data[1] |= hexVal << (i * 4);
    }

    ++idx;

    //Fourth group: 4 digits
    for (int32 i = 7; i >= 4; --i)
    {
      char charVal = uuid[idx++];
      uint8 hexVal = LITERAL_TO_HEX[static_cast<int>(charVal)];
      m_data[2] |= hexVal << (i * 4);
    }

    ++idx;

    //Fifth group: 12 digits
    for (int32 i = 3; i >= 0; --i)
    {
      char charVal = uuid[idx++];
      uint8 hexVal = LITERAL_TO_HEX[static_cast<int>(charVal)];
      m_data[2] |= hexVal << (i * 4);
    }

    for (int32 i = 7; i >= 0; --i)
    {
      char charVal = uuid[idx++];
      uint8 hexVal = LITERAL_TO_HEX[static_cast<int>(charVal)];
      m_data[3] |= hexVal << (i * 4);
    }
  }

  String
  UUID::toString() const {
    uint8 output[36];
    uint32 idx = 0;

    //First group: 8 digits
    for (int32 i = 7; i >= 0; --i) {
      uint32 hexVal = (m_data[0] >> (i * 4)) & 0xF;
      output[idx++] = HEX_TO_LITERAL[hexVal];
    }

    output[idx++] = '-';

    //Second group: 4 digits
    for (int32 i = 7; i >= 4; --i) {
      uint32 hexVal = (m_data[1] >> (i * 4)) & 0xF;
      output[idx++] = HEX_TO_LITERAL[hexVal];
    }

    output[idx++] = '-';

    //Third group: 4 digits
    for (int32 i = 3; i >= 0; --i) {
      uint32 hexVal = (m_data[1] >> (i * 4)) & 0xF;
      output[idx++] = HEX_TO_LITERAL[hexVal];
    }

    output[idx++] = '-';

    //Fourth group: 4 digits
    for (int32 i = 7; i >= 4; --i) {
      uint32 hexVal = (m_data[2] >> (i * 4)) & 0xF;
      output[idx++] = HEX_TO_LITERAL[hexVal];
    }

    output[idx++] = '-';

    //Fifth group: 12 digits
    for (int32 i = 3; i >= 0; --i) {
      uint32 hexVal = (m_data[2] >> (i * 4)) & 0xF;
      output[idx++] = HEX_TO_LITERAL[hexVal];
    }

    for (int32 i = 7; i >= 0; --i) {
      uint32 hexVal = (m_data[3] >> (i * 4)) & 0xF;
      output[idx++] = HEX_TO_LITERAL[hexVal];
    }

    return String((const char*)output, 36);
  }

  UUID
  UUIDGenerator::generateRandom() {
    return PlatformUtility::generateUUID();
  }
}
