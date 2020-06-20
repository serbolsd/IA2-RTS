/*****************************************************************************/
/**
 * @file    geUnicode.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/10/14
 * @brief   Utilities to convert between UTF-8 encoding and other encodings.
 *
 * Provides methods to converting between UTF-8 character encoding and other
*  popular encodings.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
* Includes
*/
/*****************************************************************************/
#include "geUnicode.h"

namespace geEngineSDK {
  /**
   * @brief Converts an UTF-8 encoded character (possibly multibyte) into an UTF-32 character.
   */
  template<typename T>
  T
  UTF8To32(T begin, T end, char32_t& output, char32_t invalidChar = 0) {
    //Nothing to parse
    if (begin >= end) {
      return begin;
    }

    //Determine the number of bytes used by the character
    uint32 numBytes;

    uint8 firstByte = static_cast<uint8>(*begin);
    if (192 > firstByte) {
      numBytes = 1;
    }
    else if (224 > firstByte) {
      numBytes = 2;
    }
    else if (240 > firstByte) {
      numBytes = 3;
    }
    else if (248 > firstByte) {
      numBytes = 4;
    }
    else if (252 > firstByte) {
      numBytes = 5;
    }
    else {// < 256
      numBytes = 6;
    }

    //Not enough bytes were provided, invalid character
    if ((begin + numBytes) > end) {
      output = invalidChar;
      return end;
    }

    //Decode the character
    output = 0;
    switch (numBytes)
    {
      case 6: output += static_cast<uint8>(*begin); ++begin; output <<= 6; GE_FALLTHROUGH;
      case 5: output += static_cast<uint8>(*begin); ++begin; output <<= 6; GE_FALLTHROUGH;
      case 4: output += static_cast<uint8>(*begin); ++begin; output <<= 6; GE_FALLTHROUGH;
      case 3: output += static_cast<uint8>(*begin); ++begin; output <<= 6; GE_FALLTHROUGH;
      case 2: output += static_cast<uint8>(*begin); ++begin; output <<= 6; GE_FALLTHROUGH;
      case 1: output += static_cast<uint8>(*begin); ++begin; GE_FALLTHROUGH;
      default: break;
    }

    constexpr uint32 offsets[6] = { 0x00000000,
                                    0x00003080,
                                    0x000E2080,
                                    0x03C82080,
                                    0xFA082080,
                                    0x82082080 };
    output -= offsets[numBytes - 1];
    return begin;
  }

  /**
   * @brief Converts an UTF-32 encoded character into an (possibly multibyte) UTF-8 character.
   */
  template<typename T>
  T
  UTF32To8(char32_t input, T output, uint32 maxElems, char invalidChar = 0) {
    //No place to write the character
    if (0 == maxElems) {
      return output;
    }

    //Check if character is valid
    if ((0x0010FFFF < input) || ((0xD800 <= input) && (0xDBFF >= input))) {
      *output = invalidChar;
      ++output;
      return output;
    }

    //Determine the number of bytes used by the character
    uint32 numBytes;
    if (0x80 > input) {
      numBytes = 1;
    }
    else if (0x800 > input) {
      numBytes = 2;
    }
    else if (0x10000 > input) {
      numBytes = 3;
    }
    else {// <= 0x0010FFFF 
      numBytes = 4;
    }

    //Check if we have enough space
    if (numBytes > maxElems) {
      *output = invalidChar;
      ++output;
      return output;
    }

    //Encode the character
    constexpr uint8 headers[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

    char bytes[4];
    switch (numBytes)
    {
      case 4:
        bytes[3] = static_cast<char>((input | 0x80) & 0xBF); input >>= 6; GE_FALLTHROUGH;
      case 3:
        bytes[2] = static_cast<char>((input | 0x80) & 0xBF); input >>= 6; GE_FALLTHROUGH;
      case 2:
        bytes[1] = static_cast<char>((input | 0x80) & 0xBF); input >>= 6; GE_FALLTHROUGH;
      case 1:
        bytes[0] = static_cast<char>(input | headers[numBytes]); GE_FALLTHROUGH;
      default:
        break;
    }

    output = std::copy(bytes, bytes + numBytes, output);
    return output;
  }

  /**
   * @brief Converts an UTF-16 encoded character into an UTF-32 character.
   */
  template<typename T>
  T
  UTF16To32(T begin, T end, char32_t& output, char32_t invalidChar = 0) {
    //Nothing to parse
    if (begin >= end) {
      return begin;
    }

    char16_t firstElem = static_cast<char16_t>(*begin);
    ++begin;

    //Check if it's a surrogate pair
    if ((0xD800 <= firstElem) && (0xDBFF >= firstElem)) {
      //Invalid character
      if (begin >= end) {
        output = invalidChar;
        return end;
      }

      char32_t secondElem = static_cast<char32_t>(*begin);
      ++begin;

      if ((0xDC00 <= secondElem) && (0xDFFF >= secondElem)) {
        output = static_cast<char32_t>(((firstElem - 0xD800) << 10) +
                                        (secondElem - 0xDC00) + 0x0010000);
      }
      else {// Invalid character
        output = invalidChar;
      }
    }
    else {
      output = static_cast<char32_t>(firstElem);
      return begin;
    }

    return begin;
  }

  /**
   * @brief Converts an UTF-32 encoded character into an UTF-16 character.
   */
  template<typename T>
  T
  UTF32To16(char32_t input, T output, uint32 maxElems, char16_t invalidChar = 0) {
    //No place to write the character
    if (0 == maxElems) {
      return output;
    }

    //Invalid character
    if (0x0010FFFF < input) {
      *output = invalidChar;
      ++output;
      return output;
    }

    //Can be encoded as a single element
    if (0xFFFF >= input) {
      //Check if in valid range
      if ((0xD800 <= input) && (0xDFFF >= input)) {
        *output = invalidChar;
        ++output;
        return output;
      }

      *output = static_cast<char16_t>(input);
      ++output;
    }
    else {  //Must be encoded as two elements
      //Two elements won't fit
      if (2 > maxElems) {
        *output = invalidChar;
        ++output;
        return output;
      }

      input -= 0x0010000;

      *output = static_cast<char16_t>((input >> 10) + 0xD800);
      ++output;

      *output = static_cast<char16_t>((input & 0x3FFUL) + 0xDC00);
      ++output;
    }

    return output;
  }

  template<typename T>
  T
  wideToUTF32(T begin, T end, char32_t& output, char32_t invalidChar = 0) {
    //Assuming UTF-32 (i.e. Unix)
    SIZE_T sizeofWChar = sizeof(wchar_t);
    if (4 == sizeofWChar) {
      output = (char32_t)*begin;
      ++begin;

      return begin;
    }
    
    //Assuming UTF-16 (i.e. Windows)
    return UTF16To32(begin, end, output, invalidChar);
  }

  char32_t
  ANSIToUTF32(char input, const std::locale& locale = std::locale("")) {
    const std::ctype<wchar_t>& facet = std::use_facet<std::ctype<wchar_t>>(locale);

    /**
     * Note: Not exactly valid on Windows, since the input character could
     * require a surrogate pair. Consider improving this if it ever becomes an issue.
     */
    wchar_t wideChar = facet.widen(input);

    char32_t output;
    wideToUTF32(&wideChar, &wideChar + 1, output);

    return output;
  }

  template<typename T>
  T
  UTF32ToWide(char32_t input, T output, uint32 maxElems, wchar_t invalidChar = 0) {
    //Assuming UTF-32 (i.e. Unix)
    SIZE_T sizeofWChar = sizeof(wchar_t);
    if (4 == sizeofWChar) {
      *output = (wchar_t)input;
      ++output;
      return output;
    }
    
    //Assuming UTF-16 (i.e. Windows)
    return UTF32To16(input, output, maxElems, invalidChar);
  }

  char
  UTF32ToANSI(char32_t input,
              char invalidChar = 0,
              const std::locale& locale = std::locale("")) {
    const std::ctype<wchar_t>& facet = std::use_facet<std::ctype<wchar_t>>(locale);

    //Note: Same as above, not exactly correct as narrow() doesn't accept a surrogate pair
    return facet.narrow((wchar_t)input, invalidChar);
  }

  String
  UTF8::fromANSI(const String& input, const std::locale& locale) {
    String output;
    output.reserve(input.size());

    auto backInserter = std::back_inserter(output);

    auto iter = input.begin();
    while (iter != input.end()) {
      char32_t u32char = ANSIToUTF32(*iter, locale);
      UTF32To8(u32char, backInserter, 4);
      ++iter;
    }

    return output;
  }

  String
  UTF8::toANSI(const String& input, const std::locale& locale, char invalidChar) {
    String output;

    auto iter = input.begin();
    while (iter != input.end()) {
      char32_t u32char = 0;
      iter = UTF8To32(iter, input.end(), u32char, invalidChar);
      output.push_back(UTF32ToANSI(u32char, invalidChar, locale));
    }

    return output;
  }

  String
  UTF8::fromWide(const WString& input) {
    String output;
    output.reserve(input.size());

    auto backInserter = std::back_inserter(output);

    auto iter = input.begin();
    while (iter != input.end()) {
      char32_t u32char = 0;
      iter = wideToUTF32(iter, input.end(), u32char);
      UTF32To8(u32char, backInserter, 4);
    }

    return output;
  }

  WString
  UTF8::toWide(const String& input) {
    WString output;
    auto backInserter = std::back_inserter(output);

    auto iter = input.begin();
    while (iter != input.end()) {
      char32_t u32char = 0;
      iter = UTF8To32(iter, input.end(), u32char);
      UTF32ToWide(u32char, backInserter, 2);
    }

    return output;
  }

  String
  UTF8::fromUTF16(const U16String& input) {
    String output;
    output.reserve(input.size());

    auto backInserter = std::back_inserter(output);

    auto iter = input.begin();
    while (iter != input.end()) {
      char32_t u32char = 0;
      iter = UTF16To32(iter, input.end(), u32char);
      UTF32To8(u32char, backInserter, 4);
    }

    return output;
  }

  U16String
  UTF8::toUTF16(const String& input) {
    U16String output;
    auto backInserter = std::back_inserter(output);

    auto iter = input.begin();
    while (iter != input.end()) {
      char32_t u32char = 0;
      iter = UTF8To32(iter, input.end(), u32char);
      UTF32To16(u32char, backInserter, 2);
    }

    return output;
  }

  String
  UTF8::fromUTF32(const U32String& input) {
    String output;
    output.reserve(input.size());

    auto backInserter = std::back_inserter(output);

    auto iter = input.begin();
    while (iter != input.end()) {
      UTF32To8(*iter, backInserter, 4);
      ++iter;
    }

    return output;
  }

  U32String
  UTF8::toUTF32(const String& input) {
    U32String output;

    auto iter = input.begin();
    while (iter != input.end()) {
      char32_t u32char;
      iter = UTF8To32(iter, input.end(), u32char);
      output.push_back(u32char);
    }

    return output;
  }

  SIZE_T
  UTF8::count(const String& input) {
    SIZE_T length = 0;
    for (ANSICHAR i : input) {
      //Include only characters that don't start with bits 10
      length += (i & 0xc0) != 0x80;
    }
    return length;
  }

  SIZE_T
  UTF8::charToByteIndex(const String& input, SIZE_T charIdx) {
    SIZE_T curChar = 0;
    SIZE_T curByte = 0;
    for (ANSICHAR i : input) {
      //Include only characters that don't start with bits 10
      if ((i & 0xc0) != 0x80) {
        if (curChar == charIdx) {
          return curByte;
        }
        ++curChar;
      }
      ++curByte;
    }

    return input.size();
  }

  SIZE_T
  UTF8::charByteCount(const String& input, SIZE_T charIdx) {
    const SIZE_T byteIdx = charToByteIndex(input, charIdx);

    SIZE_T count = 1;
    for (auto i = byteIdx + 1; i < input.size(); ++i) {
      if ((i & 0xc0) != 0x80) {
        break;
      }
      ++count;
    }
    return count;
  }

  String
  UTF8::toLower(const String& input) {
    return PlatformUtility::convertCaseUTF8(input, false);
  }

  String
  UTF8::toUpper(const String& input) {
    return PlatformUtility::convertCaseUTF8(input, true);
  }
}
