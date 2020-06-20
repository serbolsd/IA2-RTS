//#include <vld.h>
#include <DirectXMath.h>

#define GTEST_HAS_TR1_TUPLE 0
#define GTEST_USE_OWN_TR1_TUPLE 0
#include <gtest/gtest.h>

#include <gePrerequisitesUtil.h>
#include <geOctree.h>

using namespace geEngineSDK;

struct DebugOctreeElem
{
  AABox box;
  mutable OctreeElementId octreeId;
};

struct DebugOctreeData
{
  Vector<DebugOctreeElem> elements;
};

struct DebugOctreeOptions
{
  enum { loosePadding = 16 };
  enum { minElementsPerNode = 8 };
  enum { maxElementsPerNode = 16 };
  enum { maxDepth = 12 };

  static simd::AABox
  getBounds(uint32 elem, void* context) {
    DebugOctreeData* octreeData = reinterpret_cast<DebugOctreeData*>(context);
    return simd::AABox(octreeData->elements[elem].box);
  }

  static void
  setElementId(uint32 elem, const OctreeElementId& id, void* context) {
    DebugOctreeData* octreeData = reinterpret_cast<DebugOctreeData*>(context);
    octreeData->elements[elem].octreeId = id;
  }
};

typedef Octree<uint32, DebugOctreeOptions> DebugOctree;

TEST(geOctree, Construct_Octree) {
  DebugOctreeData octreeData;
  DebugOctree octree(Vector3::ZERO, 800.0f, &octreeData);

  struct SizeAndCount
  {
    float sizeMin;
    float sizeMax;
    uint32 count;
  };

  SizeAndCount types[]
  {
    { 0.02f, 0.2f, 2000 },  //Very small objects
    { 0.2f, 1.0f, 2000 },   //Small objects
    { 1.0f, 5.0f, 5000 },   //Medium sized objects
    { 5.0f, 30.0f, 4000 },  //Large objects
    { 30.0f, 100.0f, 2000 } //Very large objects
  };

  float placementExtents = 750.0f;
  for (uint32 i = 0; i < sizeof(types) / sizeof(types[0]); ++i) {
    for (uint32 j = 0; j < types[i].count; ++j) {
      Vector3 position(((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents,
                       ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents,
                       ((rand() / (float)RAND_MAX) * 2.0f - 1.0f) * placementExtents);

      float realSize = types[i].sizeMax - types[i].sizeMin;
      Vector3 extents(types[i].sizeMin + ((rand() / (float)RAND_MAX)) * realSize * 0.5f,
                      types[i].sizeMin + ((rand() / (float)RAND_MAX)) * realSize * 0.5f,
                      types[i].sizeMin + ((rand() / (float)RAND_MAX)) * realSize * 0.5f);

      DebugOctreeElem elem;
      elem.box = AABox(position - extents, position + extents);

      uint32 elemIdx = static_cast<uint32>(octreeData.elements.size());
      octreeData.elements.push_back(elem);
      octree.addElement(elemIdx);
    }
  }

  DebugOctreeElem manualElems[3];
  manualElems[0].box = AABox(Vector3(100.0f, 100.0f, 100.f), Vector3(110.0f, 115.0f, 110.0f));
  manualElems[1].box = AABox(Vector3(200.0f, 100.0f, 100.f), Vector3(250.0f, 150.0f, 150.0f));
  manualElems[2].box = AABox(Vector3(90.0f, 90.0f, 90.f), Vector3(105.0f, 105.0f, 110.0f));

  for (uint32 i = 0; i < 3; ++i) {
    uint32 elemIdx = static_cast<uint32>(octreeData.elements.size());
    octreeData.elements.push_back(manualElems[i]);
    octree.addElement(elemIdx);
  }

  AABox queryBounds = manualElems[0].box;
  DebugOctree::BoxIntersectIterator interIter(octree, queryBounds);

  Vector<uint32> overlapElements;
  while (interIter.moveNext()) {
    uint32 element = interIter.getElement();
    overlapElements.push_back(element);

    //Manually check for intersections
    EXPECT_TRUE(octreeData.elements[element].box.intersect(queryBounds));
  }

  // Ensure that all we have found all possible overlaps by manually testing all elements
  uint32 elemIdx = 0;
  for (auto& entry : octreeData.elements) {
    if (entry.box.intersect(queryBounds)) {
      auto iterFind = std::find(overlapElements.begin(), overlapElements.end(), elemIdx);
      EXPECT_TRUE(iterFind != overlapElements.end());
    }
    elemIdx++;
  }

  //Ensure nothing goes wrong during element removal
  for (auto& entry : octreeData.elements) {
    octree.removeElement(entry.octreeId);
  }
}
