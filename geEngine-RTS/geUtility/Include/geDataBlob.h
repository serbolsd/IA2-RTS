/*****************************************************************************/
/**
 * @file    geDataBlob.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2018/04/24
 * @brief   Serializable blob of raw memory.
 *
 * Serializable blob of raw memory.
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
#include "geException.h"

namespace geEngineSDK {
  struct DataBlob
  {
    uint8* data = nullptr;
    uint32 size = 0;
  };

  template<>
  struct RTTIPlainType<DataBlob>
  {
    enum { kID = TYPEID_UTILITY::kID_DataBlob }; enum { kHasDynamicSize = 1 };

    static void
    toMemory(const DataBlob& data, char* memory) {
      uint32 size = getDynamicSize(data);
      memcpy(memory, &size, sizeof(uint32));
      memory += sizeof(uint32);
      memcpy(memory, data.data, data.size);
    }

    static uint32
    fromMemory(DataBlob& data, char* memory) {
      uint32 size;
      memcpy(&size, memory, sizeof(uint32));
      memory += sizeof(uint32);

      if (nullptr != data.data) {
        ge_free(data.data);
      }

      data.size = size - sizeof(uint32);
      data.data = reinterpret_cast<uint8*>(ge_alloc(data.size));
      memcpy(data.data, memory, data.size);

      return size;
    }

    static uint32
    getDynamicSize(const DataBlob& data) {
      uint64 dataSize = data.size + sizeof(uint32);

#if GE_DEBUG_MODE
      if (dataSize > NumLimit::MAX_UINT32) {
        GE_EXCEPT(InternalErrorException,
                  "Data overflow! Size doesn't fit into 32 bits.");
      }
#endif
      return static_cast<uint32>(dataSize);
    }
  };
}
