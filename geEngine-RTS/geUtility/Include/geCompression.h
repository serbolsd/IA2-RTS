/*****************************************************************************/
/**
 * @file    geCompression.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/10/15
 * @brief   Performs generic compression and decompression on raw data
 *
 * Performs generic compression and decompression on raw data
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

namespace geEngineSDK {
  class GE_UTILITY_EXPORT Compression
  {
   public:
    /**
     * @brief Compresses the data from the provided data stream and outputs the
     *        new stream with compressed data.
     */
    static SPtr<MemoryDataStream>
    compress(SPtr<DataStream>& input);

    /**
     * @brief Decompresses the data from the provided data stream and outputs
     *        the new stream with decompressed data.
     */
    static SPtr<MemoryDataStream>
    decompress(SPtr<DataStream>& input);
  };
}
