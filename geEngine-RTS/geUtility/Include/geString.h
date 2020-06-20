/*****************************************************************************/
/**
 * @file    geString.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2015/02/18
 * @brief   Base classes for the use of Strings objects.
 *
 * Base classes for the use of Strings objects
 * (CHAR Strings - ASCII, WCHAR - UNICODE and String Streams)
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
#include "gePlatformDefines.h"
#include "geRTTIPrerequisites.h"
#include "geMemoryAllocator.h"
#include <string>

namespace geEngineSDK {
  using std::char_traits;
  using std::basic_string;
  using std::basic_stringstream;
  using std::min;
  using std::forward;
  using std::ios;

  /**
   * @brief Basic string that uses geEngine memory allocators.
   */
  template<typename T>
  using BasicString = basic_string<T, char_traits<T>, StdAlloc<T>>;

  /**
   * @brief Basic string stream that uses geEngine memory allocators.
   */
  template<typename T>
  using BasicStringStream = basic_stringstream<T, char_traits<T>, StdAlloc<T>>;

  /**
   * @brief Wide string used primarily for handling Unicode text.
   */
  using WString = BasicString<UNICHAR>;

  /**
   * @brief Narrow string used primarily for handling ASCII text.
   */
  using String = BasicString<ANSICHAR>;

  /**
   * @brief Wide string used UTF-16 encoded strings.
   */
  using U16String = BasicString<char16_t>;

  /**
   * @brief Wide string used UTF-32 encoded strings.
   */
  using U32String = BasicString<char32_t>;

  /**
   * @brief Wide string stream used for primarily for constructing strings
   *        consisting of Unicode text.
   */
  using WStringStream = BasicStringStream<UNICHAR>;

  /**
   * @brief Wide string stream used for primarily for constructing strings
   *        consisting of ASCII text.
   */
  using StringStream = BasicStringStream<ANSICHAR>;

  /**
   * @brief Wide string stream used primarily for constructing UTF-16 strings.
   */
  using U16StringStream = BasicStringStream<char16_t>;

  /**
   * @brief Wide string stream used primarily for constructing UTF-32 strings.
   */
  using U32StringStream = BasicStringStream<char32_t>;

  /**
   * @brief Equivalent to String, except it avoids any dynamic allocations
   *        until the number of elements exceeds @p Count.
   */
  //TODO: Currently equivalent to String, need to implement the allocator
  template <int Count>
  using SmallString = basic_string<char, char_traits<char>, StdAlloc<char>>;
}

#include "geStringFormat.h"

namespace geEngineSDK {
  /**
   * @brief Utility class for manipulating Strings.
   */
  class GE_UTILITY_EXPORT StringUtil
  {
   public:
    /**
     * @brief	Removes any whitespace characters from beginning or end of the string.
     */
    static void
    trim(String& str, bool left = true, bool right = true);

    /**
     * @copydoc StringUtil::trim(String&, bool, bool)
     */
    static void
    trim(WString& str, bool left = true, bool right = true);

    /**
     * @brief	Removes specified characters from beginning or end of the string.
     */
    static void
    trim(String& str, const String& delims, bool left = true, bool right = true);

    /**
     * @copydoc StringUtil::trim(String&, const String&, bool, bool)
     */
    static void
    trim(WString& str, const WString& delims, bool left = true, bool right = true);

    /**
     * @brief Returns a vector of strings containing all the substrings
     *        delimited by the provided delimiter characters.
     *
     * @param str       The string to split.
     * @param	delims    (optional) Delimiter characters to split the string by.
     *                  They will not be included in resulting substrings.
     * @param maxSplits (optional) The maximum number of splits to perform
     *                  (0 for unlimited splits). If this parameters is > 0,
     *                  the splitting process will stop after this many splits,
     *                  left to right.
     */
    static Vector<String>
    split(const String& str, const String& delims = "\t\n ", uint32 maxSplits = 0);

    /**
     * @copydoc	StringUtil::split(const String&, const String&, uint32)
     */
    static Vector<WString>
    split(const WString& str, const WString& delims = L"\t\n ", uint32 maxSplits = 0);

    /**
     * @brief Returns a vector of strings containing all the substrings
     *        delimited by the provided delimiter characters, or the double
     *        delimiters used for including normal delimiter characters in the
     *        tokenized string.
     *
     * @param str           The string to split.
     * @param	delims        (optional) Delimiter characters to split the string by.
     *                      They will not be included in resulting substrings.
     * @param	doubleDelims  (optional) Delimiter character you may use to surround
     *                      other normal delimiters, in order to include them in the
     *                      tokenized string.
     * @param maxSplits     (optional) The maximum number of splits to perform
     *                      (0 for unlimited splits). If this parameters is > 0,
     *                      the splitting process will stop after this many splits,
     *                      left to right.
     */
    static Vector<String>
    tokenise(const String& str,
             const String& singleDelims = "\t\n ",
             const String& doubleDelims = "\"", 
             uint32 maxSplits = 0);

    /**
     * @copydoc StringUtil::tokenise(const String&, const String&, const String&, uint32)
     */
    static Vector<WString>
    tokenise(const WString& str, 
             const WString& singleDelims = L"\t\n ",
             const WString& doubleDelims = L"\"", 
             uint32 maxSplits = 0);

    /**
     * @brief Converts all the characters in the string to lower case.
     */
    static void
    toLowerCase(String& str);

    /**
     * @brief	Converts all the characters in the string to lower case.
     */
    static void
    toLowerCase(WString& str);

    /**
     * @brief Converts all the characters in the string to upper case.
     */
    static void
    toUpperCase(String& str);

    /**
     * @brief Converts all the characters in the string to upper case.
     */
    static void
    toUpperCase(WString& str);

    /**
     * @brief	Returns whether the string begins with the pattern passed in.
     *
     * @param	str       String to compare.
     * @param	pattern   Pattern to compare with.
     * @param	lowerCase (optional) If true, the start of the string will be
     *                  lower cased before comparison and the pattern should
     *                  also be in lower case.
     */
    static bool
    startsWith(const String& str, const String& pattern, bool lowerCase = true);

    /**
     * @copydoc startsWith(const String&, const String&, bool)
     */
    static bool
    startsWith(const WString& str, const WString& pattern, bool lowerCase = true);

    /**
     * @brief	Returns whether the string end with the pattern passed in.
     *
     * @param	str       String to compare.
     * @param	pattern   Pattern to compare with.
     * @param	lowerCase (optional) If true, the start of the string will be
     *                  lower cased before comparison, and the pattern should
     *                  also be in lower case.
     */
    static bool
    endsWith(const String& str, const String& pattern, bool lowerCase = true);

    /**
     * @copydoc endsWith(const String&, const String&, bool)
     */
    static bool
    endsWith(const WString& str, const WString& pattern, bool lowerCase = true);

    /**
     * @brief	Returns true if the string matches the provided pattern.
     *        Pattern may use a "*" wild card for matching any characters.
     *
     * @param	str           The string to test.
     * @param	pattern       Patterns to look for.
     * @param	caseSensitive (optional) Should the match be case sensitive or not.
     */
    static bool
    match(const String& str, const String& pattern, bool caseSensitive = true);

    /**
     * @copydoc match(const String&, const String&, bool)
     */
    static bool
    match(const WString& str, const WString& pattern, bool caseSensitive = true);

    /**
     * @brief Replace all instances of a substring with a another substring.
     * @param	source          String to search.
     * @param	replaceWhat     Substring to find and replace
     * @param	replaceWithWhat Substring to replace with (the new sub-string)
     * @return	An updated string with the substrings replaced.
     */
    static const String
    replaceAll(const String& source, 
               const String& replaceWhat, 
               const String& replaceWithWhat);

    /**
     * @copydoc  replaceAll(const String&, const String&, const String&)
     */
    static const WString
    replaceAll(const WString& source, 
               const WString& replaceWhat, 
               const WString& replaceWithWhat);

    /**
     * @brief Compares two strings. Returns 0 if the two compare equal,
     *        < 0 if the value of the left string is lower than of the right string, or
     *        > 0 if the value of the left string is higher than the right string.
     *
     * @param[in]  lhs           Left string to compare.
     * @param[in]  rhs           Right string to compare.
     * @param[in]  caseSensitive If true the comparison will consider uppercase
     *                           and lowercase characters different.
     */
    template <class T>
    static int32
    compare(const BasicString<T>& lhs,
            const BasicString<T>& rhs,
            bool caseSensitive = true) {
      if (caseSensitive) {
        return lhs.compare(rhs);
      }

      SIZE_T size = min(lhs.size(), rhs.size());
      for (SIZE_T i = 0; i < size; ++i) {
        if (toupper(lhs[i]) < toupper(rhs[i])) return -1;
        if (toupper(lhs[i]) > toupper(rhs[i])) return 1;
      }

      return (lhs.size() < rhs.size() ? -1 : (lhs.size() == rhs.size() ? 0 : 1));
    }

    /**
     * @copydoc StringFormat::format
     */
    template<class T, class... Args>
    static BasicString<T>
    format(const BasicString<T>& source, Args&&... args) {
      return StringFormat::format(source.c_str(), forward<Args>(args)...);
    }

    /**
     * @copydoc StringFormat::format
     */
    template<class T, class... Args>
    static BasicString<T>
    format(const T* source, Args&&... args) {
      return StringFormat::format(source, forward<Args>(args)...);
    }

    /**
     * @brief Constant blank string, useful for returning by ref where local does not exist.
     */
    static const String BLANK;

    /**
     * @brief Constant blank wide string, useful for returning by ref where
     *        local does not exist.
     */
    static const WString WBLANK;

   private:
    template <class T>
    static Vector<BasicString<T>>
    splitInternal(const BasicString<T>& str, 
                  const BasicString<T>& delims,
                  uint32 maxSplits) {
      Vector<BasicString<T>> ret;
      //Preallocate some space for performance
      ret.reserve(maxSplits ? maxSplits + 1 : 10);  //10 is guessed capacity for most cases

      uint32 numSplits = 0;

      //Use STL methods 
      SIZE_T start, pos;
      start = 0;
      do {
        pos = str.find_first_of(delims, start);
        if (pos == start) {
          start = pos + 1;
        }
        else if (pos == BasicString<T>::npos || (maxSplits && numSplits == maxSplits)) {
          //Copy the rest of the string
          ret.push_back(str.substr(start));
          break;
        }
        else {//Copy up to delimiter
          ret.push_back(str.substr(start, pos - start));
          start = pos + 1;
        }

        //Parse up to next real data
        start = str.find_first_not_of(delims, start);
        ++numSplits;

      } while (pos != BasicString<T>::npos);

      return ret;
    }

    template <class T>
    static Vector<BasicString<T>>
    tokeniseInternal(const BasicString<T>& str, 
                     const BasicString<T>& singleDelims, 
                     const BasicString<T>& doubleDelims, 
                     uint32 maxSplits) {
      Vector<BasicString<T>> ret;
      //Preallocate some space for performance
      ret.reserve(maxSplits ? maxSplits + 1 : 10);  // 10 is guessed capacity for most cases

      uint32 numSplits = 0;
      BasicString<T> delims = singleDelims + doubleDelims;

      //Use STL methods 
      SIZE_T start, pos;
      T curDoubleDelim = 0;
      start = 0;
      do {
        if (0 != curDoubleDelim ) {
          pos = str.find(curDoubleDelim, start);
        }
        else {
          pos = str.find_first_of(delims, start);
        }

        if (pos == start) {
          T curDelim = str.at(pos);
          if (doubleDelims.find_first_of(curDelim) != BasicString<T>::npos) {
            curDoubleDelim = curDelim;
          }
          // Do nothing
          start = pos + 1;
        }
        else if (pos == BasicString<T>::npos || (maxSplits && numSplits == maxSplits)) {
          if (curDoubleDelim != 0) {
            //Missing closer. Warn or throw exception?
          }
          // Copy the rest of the string
          ret.push_back(str.substr(start));
          break;
        }
        else {
          if (curDoubleDelim != 0) {
            curDoubleDelim = 0;
          }

          // Copy up to delimiter
          ret.push_back(str.substr(start, pos - start));
          start = pos + 1;
        }
        if (0 == curDoubleDelim) {
          // parse up to next real data
          start = str.find_first_not_of(singleDelims, start);
        }

        ++numSplits;

      } while (pos != BasicString<T>::npos);

      return ret;
    }

    template <class T>
    static bool
    startsWithInternal(const BasicString<T>& str, 
                       const BasicString<T>& pattern, 
                       bool lowerCase) {
      SIZE_T thisLen = str.length();
      SIZE_T patternLen = pattern.length();
      if (thisLen < patternLen || 0 == patternLen) {
        return false;
      }

      BasicString<T> startOfThis = str.substr(0, patternLen);
      if (lowerCase) {
        StringUtil::toLowerCase(startOfThis);
      }

      return (startOfThis == pattern);
    }

    template <class T>
    static bool
    endsWithInternal(const BasicString<T>& str, 
                     const BasicString<T>& pattern, 
                     bool lowerCase) {
      SIZE_T thisLen = str.length();
      SIZE_T patternLen = pattern.length();
      if (thisLen < patternLen || 0 == patternLen) {
        return false;
      }

      BasicString<T> endOfThis = str.substr(thisLen - patternLen, patternLen);
      if (lowerCase) {
        StringUtil::toLowerCase(endOfThis);
      }

      return (endOfThis == pattern);
    }

    template <class T>
    static bool
    matchInternal(const BasicString<T>& str, 
                  const BasicString<T>& pattern, 
                  bool caseSensitive) {
      BasicString<T> tmpStr = str;
      BasicString<T> tmpPattern = pattern;
      if (!caseSensitive) {
        StringUtil::toLowerCase(tmpStr);
        StringUtil::toLowerCase(tmpPattern);
      }

      typename BasicString<T>::const_iterator strIt = tmpStr.begin();
      typename BasicString<T>::const_iterator patIt = tmpPattern.begin();
      typename BasicString<T>::const_iterator lastWildCardIt = tmpPattern.end();
      while (strIt != tmpStr.end() && patIt != tmpPattern.end()) {
        if (*patIt == '*') {
          lastWildCardIt = patIt;

          //Skip over looking for next character
          ++patIt;
          if (patIt == tmpPattern.end()) {
            // Skip right to the end since * matches the entire rest of the string
            strIt = tmpStr.end();
          }
          else {
            // scan until we find next pattern character
            while (strIt != tmpStr.end() && *strIt != *patIt) {
              ++strIt;
            }
          }
        }
        else {
          if (*patIt != *strIt) {
            if (lastWildCardIt != tmpPattern.end()) {
              //The last wildcard can match this incorrect sequence
              //rewind pattern to wildcard and keep searching
              patIt = lastWildCardIt;
              lastWildCardIt = tmpPattern.end();
            }
            else {
              //No wildcards left
              return false;
            }
          }
          else {
            ++patIt;
            ++strIt;
          }
        }

      }

      // If we reached the end of both the pattern and the string, we succeeded
      if (patIt == tmpPattern.end() && strIt == tmpStr.end()) {
        return true;
      }
      else {
        return false;
      }
    }

    template <class T>
    static const BasicString<T>
    replaceAllInternal(const BasicString<T>& source, 
                       const BasicString<T>& replaceWhat, 
                       const BasicString<T>& replaceWithWhat) {
      BasicString<T> result = source;
      typename BasicString<T>::size_type pos = 0;
      for (;;) {
        pos = result.find(replaceWhat, pos);
        if (pos == BasicString<T>::npos) break;
        result.replace(pos, replaceWhat.size(), replaceWithWhat);
        pos += replaceWithWhat.size();
      }
      return result;
    }
  };

  /***************************************************************************/
  /**
  * String conversion functions
  */
  /***************************************************************************/
  
  /**
   * @brief Converts a narrow string to a wide string.
   */
  GE_UTILITY_EXPORT WString
  toWString(const String& source);

  /**
   * @brief Converts a float to a wide string.
   */
  GE_UTILITY_EXPORT WString
  toWString(float val,
            uint16 precision = 6,
            uint16 width = 0,
            char fill = ' ',
            ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts a double to a wide string.
   */
  GE_UTILITY_EXPORT WString
  toWString(double val,
            uint16 precision = 6,
            uint16 width = 0,
            char fill = ' ',
            ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts a Radian to a wide string.
   */
  GE_UTILITY_EXPORT WString
  toWString(Radian val,
            uint16 precision = 6,
            uint16 width = 0,
            char fill = ' ',
            ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts a Degree to a wide string.
   */
  GE_UTILITY_EXPORT WString 
  toWString(Degree val,
            uint16 precision = 6,
            uint16 width = 0,
            char fill = ' ',
            ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts an int to a wide string.
   */
  GE_UTILITY_EXPORT WString 
  toWString(int32 val,
            uint16 width = 0,
            char fill = ' ',
            ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts an unsigned int to a wide string.
   */
  GE_UTILITY_EXPORT WString 
  toWString(uint32 val,
            uint16 width = 0,
            char fill = ' ',
            ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts an 64bit integer to a wide string.
   */
  GE_UTILITY_EXPORT WString 
  toWString(int64 val,
            uint16 width = 0,
            char fill = ' ',
            ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts an 64bit unsigned to a wide string.
   */
  GE_UTILITY_EXPORT WString 
  toWString(uint64 val,
            uint16 width = 0,
            char fill = ' ',
            ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts an narrow char unsigned to a wide string.
   */
  GE_UTILITY_EXPORT WString 
  toWString(ANSICHAR val,
            uint16 width = 0,
            char fill = ' ',
            ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts an wide bit char unsigned to a wide string.
   */
  GE_UTILITY_EXPORT WString 
  toWString(UNICHAR val,
            uint16 width = 0,
            char fill = ' ',
            ios::fmtflags flags = ios::fmtflags(0));

  /**
   * @brief	Converts a boolean to a wide string.
   * @param	val   Value to convert.
   * @param	yesNo (optional) If set to true, result is "yes" or "no"
   *              instead of "true" or "false".
   */
  GE_UTILITY_EXPORT WString 
  toWString(bool val, bool yesNo = false);

  /**
   * @brief Converts a 2 dimensional int vector to a wide string.
   * @note  Format is "x y".
   */
  GE_UTILITY_EXPORT WString
  toWString(const Vector2& val);

  /**
   * @brief Converts a 2 dimensional vector to a wide string.
   * @note  Format is "x y".
   */
  GE_UTILITY_EXPORT WString 
  toWString(const Vector2& val);

  /**
   * @brief Converts a 3 dimensional vector to a wide string.
   * @note  Format is "x y z".
   */
  GE_UTILITY_EXPORT WString 
  toWString(const Vector3& val);

  /**
   * @brief Converts a 4 dimensional vector to a wide string.
   * @note  Format is "x y z w".
   */
  GE_UTILITY_EXPORT WString 
  toWString(const Vector4& val);

  /**
   * @brief Converts a 4x4 matrix to a wide string.
   * @note  Format is "00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33".
   */
  GE_UTILITY_EXPORT WString 
  toWString(const Matrix4& val);

  /**
   * @brief Converts a Quaternion to a wide string.
   * @note  Format is "w x y z".
   */
  GE_UTILITY_EXPORT WString 
  toWString(const Quaternion& val);

  /**
   * @brief Converts a linear color to a string.
   * @note  Format is "r g b a".
   */
  GE_UTILITY_EXPORT WString 
  toWString(const LinearColor& val);

  /**
   * @brief Converts a color to a string.
   * @note  Format is "r g b a".
   */
  GE_UTILITY_EXPORT WString 
  toWString(const Color& val);

  /**
   * @brief Converts a vector of strings into a single string where the
   *        substrings are delimited by spaces.
   */
  GE_UTILITY_EXPORT WString 
  toWString(const Vector<geEngineSDK::WString>& val);

  /**
   * @brief Converts a wide string to a narrow string.
   */
  GE_UTILITY_EXPORT String 
  toString(const WString& source);

  /**
   * @brief Converts a float to a string.
   */
  GE_UTILITY_EXPORT String 
  toString(float val,
           uint16 precision = 6,
           uint16 width = 0,
           char fill = ' ',
           ios::fmtflags flags = ios::fmtflags(0));

  /**
   * @brief Converts a double to a string.
   */
  GE_UTILITY_EXPORT String 
  toString(double val,
           uint16 precision = 6,
           uint16 width = 0,
           char fill = ' ',
           ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts a Radian to a string.
   */
  GE_UTILITY_EXPORT String 
  toString(Radian val,
           uint16 precision = 6,
           uint16 width = 0,
           char fill = ' ',
           ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts a Degree to a string.
   */
  GE_UTILITY_EXPORT String 
  toString(Degree val,
           uint16 precision = 6,
           uint16 width = 0,
           char fill = ' ',
           ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts an int to a string.
   */
  GE_UTILITY_EXPORT String 
  toString(int32 val,
           uint16 width = 0,
           char fill = ' ',
           ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts an unsigned int to a string.
   */
  GE_UTILITY_EXPORT String 
  toString(uint32 val,
           uint16 width = 0,
           char fill = ' ',
           ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts a 64bit int to a string.
   */
  GE_UTILITY_EXPORT String 
  toString(int64 val,
           uint16 width = 0,
           char fill = ' ',
           ios::fmtflags flags = ios::fmtflags(0));
  
  /**
   * @brief Converts an 64bit unsigned int to a string.
   */
  GE_UTILITY_EXPORT String 
  toString(uint64 val,
           uint16 width = 0,
           char fill = ' ',
           ios::fmtflags flags = ios::fmtflags(0));

  /**
   * @brief Converts a boolean to a string.
   * @param	val   true to value.
   * @param	yesNo (optional) If set to true, result is "yes" or "no"
   *              instead of "true" or "false".
   */
  GE_UTILITY_EXPORT String 
  toString(bool val, bool yesNo = false);

  /**
   * @brief Converts a 2 dimensional vector to a string.
   * @note  Format is "x y".
   */
  GE_UTILITY_EXPORT String 
  toString(const Vector2& val);

  /**
   * @brief Converts a 2 dimensional int vector to a string.
   * @note  Format is "x y".
   */
  GE_UTILITY_EXPORT String
  toString(const Vector2I& val);

  /**
   * @brief Converts a 3 dimensional vector to a string.
   * @note  Format is "x y z".
   */
  GE_UTILITY_EXPORT String 
  toString(const Vector3& val);

  /**
   * @brief Converts a 4 dimensional vector to a string.
   * @note  Format is "x y z w".
   */
  GE_UTILITY_EXPORT String 
  toString(const Vector4& val);

  /**
   * @brief Converts a 4x4 matrix to a string.
   * @note  Format is "00 01 02 03 10 11 12 13 20 21 22 23 30 31 32 33".
   */
  GE_UTILITY_EXPORT String 
  toString(const Matrix4& val);

  /**
   * @brief Converts a Quaternion to a string.
   * @note  Format is "w x y z".
   */
  GE_UTILITY_EXPORT String 
  toString(const Quaternion& val);

  /**
   * @brief Converts a linear color to a string.
   * @note  Format is "r g b a".
   */
  GE_UTILITY_EXPORT String 
  toString(const LinearColor& val);

  /**
   * @brief Converts a color to a string.
   * @note  Format is "r g b a".
   */
  GE_UTILITY_EXPORT String 
  toString(const Color& val);

  /**
   * @brief Converts a vector of strings into a single string where the
   *        substrings are delimited by spaces.
   */
  GE_UTILITY_EXPORT String 
  toString(const Vector<geEngineSDK::String>& val);

  /**
   * @brief Converts a String to a float.
   * @note  0.0f if the value could not be parsed, otherwise the numeric
   *        version of the string.
   */
  GE_UTILITY_EXPORT float 
  parseFloat(const String& val, float defaultValue = 0);

  /**
   * @brief Converts a String to a whole number.
   * @note  0 if the value could not be parsed, otherwise the numeric
   *        version of the string.
   */
  GE_UTILITY_EXPORT int32 
  parseInt(const String& val, int32 defaultValue = 0);

  /**
   * @brief Converts a String to a whole number.
   * @note  0 if the value could not be parsed, otherwise the numeric
   *        version of the string.
   */
  GE_UTILITY_EXPORT uint32 
  parseUnsignedInt(const String& val, uint32 defaultValue = 0);

  /**
   * @brief Converts a String to a boolean.
   * @note  Returns true if case-insensitive match of the start of the string
   *        matches "true", "yes" or "1", false otherwise.
   */
  GE_UTILITY_EXPORT bool 
  parseBool(const String& val, bool defaultValue = false);

  /**
   * @brief Checks the String is a valid number value.
   */
  GE_UTILITY_EXPORT bool 
  isNumber(const String& val);

  /**
   * @brief Converts a WString to a float.
   * @note  0.0f if the value could not be parsed, otherwise the numeric
   *        version of the string.
   */
  GE_UTILITY_EXPORT float 
  parseFloat(const WString& val, float defaultValue = 0);

  /**
   * @brief Converts a WString to a whole number.
   * @note  0 if the value could not be parsed, otherwise the numeric version
   *        of the string.
   */
  GE_UTILITY_EXPORT int32 
  parseInt(const WString& val, int32 defaultValue = 0);

  /**
   * @brief Converts a WString to a whole number.
   * @note  0 if the value could not be parsed, otherwise the numeric version
   *        of the string.
   */
  GE_UTILITY_EXPORT uint32 
  parseUnsignedInt(const WString& val, uint32 defaultValue = 0);

  /**
   * @brief Converts a WString to a boolean.
   * @note  Returns true if case-insensitive match of the start of the string
   *        matches "true", "yes" or "1", false otherwise.
   */
  GE_UTILITY_EXPORT bool 
  parseBool(const WString& val, bool defaultValue = false);

  /**
   * @brief	Checks the WString is a valid number value.
   */
  GE_UTILITY_EXPORT bool 
  isNumber(const WString& val);

  /**
   * @brief Helper method that throws an exception regarding a data overflow.
   */
  void GE_UTILITY_EXPORT 
  __string_throwDataOverflowException();

  /**
   * @brief RTTIPlainType specialization for String that allows strings be
   *        serialized as value types.
   * @see   RTTIPlainType
   */
  template<>
  struct RTTIPlainType<String>
  {
    enum { kID = TYPEID_UTILITY::kID_String }; enum { kHasDynamicSize = 1 };

    static void
    toMemory(const String& data, char* memory) {
      uint32 size = getDynamicSize(data);

      memcpy(memory, &size, sizeof(uint32));
      memory += sizeof(uint32);
      size -= sizeof(uint32);
      memcpy(memory, data.data(), static_cast<SIZE_T>(size));
    }

    static uint32
    fromMemory(String& data, char* memory) {
      uint32 size;
      memcpy(&size, memory, sizeof(uint32));
      memory += sizeof(uint32);

      uint32 stringSize = size - sizeof(uint32);
      char* buffer = reinterpret_cast<char*>(ge_alloc(static_cast<SIZE_T>(stringSize + 1)));
      memcpy(buffer, memory, static_cast<SIZE_T>(stringSize));
      buffer[stringSize] = '\0';
      data = String(buffer);

      ge_free(buffer);

      return size;
    }

    static uint32
    getDynamicSize(const String& data) {
      uint64 dataSize = data.size() * sizeof(String::value_type) + sizeof(uint32);

#if GE_DEBUG_MODE
      uint64 maxSize = static_cast<uint64>(NumLimit::MAX_UINT32);
      if (dataSize > maxSize) {
        __string_throwDataOverflowException();
      }
#endif

      return static_cast<uint32>(dataSize);
    }
  };

  /**
  * @brief RTTIPlainType specialization for WString that allows strings be
  *        serialized as value types.
  * @see   RTTIPlainType
  */
  template<>
  struct RTTIPlainType<WString>
  {
    enum { kID = TYPEID_UTILITY::kID_WString }; enum { kHasDynamicSize = 1 };

    static void
    toMemory(const WString& data, char* memory) {
      uint32 size = getDynamicSize(data);

      memcpy(memory, &size, sizeof(uint32));
      memory += sizeof(uint32);
      size -= sizeof(uint32);
      memcpy(memory, data.data(), static_cast<SIZE_T>(size));
    }

    static uint32
    fromMemory(WString& data, char* memory) {
      using wcTemp = WString::value_type;

      uint32 size;
      memcpy(&size, memory, sizeof(uint32));
      memory += sizeof(uint32);

      uint32 stringSize = size - sizeof(uint32);
      wcTemp* buffer = reinterpret_cast<wcTemp*>(ge_alloc(static_cast<SIZE_T>(stringSize)
                                                          + sizeof(wcTemp)));
      memcpy(buffer, memory, static_cast<SIZE_T>(stringSize));

      uint32 numChars = stringSize / sizeof(wcTemp);
      buffer[numChars] = L'\0';

      data = WString(buffer);

      ge_free(buffer);

      return size;
    }

    static uint32
    getDynamicSize(const WString& data) {
      uint64 dataSize = data.size() * sizeof(WString::value_type) + sizeof(uint32);

#if GE_DEBUG_MODE
      if (dataSize > NumLimit::MAX_UINT32) {
        __string_throwDataOverflowException();
      }
#endif

      return static_cast<uint32>(dataSize);
    }
  };
}

namespace std {
  /**
   * @brief Hash value generator for SString.
   */
  template<>
  struct hash<geEngineSDK::String>
  {
    size_t operator()(const geEngineSDK::String& string) const {
      size_t hash = 0;
      for (char i : string) {
        hash = 65599 * hash + i;
      }

      return hash ^ (hash >> 16);
    }
  };

  /**
   * @brief Hash value generator for WString.
   */
  template<>
  struct hash<geEngineSDK::WString>
  {
    size_t operator()(const geEngineSDK::WString& string) const {
      size_t hash = 0;
      for (unsigned short i : string) {
        hash = 65599 * hash + i;
      }
      return hash ^ (hash >> 16);
    }
  };
}