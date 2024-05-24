// Copyright 2024 The Amber Authors.
// Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_ACCELERATION_STRUCTURE_H_
#define SRC_ACCELERATION_STRUCTURE_H_

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "amber/amber.h"
#include "amber/result.h"
#include "amber/value.h"
#include "src/format.h"
#include "src/image.h"

namespace amber {

enum class GeometryType : int8_t {
  kUnknown = 0,
  kTriangle,
  kAABB,
};

class Shader;

class Geometry {
 public:
  Geometry();
  ~Geometry();

  void SetType(GeometryType type) { type_ = type; }
  GeometryType GetType() { return type_; }

  void SetData(std::vector<float>& data) { data_.swap(data); }
  std::vector<float>& GetData() { return data_; }

  void SetFlags(uint32_t flags) { flags_ = flags; }
  uint32_t GetFlags() { return flags_; }

  size_t getVertexCount() const {
    return data_.size() / 3;  // Three floats to define vertex
  }

  size_t getPrimitiveCount() const {
    return IsTriangle() ? (getVertexCount() / 3)  // 3 vertices per triangle
           : IsAABB()   ? (getVertexCount() / 2)  // 2 vertices per AABB
                        : 0;
  }

  bool IsTriangle() const { return type_ == GeometryType::kTriangle; }
  bool IsAABB() const { return type_ == GeometryType::kAABB; }

 private:
  GeometryType type_ = GeometryType::kUnknown;
  std::vector<float> data_;
  uint32_t flags_ = 0u;
};

class BLAS {
 public:
  BLAS();
  ~BLAS();

  void SetName(const std::string& name) { name_ = name; }
  std::string GetName() const { return name_; }

  void AddGeometry(std::unique_ptr<Geometry>* geometry) {
    geometry_.push_back(std::move(*geometry));
  }
  size_t GetGeometrySize() { return geometry_.size(); }
  std::vector<std::unique_ptr<Geometry>>& GetGeometries() { return geometry_; }

 private:
  std::string name_;
  std::vector<std::unique_ptr<Geometry>> geometry_;
};

class BLASInstance {
 public:
  BLASInstance()
      : used_blas_name_(),
        used_blas_(nullptr),
        transform_(0),
        instance_custom_index_(0),
        mask_(0xFF),
        instanceShaderBindingTableRecordOffset_(0),
        flags_(0) {}
  ~BLASInstance();

  void SetUsedBLAS(const std::string& name, BLAS* blas) {
    used_blas_name_ = name;
    used_blas_ = blas;
  }
  std::string GetUsedBLASName() const { return used_blas_name_; }
  BLAS* GetUsedBLAS() const { return used_blas_; }

  void SetTransform(const std::vector<float>& transform) {
    transform_ = transform;
  }
  const float* GetTransform() const { return transform_.data(); }

  void SetInstanceIndex(uint32_t instance_custom_index) {
    instance_custom_index_ = instance_custom_index;
    // Make sure argument was not cut off
    assert(instance_custom_index_ == instance_custom_index);
  }
  uint32_t GetInstanceIndex() const { return instance_custom_index_; }

  void SetMask(uint32_t mask) {
    mask_ = mask;
    // Make sure argument was not cut off
    assert(mask_ == mask);
  }
  uint32_t GetMask() const { return mask_; }

  void SetOffset(uint32_t offset) {
    instanceShaderBindingTableRecordOffset_ = offset;
    // Make sure argument was not cut off
    assert(instanceShaderBindingTableRecordOffset_ == offset);
  }
  uint32_t GetOffset() const { return instanceShaderBindingTableRecordOffset_; }

  void SetFlags(uint32_t flags) {
    flags_ = flags;
    // Make sure argument was not cut off
    assert(flags_ == flags);
  }
  uint32_t GetFlags() const { return flags_; }

 private:
  std::string used_blas_name_;
  BLAS* used_blas_;
  std::vector<float> transform_;
  uint32_t instance_custom_index_ : 24;
  uint32_t mask_ : 8;
  uint32_t instanceShaderBindingTableRecordOffset_ : 24;
  uint32_t flags_ : 8;
};

class TLAS {
 public:
  TLAS();
  ~TLAS();

  void SetName(const std::string& name) { name_ = name; }
  std::string GetName() const { return name_; }

  void AddInstance(std::unique_ptr<BLASInstance> instance) {
    blas_instances_.push_back(
        std::unique_ptr<BLASInstance>(instance.release()));
  }
  size_t GetInstanceSize() { return blas_instances_.size(); }
  std::vector<std::unique_ptr<BLASInstance>>& GetInstances() {
    return blas_instances_;
  }

 private:
  std::string name_;
  std::vector<std::unique_ptr<BLASInstance>> blas_instances_;
};

class ShaderGroup {
 public:
  ShaderGroup();
  ~ShaderGroup();

  void SetName(const std::string& name) { name_ = name; }
  std::string GetName() const { return name_; }

  void SetGeneralShader(Shader* shader) { generalShader_ = shader; }
  Shader* GetGeneralShader() const { return generalShader_; }

  void SetClosestHitShader(Shader* shader) { closestHitShader_ = shader; }
  Shader* GetClosestHitShader() const { return closestHitShader_; }

  void SetAnyHitShader(Shader* shader) { anyHitShader_ = shader; }
  Shader* GetAnyHitShader() const { return anyHitShader_; }

  void SetIntersectionShader(Shader* shader) { intersectionShader_ = shader; }
  Shader* GetIntersectionShader() const { return intersectionShader_; }

  bool IsGeneralGroup() const { return generalShader_ != nullptr; }
  bool IsHitGroup() const {
    return closestHitShader_ != nullptr || anyHitShader_ != nullptr ||
           intersectionShader_ != nullptr;
  }
  Shader* GetShaderByType(ShaderType type) const {
    switch (type) {
      case kShaderTypeRayGeneration:
      case kShaderTypeMiss:
      case kShaderTypeCall:
        return generalShader_;
      case kShaderTypeAnyHit:
        return anyHitShader_;
      case kShaderTypeClosestHit:
        return closestHitShader_;
      case kShaderTypeIntersection:
        return intersectionShader_;
      default:
        assert(0 && "Unsupported shader type");
        return nullptr;
    }
  }

 private:
  std::string name_;
  Shader* generalShader_;
  Shader* closestHitShader_;
  Shader* anyHitShader_;
  Shader* intersectionShader_;
};

class SBTRecord {
 public:
  SBTRecord();
  ~SBTRecord();

  void SetUsedShaderGroupName(const std::string& shader_group_name) {
    used_shader_group_name_ = shader_group_name;
  }
  std::string GetUsedShaderGroupName() const { return used_shader_group_name_; }

  void SetCount(const uint32_t count) { count_ = count; }
  uint32_t GetCount() const { return count_; }

  void SetIndex(const uint32_t index) { index_ = index; }
  uint32_t GetIndex() const { return index_; }

 private:
  std::string used_shader_group_name_;
  uint32_t count_ = 1;
  uint32_t index_ = static_cast<uint32_t>(-1);
};

class SBT {
 public:
  SBT();
  ~SBT();

  void SetName(const std::string& name) { name_ = name; }
  std::string GetName() const { return name_; }

  void AddSBTRecord(std::unique_ptr<SBTRecord> record) {
    records_.push_back(std::move(record));
  }
  size_t GetSBTRecordCount() { return records_.size(); }
  std::vector<std::unique_ptr<SBTRecord>>& GetSBTRecords() { return records_; }
  uint32_t GetSBTSize() {
    uint32_t size = 0;
    for (auto& x : records_)
      size += x->GetCount();

    return size;
  }

 private:
  std::string name_;
  std::vector<std::unique_ptr<SBTRecord>> records_;
};

}  // namespace amber

#endif  // SRC_ACCELERATION_STRUCTURE_H_
