/*****************************************************************************/
/**
 * @file    gePoolAlloc.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2018/02/17
 * @brief   Pool allocator
 *
 * A memory allocator that allocates elements of the same size. Allows for
 * fairly quick allocations and deallocations.
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
#include <climits>

namespace geEngineSDK {
  /**
   * @brief A memory allocator that allocates elements of the same size.
   *        Allows for fairly quick allocations and deallocations.
   *
   * @tparam  ElemSize      Size of a single element in the pool. This will be
   *                        the exact allocation size. 4 byte minimum.
   * @tparam  ElemsPerBlock Determines how much space to reserve for elements.
   *                        This determines the initial size of the pool, and
   *                        the additional size the pool will be expanded by
   *                        every time the number of elements goes over the
   *                        available storage limit.
   * @tparam  Alignment     Memory alignment of each allocated element. Note
   *                        that alignments that are larger than element size,
   *                        or aren't a multiplier of element size will
   *                        introduce additionally padding for each element,
   *                        and therefore require more internal memory.
   */
  template<int ElemSize, int ElemsPerBlock=512, int Alignment=4>
  class PoolAlloc
  {
   private:
    /**
     * @brief A single block able to hold ElemsPerBlock elements.
     */
    class MemBlock
    {
     public:
      MemBlock(uint8* data)
        : m_data(data),
          m_freePtr(0),
          m_freeElems(ElemsPerBlock),
          m_nextBlock(nullptr)
      {
        SIZE_T offset = 0;
        for (uint32 i = 0; i < ElemsPerBlock; ++i) {
          SIZE_T* entryPtr = reinterpret_cast<SIZE_T*>(&data[offset]);

          offset += ActualElemSize;
          *entryPtr = offset;
        }
      }

      ~MemBlock() {
        GE_ASSERT(m_freeElems == ElemsPerBlock &&
                  "Not all elements were deallocated from a block.");
      }

      /**
       * @brief Returns the first free address and increments the free pointer.
       *        Caller needs to ensure the remaining block size is adequate
       *        before calling.
       */
      uint8*
      alloc() {
        uint8* freeEntry = &m_data[m_freePtr];
        m_freePtr = *reinterpret_cast<SIZE_T*>(freeEntry);
        --m_freeElems;
        return freeEntry;
      }

      /**
       * @brief Deallocates the provided pointer.
       */
      void
      dealloc(void* data) {
        SIZE_T* entryPtr = reinterpret_cast<SIZE_T*>(data);
        *entryPtr = m_freePtr;
        ++m_freeElems;

        m_freePtr = static_cast<SIZE_T>((reinterpret_cast<uint8*>(data)) - m_data);
      }

      uint8* m_data;
      SIZE_T m_freePtr;
      SIZE_T m_freeElems;
      MemBlock* m_nextBlock;
    };

   public:
    PoolAlloc() {
      static_assert(ElemSize >= 4,
                    "Pool allocator minimum allowed element size is 4 bytes.");
      static_assert(ElemsPerBlock > 0,
                    "Number of elements per block must be at least 1.");
      static_assert(ElemsPerBlock * ActualElemSize <= UINT_MAX,
                    "Pool allocator block size too large.");
    }

    ~PoolAlloc() {
      MemBlock* curBlock = m_freeBlock;
      while (nullptr != curBlock) {
        MemBlock* nextBlock = curBlock->m_nextBlock;
        deallocBlock(curBlock);
        curBlock = nextBlock;
      }
    }

    /**
     * @brief Allocates enough memory for a single element in the pool.
     */
    uint8*
    alloc() {
      if (nullptr == m_freeBlock || 0 == m_freeBlock->m_freeElems) {
        allocBlock();
      }

      m_totalNumElems++;
      return m_freeBlock->alloc();
    }

    /**
     * @brief Deallocates an element from the pool.
     */
    void
    free(void* data) {
      MemBlock* curBlock = m_freeBlock;
      while (curBlock) {
        constexpr SIZE_T blockDataSize = ActualElemSize * ElemsPerBlock;
        if (data >= curBlock->m_data &&
            data < (curBlock->m_data + blockDataSize)) {
          curBlock->dealloc(data);
          m_totalNumElems--;

          if (0 == curBlock->m_freeElems && curBlock->m_nextBlock) {
            //Free the block, but only if there is some extra free space in other blocks
            const SIZE_T totalSpace = (m_numBlocks - 1) * ElemsPerBlock;
            const SIZE_T freeSpace = totalSpace - m_totalNumElems;

            if (freeSpace > ElemsPerBlock / 2) {
              m_freeBlock = curBlock->m_nextBlock;
              deallocBlock(curBlock);
            }
          }

          return;
        }

        curBlock = curBlock->m_nextBlock;
      }

      GE_ASSERT(false);
    }

    /**
     * @brief Allocates and constructs a single pool element.
     */
    template<class T, class... Args>
    T*
    construct(Args &&...args) {
      T* data = reinterpret_cast<T*>(alloc());
      new ((void*)data) T(std::forward<Args>(args)...);
      return data;
    }

    /**
     * @brief Destructs and deallocates a single pool element.
     */
    template<class T>
    void
    destruct(T* data) {
      data->~T();
      free(data);
    }

   private:
    /**
     * @brief Allocates a new block of memory using a heap allocator.
     */
    MemBlock*
    allocBlock() {
      MemBlock* newBlock = nullptr;
      MemBlock* curBlock = m_freeBlock;

      while (nullptr != curBlock) {
        MemBlock* nextBlock = curBlock->m_nextBlock;
        if (nullptr != nextBlock && nextBlock->m_freeElems > 0) {
          //Found an existing block with free space
          newBlock = nextBlock;

          curBlock->m_nextBlock = newBlock->m_nextBlock;
          newBlock->m_nextBlock = m_freeBlock;

          break;
        }

        curBlock = nextBlock;
      }

      if (nullptr == newBlock) {
        constexpr SIZE_T blockDataSize = ActualElemSize * ElemsPerBlock;
        
        //Padding for potential alignment correction
        SIZE_T paddedBlockDataSize = blockDataSize + (Alignment - 1);

        uint8* data = reinterpret_cast<uint8*>(ge_alloc(sizeof(MemBlock) +
                                                        paddedBlockDataSize));

        void* blockData = data + sizeof(MemBlock);
        blockData = std::align(Alignment,
                               blockDataSize,
                               blockData,
                               paddedBlockDataSize);

        newBlock = new (data) MemBlock(reinterpret_cast<uint8*>(blockData));
        m_numBlocks++;

        newBlock->m_nextBlock = m_freeBlock;
      }

      m_freeBlock = newBlock;
      return newBlock;
    }

    /**
     * @brief Deallocates a block of memory.
     */
    void
    deallocBlock(MemBlock* block) {
      block->~MemBlock();
      ge_free(block);
      m_numBlocks--;
    }

    static constexpr SIZE_T
      ActualElemSize = ((ElemSize + Alignment - 1) / Alignment) * Alignment;

    MemBlock* m_freeBlock = nullptr;
    SIZE_T m_totalNumElems = 0;
    SIZE_T m_numBlocks = 0;
  };
}
