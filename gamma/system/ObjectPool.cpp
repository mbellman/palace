#include "system/assert.h"
#include "system/camera.h"
#include "system/entities.h"
#include "system/ObjectPool.h"

#define UNUSED_OBJECT_INDEX 0xffff

namespace Gamma {
  /**
   * ObjectPool
   * ----------
   */
  Object& ObjectPool::operator[](uint32 index) {
    return objects[index];
  }

  Object* ObjectPool::begin() const {
    return objects;
  }

  Object& ObjectPool::createObject() {
    uint16 id = runningId++;

    assert(max() > totalActive(), "Object Pool out of space: " + std::to_string(max()) + " objects allowed in this pool");
    // @todo cycle through indices until an unoccupied slot is found
    assert(indices[id] == UNUSED_OBJECT_INDEX, "Attempted to create an Object in an occupied slot");

    // Retrieve and initialize object
    uint16 index = totalActiveObjects;
    Object& object = objects[index];

    object._record.id = id;
    object._record.generation++;

    // Reset object matrix/color
    matrices[index] = Matrix4f::identity();
    colors[index] = pVec4(255, 255, 255);

    // Enable object lookup by ID -> index
    indices[id] = index;

    totalActiveObjects++;
    totalVisibleObjects++;

    return object;
  }

  Object* ObjectPool::end() const {
    return &objects[totalActiveObjects];
  }

  void ObjectPool::free() {
    for (uint16 i = 0; i < USHRT_MAX; i++) {
      indices[i] = UNUSED_OBJECT_INDEX;
    }

    if (objects != nullptr) {
      delete[] objects;
    }

    if (matrices != nullptr) {
      delete[] matrices;
    }

    if (colors != nullptr) {
      delete[] colors;
    }

    objects = nullptr;
    matrices = nullptr;
    colors = nullptr;
  }

  Object* ObjectPool::getById(uint16 objectId) const {
    uint16 index = indices[objectId];

    return index == UNUSED_OBJECT_INDEX ? nullptr : &objects[index];
  }

  Object* ObjectPool::getByRecord(const ObjectRecord& record) const {
    auto* object = getById(record.id);

    if (object == nullptr || object->_record.generation != record.generation) {
      return nullptr;
    }

    return object;
  }

  pVec4* ObjectPool::getColors() const {
    return colors;
  }

  Matrix4f* ObjectPool::getMatrices() const {
    return matrices;
  }

  uint16 ObjectPool::max() const {
    return maxObjects;
  }

  // @todo consolidate logic in partitionByDistance/partitionByVisibility
  uint16 ObjectPool::partitionByDistance(uint16 start, float distance, const Vec3f& cameraPosition) {
    uint16 current = start;
    uint16 end = totalVisible();

    while (end > current) {
      float currentObjectDistance = (objects[current].position - cameraPosition).magnitude();

      if (currentObjectDistance <= distance) {
        current++;
      } else {
        float endObjectDistance;

        do {
          endObjectDistance = (objects[--end].position - cameraPosition).magnitude();
        } while (endObjectDistance > distance && end > current);

        if (current != end) {
          swapObjects(current, end);
        }

        // @optimize we can increment 'current' here if endObjectDistance < distance
        // to avoid recalculating the same distance on the next while loop cycle
        // @todo test this out and make sure it works
      }
    }

    return current;
  }

  // @todo consolidate logic in partitionByDistance/partitionByVisibility
  // @todo accept a distance threshold to avoid culling partially
  // in-frame/partially out-of-frame objects
  // @todo use camera FoV to determine dot product threshold
  void ObjectPool::partitionByVisibility(const Camera& camera) {
    uint16 current = 0;
    uint16 end = totalActive();
    Vec3f cameraDirection = camera.orientation.getDirection();

    while (end > current) {
      Vec3f objectUnitViewPosition = (objects[current].position - camera.position).unit();

      if (Vec3f::dot(cameraDirection, objectUnitViewPosition) >= 0.7f) {
        current++;
      } else {
        Vec3f endObjectUnitViewPosition;

        do {
          endObjectUnitViewPosition = (objects[--end].position - camera.position).unit();
        } while (Vec3f::dot(cameraDirection, endObjectUnitViewPosition) < 0.7f && end > current);

        if (current != end) {
          swapObjects(current, end);
        }
      }
    }

    totalVisibleObjects = current;
  }

  void ObjectPool::removeById(uint16 objectId) {
    uint16 index = indices[objectId];

    if (index == UNUSED_OBJECT_INDEX) {
      return;
    }

    totalActiveObjects--;
    totalVisibleObjects--;

    uint16 lastIndex = totalActiveObjects;

    // Move last object/matrix/color into removed index
    objects[index] = objects[lastIndex];
    matrices[index] = matrices[lastIndex];
    colors[index] = colors[lastIndex];

    // Update ID -> index lookup table
    indices[objects[index]._record.id] = index;
    indices[objectId] = UNUSED_OBJECT_INDEX;
  }

  void ObjectPool::reserve(uint16 size) {
    free();

    maxObjects = size;
    totalActiveObjects = 0;
    totalVisibleObjects = 0;
    objects = new Object[size];
    matrices = new Matrix4f[size];
    colors = new pVec4[size];
  }

  void ObjectPool::swapObjects(uint16 indexA, uint16 indexB) {
    Object objectA = objects[indexA];
    Matrix4f matrixA = matrices[indexA];
    pVec4 colorA = colors[indexA];

    objects[indexA] = objects[indexB];
    matrices[indexA] = matrices[indexB];
    colors[indexA] = colors[indexB];

    objects[indexB] = objectA;
    matrices[indexB] = matrixA;
    colors[indexB] = colorA;

    indices[objects[indexA]._record.id] = indexA;
    indices[objects[indexB]._record.id] = indexB;
  }

  void ObjectPool::setColorById(uint16 objectId, const pVec4& color) {
    colors[indices[objectId]] = color;
  }

  uint16 ObjectPool::totalActive() const {
    return totalActiveObjects;
  }

  uint16 ObjectPool::totalVisible() const {
    return totalVisibleObjects;
  }

  void ObjectPool::transformById(uint16 objectId, const Matrix4f& matrix) {
    matrices[indices[objectId]] = matrix;
  }
}