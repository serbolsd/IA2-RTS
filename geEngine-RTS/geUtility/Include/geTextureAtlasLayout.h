/*****************************************************************************/
/**
 * @file    geTextureAtlasLayout.h
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/10/31
 * @brief   Class that organizes a set of textures into a texture atlas.
 *
 * Organizes a set of textures into a single larger texture (an atlas) by
 * minimizing empty space.
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
#include "geVector2.h"
#include "geNumericLimits.h"

namespace geEngineSDK {
  /**
   * @brief Organizes a set of textures into a single larger texture (an atlas)
   *        by minimizing empty space.
   */
  class GE_UTILITY_EXPORT TextureAtlasLayout
  {
    /**
     * @brief Represent a single node in the texture atlas binary tree.
     */
    struct TexAtlasNode
    {
      constexpr TexAtlasNode() = default;
      
      constexpr TexAtlasNode(uint32 x, uint32 y, uint32 width, uint32 height)
        : x(x),
          y(y),
          width(width),
          height(height)
      {}

      uint32 x = 0;
      uint32 y = 0;
      uint32 width = 0;
      uint32 height = 0;
      uint32 children[2] = { NumLimit::MAX_UINT32, NumLimit::MAX_UINT32 };
      bool nodeFull = false;
    };

   public:
    TextureAtlasLayout() = default;

    /**
     * @brief Constructs a new texture atlas layout with the provided parameters.
     * @param[in] width     Initial width of the atlas texture.
     * @param[in] height    Initial height of the atlas texture.
     * @param[in] maxWidth  Maximum width the atlas texture is allowed to grow to,
     *                      when elements don't fit.
     * @param[in] maxHeight Maximum height the atlas texture is allowed to grow to,
     *                      when elements don't fit.
     * @param[in] pow2 When true the resulting atlas size will always be a power of two.
     */
    TextureAtlasLayout(uint32 width,
                       uint32 height,
                       uint32 maxWidth,
                       uint32 maxHeight,
                       bool pow2 = false)
      : m_initialWidth(width),
        m_initialHeight(height),
        m_width(width),
        m_height(height),
        m_maxWidth(maxWidth),
        m_maxHeight(maxHeight),
        m_pow2(pow2) {
      m_nodes.emplace_back(0, 0, maxWidth, maxHeight);
    }

    /**
     * @brief Attempts to add a new element in the layout. Elements should be
     *        added to the atlas from largest to smallest, otherwise a
     *        non-optimal layout is likely to be generated.
     * @param[in] width	Width of the new element, in pixels.
     * @param[in] height	Height of the new element, in pixels.
     * @param[out] x  Horizontal position of the new element within the atlas.
     *                Only valid if method returns true.
     * @param[out] y  Vertical position of the new element within the atlas.
     *                Only valid if method returns true.
     * @return true if the element was added to the atlas,
     *         false if the element doesn't fit.
     */
    bool
    addElement(uint32 width, uint32 height, uint32& x, uint32& y);

    /**
     * @brief Removes all entries from the layout.
     */
    void
    clear();

    /**
     * @brief Checks have any elements been added to the layout.
     */
    bool
    isEmpty() const {
      return 1 == m_nodes.size();
    }

    /**
     * @brief Returns the width of the atlas texture, in pixels.
     */
    uint32
    getWidth() const {
      return m_width;
    }

    /**
     * @brief Returns the height of the atlas texture, in pixels.
     */
    uint32
    getHeight() const {
      return m_height;
    }

   private:
    /**
     * @brief Attempts to add a new element to the specified layout node.
     * @param[in] nodeIdx Index of the node to which to add the element.
     * @param[in] width   Width of the new element, in pixels.
     * @param[in] height  Height of the new element, in pixels.
     * @param[out] x Horizontal position of the new element within the atlas.
     *             Only valid if method returns true.
     * @param[out] y Vertical position of the new element within the atlas.
     *             Only valid if method returns true.
     * @param[in] allowGrowth When true, the width/height of the atlas will be
     *            allowed to grow to fit the element.
     * @return true if the element was added to the atlas,
     *         false if the element doesn't fit.
     */
    bool
    addToNode(uint32 nodeIdx,
              uint32 width,
              uint32 height,
              uint32& x,
              uint32& y,
              bool allowGrowth);

    uint32 m_initialWidth = 0;
    uint32 m_initialHeight = 0;
    uint32 m_width = 0;
    uint32 m_height = 0;
    uint32 m_maxWidth = 0;
    uint32 m_maxHeight = 0;
    bool m_pow2 = false;

    Vector<TexAtlasNode> m_nodes;
  };

  /**
   * @brief Utility class used for texture atlas layouts.
   */
  class GE_UTILITY_EXPORT TextureAtlasUtility
  {
   public:
    /**
     * @brief Represents a single element used as in input to TextureAtlasUtility.
     *        Usually represents a single texture.
     * @note  input is required to be filled in before passing it to TextureAtlasUtility.
     * @note  output will be filled after a call to TextureAtlasUtility::createAtlasLayout().
     */
    struct Element
    {
      struct
      {
        uint32 width;
        uint32 height;
      } input;

      struct
      {
        uint32 x;
        uint32 y;
        uint32 idx;
        int32 page;
      } output;
    };

    /**
     * @brief Describes a single page of the texture atlas.
     */
    struct Page
    {
      uint32 width;
      uint32 height;
    };

    /**
     * @brief Creates an optimal texture layout by packing texture elements in
     *        order to end up with as little empty space as possible. Algorithm
     *        will split elements over multiple textures if they don't fit in a
     *        single texture.
     * @param[in] elements  Elements to process. They need to have their input
     *            structures filled in, and this method will fill output when
     *            it returns.
     * @param[in] width     Initial width of the atlas texture.
     * @param[in] height    Initial height of the atlas texture.
     * @param[in] maxWidth  Maximum width the atlas texture is allowed to grow
     *                      to, when elements don't fit.
     * @param[in] maxHeight Maximum height the atlas texture is allowed to grow
     *                      to, when elements don't fit.
     * @param[in] pow2      When true the resulting atlas size will always be a
     *                      power of two.
     * @return One or more descriptors that determine the size of the final
     *         atlas textures. Texture elements will reference these pages with
     *         their output.page parameter.
     */
    static Vector<Page>
    createAtlasLayout(Vector<Element>& elements,
                      uint32 width,
                      uint32 height,
                      uint32 maxWidth,
                      uint32 maxHeight,
                      bool pow2 = false);
  };
}
