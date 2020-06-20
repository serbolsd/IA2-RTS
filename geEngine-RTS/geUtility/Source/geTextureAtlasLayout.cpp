/*****************************************************************************/
/**
 * @file    geTextureAtlasLayout.cpp
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

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "geTextureAtlasLayout.h"
#include "geBitwise.h"
#include "geDebug.h"

namespace geEngineSDK {
  using std::sort;

  bool
  TextureAtlasLayout::addElement(uint32 width, uint32 height, uint32& x, uint32& y) {
    if (0 == width || 0 == height) {
      x = 0;
      y = 0;
      return true;
    }

    //Try adding without expanding, if that fails try to expand
    if (!addToNode(0, width, height, x, y, false)) {
      if (!addToNode(0, width, height, x, y, true))
        return false;
    }

    //Update size to cover all nodes
    if (m_pow2) {
      m_width = Math::max(m_width, Bitwise::nextPow2(x + width));
      m_height = Math::max(m_height, Bitwise::nextPow2(y + height));
    }
    else {
      m_width = Math::max(m_width, x + width);
      m_height = Math::max(m_height, y + height);
    }

    return true;
  }

  void
  TextureAtlasLayout::clear() {
    m_nodes.clear();
    m_nodes.emplace_back(0, 0, m_width, m_height);

    m_width = m_initialWidth;
    m_height = m_initialHeight;
  }

  bool
  TextureAtlasLayout::addToNode(uint32 nodeIdx,
                                uint32 width,
                                uint32 height,
                                uint32& x,
                                uint32& y,
                                bool allowGrowth) {
    TexAtlasNode* node = &m_nodes[nodeIdx];
    float aspect = static_cast<float>(node->width) / static_cast<float>(node->height);

    if (NumLimit::MAX_UINT32 != node->children[0]) {
      if (addToNode(node->children[0], width, height, x, y, allowGrowth)) {
        return true;
      }
      return addToNode(node->children[1], width, height, x, y, allowGrowth);
    }
    else {
      if (node->nodeFull) {
        return false;
      }

      if (width > node->width || height > node->height) {
        return false;
      }

      if (!allowGrowth) {
        if (node->x + width > m_width || node->y + height > m_height) {
          return false;
        }
      }

      if (width == node->width && height == node->height) {
        x = node->x;
        y = node->y;
        node->nodeFull = true;
        return true;
      }

      float dw = static_cast<float>(node->width - width);
      float dh = (node->height - height) * aspect;

      uint32 nextChildIdx = static_cast<uint32>(m_nodes.size());
      node->children[0] = nextChildIdx;
      node->children[1] = nextChildIdx + 1;

      TexAtlasNode nodeCopy = *node;
      node = nullptr; //Undefined past this point
      if (dw > dh) {
        m_nodes.emplace_back(nodeCopy.x, nodeCopy.y, width, nodeCopy.height);
        m_nodes.emplace_back(nodeCopy.x + width,
                             nodeCopy.y,
                             nodeCopy.width - width,
                             nodeCopy.height);
      }
      else {
        m_nodes.emplace_back(nodeCopy.x, nodeCopy.y, nodeCopy.width, height);
        m_nodes.emplace_back(nodeCopy.x,
                             nodeCopy.y + height,
                             nodeCopy.width,
                             nodeCopy.height - height);
      }

      return addToNode(nodeCopy.children[0], width, height, x, y, allowGrowth);
    }
  }

  Vector<TextureAtlasUtility::Page>
  TextureAtlasUtility::createAtlasLayout(Vector<Element>& elements,
                                         uint32 width,
                                         uint32 height,
                                         uint32 maxWidth,
                                         uint32 maxHeight,
                                         bool pow2) {
    for (SIZE_T i = 0; i < elements.size(); ++i) {
      // Preserve original index before sorting
      elements[i].output.idx = static_cast<uint32>(i);
      elements[i].output.page = -1;
    }

    sort(elements.begin(), elements.end(), [](const Element& a, const Element& b) {
      return a.input.width * a.input.height > b.input.width * b.input.height;
    });

    Vector<TextureAtlasLayout> layouts;
    SIZE_T remainingCount = elements.size();
    while (remainingCount > 0) {
      layouts.emplace_back(width, height, maxWidth, maxHeight, pow2);
      TextureAtlasLayout& curLayout = layouts.back();

      //Find largest unassigned element that fits
      uint32 sizeLimit = NumLimit::MAX_UINT32;
      while (true) {
        uint32 largestId = NumLimit::MAX_UINT32;

        // Assumes elements are sorted from largest to smallest
        for (SIZE_T i = 0; i < elements.size(); ++i) {
          if (-1 == elements[i].output.page) {
            uint32 size = elements[i].input.width * elements[i].input.height;
            if (NumLimit::MAX_UINT32 > size) {
              largestId = static_cast<uint32>(i);
              break;
            }
          }
        }

        if (NumLimit::MAX_UINT32 == largestId){
          break;  //Nothing fits, start a new page
        }

        Element& element = elements[largestId];

        //Check if an element is too large to ever fit
        if (element.input.width > maxWidth || element.input.height > maxHeight) {
          LOGWRN("Some of the provided elements don't fit in an atlas of provided size. "
                 "Returning empty array of pages.");
          return Vector<Page>();
        }

        if (curLayout.addElement(element.input.width,
                                 element.input.height,
                                 element.output.x,
                                 element.output.y)) {
          element.output.page = static_cast<uint32>(layouts.size() - 1);
          --remainingCount;
        }
        else {
          sizeLimit = element.input.width * element.input.height;
        }
      }
    }

    Vector<Page> pages;
    for (auto& layout : layouts) {
      pages.push_back( {layout.getWidth(), layout.getHeight()} );
    }

    return pages;
  }
}
