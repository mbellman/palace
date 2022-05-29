#pragma once

#include "math/matrix.h"
#include "system/packed_data.h"
#include "system/type_aliases.h"

namespace Gamma {
  struct Object;
  struct ObjectRecord;
  struct Camera;

  /**
   * ObjectPool
   * ----------
   *
   * A collection of Objects tied to a given Mesh, designed
   * to facilitate instanced/batched rendering.
   *
   * @todo uint16 -> uint32 to support >65K objects per pool
   */
  class ObjectPool {
  public:
    Object& operator[](uint32 index);

    Object* begin() const;
    Object& createObject();
    Object* end() const;
    void free();
    Object* getById(uint16 objectId) const;
    Object* getByRecord(const ObjectRecord& record) const;
    pVec4* getColors() const;
    Matrix4f* getMatrices() const;
    uint16 max() const;
    uint16 partitionByDistance(uint16 start, float distance, const Vec3f& cameraPosition);
    void partitionByVisibility(const Camera& camera);
    void removeById(uint16 objectId);
    void reserve(uint16 size);
    void setColorById(uint16 objectId, const pVec4& color);
    uint16 totalActive() const;
    uint16 totalVisible() const;
    void transformById(uint16 objectId, const Matrix4f& matrix);

  private:
    Object* objects = nullptr;
    Matrix4f* matrices = nullptr;
    pVec4* colors = nullptr;
    // @todo if we cycle through indices to find unoccupied slots
    // when creating new objects, we needn't preallocate the entire
    // indices array up to USHRT_MAX/0xffff
    uint16 indices[0xffff];
    uint16 maxObjects = 0;
    uint16 totalActiveObjects = 0;
    uint16 totalVisibleObjects = 0;
    uint16 runningId = 0;

    void swapObjects(uint16 indexA, uint16 indexB);
  };
}