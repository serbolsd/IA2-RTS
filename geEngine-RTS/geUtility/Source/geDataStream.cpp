/*****************************************************************************/
/**
 * @file    geDataStream.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2016/09/19
 * @brief   General purpose class used for encapsulating the reading and
 *          writing of data
 *
 * General purpose class used for encapsulating the reading and writing of data
 * from and to various sources using a common interface.
 *
 * @bug -std::consume_header seems to be ignored so I manually remove the
 *      header.
 *
 *      -Regardless of not providing the std::little_endian flag it
 *      seems that is how the data is read so I manually flip it.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "geDataStream.h"
#include "geDebug.h"
#include "geUnicode.h"

namespace geEngineSDK {
  using std::stringstream;

  const uint32 DataStream::streamTempSize = 128;

  /**
   * @brief	Checks does the provided buffer has an UTF32 byte order mark in
   *        little endian order.
   */
  bool
  isUTF32LE(const char* buffer) {
    return (0xFF == static_cast<uint8>(buffer[0]) &&
            0xFE == static_cast<uint8>(buffer[1]) &&
            0x00 == static_cast<uint8>(buffer[2]) &&
            0x00 == static_cast<uint8>(buffer[3]));
  }

  /**
   * @brief Checks does the provided buffer has an UTF32 byte order mark in
   *        big endian order.
   */
  bool
  isUTF32BE(const char* buffer) {
    return (0x00 == static_cast<uint8>(buffer[0]) &&
            0x00 == static_cast<uint8>(buffer[1]) &&
            0xFE == static_cast<uint8>(buffer[2]) &&
            0xFF == static_cast<uint8>(buffer[3]));
  }

  /**
   * @brief Checks does the provided buffer has an UTF16 byte order mark in
   *        little endian order.
   */
  bool
  isUTF16LE(const char* buffer) {
    return (0xFF == static_cast<uint8>(buffer[0]) &&
            0xFE == static_cast<uint8>(buffer[1]));
  }

  /**
   * @brief Checks does the provided buffer has an UTF16 byte order mark in
   *        big endian order.
   */
  bool
  isUTF16BE(const char* buffer) {
    return (0xFE == static_cast<uint8>(buffer[0]) &&
            0xFF == static_cast<uint8>(buffer[1]));
  }

  /**
   * @brief Checks does the provided buffer has an UTF8 byte order mark.
   */
  bool
  isUTF8(const char* buffer) {
    return (0xEF == static_cast<uint8>(buffer[0]) &&
            0xBB == static_cast<uint8>(buffer[1]) &&
            0xBF == static_cast<uint8>(buffer[2]));
  }

  void
  DataStream::writeString(const String& string, STRING_ENCODING::E encoding) {
    if (STRING_ENCODING::kUTF16 == encoding) {
      //Write BOM
      uint8 bom[2] = { 0xFF, 0xFE };
      write(bom, sizeof(bom));

      U16String u16string = UTF8::toUTF16(string);
      write(u16string.data(), u16string.length() * sizeof(char16_t));
    }
    else {
      // Write BOM
      uint8 bom[3] = { 0xEF, 0xBB, 0xBF };
      write(bom, sizeof(bom));
      write(string.data(), string.length());
    }
  }

  void
  DataStream::writeString(const WString& string, STRING_ENCODING::E encoding) {
    if (STRING_ENCODING::kUTF16 == encoding) {
      //Write BOM
      uint8 bom[2] = { 0xFF, 0xFE };
      write(bom, sizeof(bom));

      String u8string = UTF8::fromWide(string);
      U16String u16string = UTF8::toUTF16(u8string);
      write(u16string.data(), u16string.length() * sizeof(char16_t));
    }
    else {
      // Write BOM
      uint8 bom[3] = { 0xEF, 0xBB, 0xBF };
      write(bom, sizeof(bom));

      String u8string = UTF8::fromWide(string);
      write(u8string.data(), u8string.length());
    }
  }

  String
  DataStream::getAsString() {
    //Ensure read from begin of stream
    seek(0);

    //Try reading header
    uint8 headerBytes[4];
    SIZE_T numHeaderBytes = read(headerBytes, 4);

    SIZE_T dataOffset = 0;
    if (4 <= numHeaderBytes) {
      if (isUTF32LE(reinterpret_cast<char*>(headerBytes))) {
        dataOffset = 4;
      }
      else if (isUTF32BE(reinterpret_cast<char*>(headerBytes))) {
        LOGWRN("UTF-32 big endian decoding not supported");
        return u8"";
      }
    }

    if (0 == dataOffset && 3 <= numHeaderBytes) {
      if (isUTF8(reinterpret_cast<char*>(headerBytes))) {
        dataOffset = 3;
      }
    }

    if (0 == dataOffset && 2 <= numHeaderBytes) {
      if (isUTF16LE(reinterpret_cast<char*>(headerBytes))) {
        dataOffset = 2;
      }
      else if (isUTF16BE(reinterpret_cast<char*>(headerBytes))) {
        LOGWRN("UTF-16 big endian decoding not supported");
        return u8"";
      }
    }

    seek(dataOffset);

    //Read the entire buffer - ideally in one read, but if the size of the
    //buffer is unknown, do multiple fixed size reads.
    SIZE_T bufSize = (m_size > 0 ? m_size : 4096);
    
    //TODO: Change this to use the stack allocator, however right now we
    //haven't initialized the stack yet on the engine
    stringstream::char_type* tempBuffer =
      static_cast<stringstream::char_type*>(ge_alloc(bufSize));

    stringstream result;
    while (!isEOF()) {
      SIZE_T numReadBytes = read(tempBuffer, bufSize);
      result.write(tempBuffer, numReadBytes);
    }

    //TODO: Change this to use the stack allocator, however right now we
    //haven't initialized the stack yet on the engine
    free(tempBuffer);

    std::string string = result.str();

    switch (dataOffset)
    {
      default:
      case 0: //No BOM = assumed UTF-8
      case 3: //UTF-8
        return String(string.data(), string.length());
      case 2: //UTF-16
        {
          SIZE_T numElems = string.length() / 2;
          return UTF8::fromUTF16(U16String(reinterpret_cast<char16_t*>(
                                            const_cast<char*>(string.data())),
                                           numElems));
        }
      case 4: //UTF-32
        {
          SIZE_T numElems = string.length() / 4;
          return UTF8::fromUTF32(U32String(reinterpret_cast<char32_t*>(
                                            const_cast<char*>(string.data())),
                                           numElems));
        }
    }
  }

  WString
  DataStream::getAsWString() {
    String u8string = getAsString();
    return UTF8::toWide(u8string);
  }

  MemoryDataStream::MemoryDataStream(SIZE_T size)
    : DataStream(ACCESS_MODE::kREAD | ACCESS_MODE::kWRITE),
      m_data(nullptr),
      m_freeOnClose(true) {
    m_data = m_pos = reinterpret_cast<uint8*>(ge_alloc(size));
    m_size = size;
    m_end = m_data + m_size;

    GE_ASSERT(m_end >= m_pos);
  }

  MemoryDataStream::MemoryDataStream(void* memory, SIZE_T inSize, bool freeOnClose)
    : DataStream(ACCESS_MODE::kREAD | ACCESS_MODE::kWRITE),
      m_data(nullptr),
      m_freeOnClose(freeOnClose) {
    m_data = m_pos = static_cast<uint8*>(memory);
    m_size = inSize;
    m_end = m_data + m_size;

    GE_ASSERT(m_end >= m_pos);
  }

  MemoryDataStream::MemoryDataStream(DataStream& sourceStream)
    : DataStream(ACCESS_MODE::kREAD | ACCESS_MODE::kWRITE),
      m_data(nullptr) {
    //Copy data from incoming stream
    m_size = sourceStream.size();

    m_data = reinterpret_cast<uint8*>(ge_alloc(m_size));
    m_pos = m_data;
    m_end = m_data + sourceStream.read(m_data, m_size);
    m_freeOnClose = true;

    GE_ASSERT(m_end >= m_pos);
  }

  MemoryDataStream::MemoryDataStream(const SPtr<DataStream>& sourceStream)
    : DataStream(ACCESS_MODE::kREAD | ACCESS_MODE::kWRITE),
      m_data(nullptr) {
    //Copy data from incoming stream
    m_size = sourceStream->size();

    m_data = reinterpret_cast<uint8*>(ge_alloc(m_size));
    m_pos = m_data;
    m_end = m_data + sourceStream->read(m_data, m_size);
    m_freeOnClose = true;

    GE_ASSERT(m_end >= m_pos);
  }

  MemoryDataStream::~MemoryDataStream() {
    close();
  }

  SIZE_T
  MemoryDataStream::read(void* buf, SIZE_T count) {
    SIZE_T cnt = count;

    if (m_pos + cnt > m_end) {
      cnt = m_end - m_pos;
    }
    if (0 == cnt) {
      return 0;
    }

    GE_ASSERT(cnt <= count);

    memcpy(buf, m_pos, cnt);
    m_pos += cnt;

    return cnt;
  }

  SIZE_T
  MemoryDataStream::write(const void* buf, SIZE_T count) {
    SIZE_T written = 0;
    if (isWriteable()) {
      written = count;

      if (m_pos + written > m_end) {
        written = m_end - m_pos;
      }
      if (0 == written) {
        return 0;
      }

      memcpy(m_pos, buf, written);
      m_pos += written;
    }

    return written;
  }

  void
  MemoryDataStream::skip(SIZE_T count) {
    SIZE_T newpos = static_cast<SIZE_T>((m_pos - m_data) + count);
    GE_ASSERT(m_data + newpos <= m_end);
    m_pos = m_data + newpos;
  }

  void
  MemoryDataStream::seek(SIZE_T pos) {
    GE_ASSERT(m_data + pos <= m_end);
    m_pos = m_data + pos;
  }

  SIZE_T
  MemoryDataStream::tell() const {
    return m_pos - m_data;
  }

  bool
  MemoryDataStream::isEOF() const {
    return m_pos >= m_end;
  }

  SPtr<DataStream>
  MemoryDataStream::clone(bool copyData) const {
    if (!copyData) {
      return ge_shared_ptr_new<MemoryDataStream>(m_data, m_size, false);
    }
    return ge_shared_ptr_new<MemoryDataStream>(*this);
  }

  void
  MemoryDataStream::close() {
    if (nullptr != m_data) {
      if (m_freeOnClose) {
        ge_free(m_data);
      }
      m_data = nullptr;
    }
  }

  FileDataStream::FileDataStream(const Path& filePath,
                                 ACCESS_MODE::E accessMode,
                                 bool freeOnClose)
    : DataStream(static_cast<uint16>(accessMode)),
      m_path(filePath),
      m_freeOnClose(freeOnClose) {
    //Always open in binary mode. Also, always include reading
    std::ios::openmode mode = std::ios::binary;

    if ((accessMode & ACCESS_MODE::kREAD) != 0) {
      mode |= std::ios::in;
    }

    if (((accessMode & ACCESS_MODE::kWRITE) != 0)) {
      mode |= std::ios::out;
      m_pFStream = ge_shared_ptr_new<std::fstream>();
      m_pFStream->open(filePath.toPlatformString().c_str(), mode);
      m_pInStream = m_pFStream;
    }
    else {
      m_pFStreamRO = ge_shared_ptr_new<std::ifstream>();
      m_pFStreamRO->open(filePath.toPlatformString().c_str(), mode);
      m_pInStream = m_pFStreamRO;
    }

    //Should check ensure open succeeded, in case fail for some reason.
    if (m_pInStream->fail()) {
      LOGWRN("Cannot open file: " + filePath.toString());
      return;
    }

    m_pInStream->seekg(0, std::ios_base::end);
    m_size = static_cast<SIZE_T>(m_pInStream->tellg());
    m_pInStream->seekg(0, std::ios_base::beg);
  }

  FileDataStream::~FileDataStream() {
    close();
  }

  SIZE_T
  FileDataStream::read(void* buf, SIZE_T count) {
    m_pInStream->read(static_cast<char*>(buf), static_cast<std::streamsize>(count));
    return static_cast<SIZE_T>(m_pInStream->gcount());
  }

  SIZE_T
  FileDataStream::write(const void* buf, SIZE_T count) {
    SIZE_T written = 0;
    if (isWriteable() && m_pFStream) {
      m_pFStream->write(static_cast<const char*>(buf), static_cast<std::streamsize>(count));
      written = count;
    }
    return written;
  }

  void
  FileDataStream::skip(SIZE_T count) {
    m_pInStream->clear(); //Clear fail status in case eof was set
    m_pInStream->seekg(static_cast<std::ifstream::pos_type>(count), std::ios::cur);
  }

  void
  FileDataStream::seek(SIZE_T pos) {
    m_pInStream->clear();	//Clear fail status in case eof was set
    m_pInStream->seekg(static_cast<std::streamoff>(pos), std::ios::beg);
  }

  SIZE_T
  FileDataStream::tell() const {
    m_pInStream->clear(); //Clear fail status in case eof was set
    return static_cast<SIZE_T>(m_pInStream->tellg());
  }

  bool
  FileDataStream::isEOF() const {
    return m_pInStream->eof();
  }

  SPtr<DataStream>
  FileDataStream::clone(bool copyData) const {
    GE_UNREFERENCED_PARAMETER(copyData);
    return ge_shared_ptr_new<FileDataStream>(m_path,
                                             static_cast<ACCESS_MODE::E>(getAccessMode()),
                                             true);
  }

  void
  FileDataStream::close() {
    if (m_pInStream) {
      if (m_pFStreamRO) {
        m_pFStreamRO->close();
      }

      if (m_pFStream) {
        m_pFStream->flush();
        m_pFStream->close();
      }

      if (m_freeOnClose) {
        m_pInStream = nullptr;
        m_pFStreamRO = nullptr;
        m_pFStream = nullptr;
      }
    }
  }
}
