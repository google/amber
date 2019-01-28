// Copyright 2018 The Amber Authors.
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

#ifndef SRC_COMMAND_H_
#define SRC_COMMAND_H_

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "amber/shader_info.h"
#include "amber/value.h"
#include "src/command_data.h"
#include "src/datum_type.h"
#include "src/pipeline_data.h"

namespace amber {

class ClearCommand;
class ClearColorCommand;
class ClearDepthCommand;
class ClearStencilCommand;
class ComputeCommand;
class DrawArraysCommand;
class DrawRectCommand;
class EntryPointCommand;
class PatchParameterVerticesCommand;
class ProbeCommand;
class ProbeSSBOCommand;
class BufferCommand;

class Command {
 public:
  enum class Type : uint8_t {
    kClear = 0,
    kClearColor,
    kClearDepth,
    kClearStencil,
    kCompute,
    kDrawArrays,
    kDrawRect,
    kEntryPoint,
    kPatchParameterVertices,
    kPipelineProperties,
    kProbe,
    kProbeSSBO,
    kBuffer
  };

  virtual ~Command();

  bool IsDrawRect() const { return command_type_ == Type::kDrawRect; }
  bool IsDrawArrays() const { return command_type_ == Type::kDrawArrays; }
  bool IsCompute() const { return command_type_ == Type::kCompute; }
  bool IsProbe() const { return command_type_ == Type::kProbe; }
  bool IsProbeSSBO() const { return command_type_ == Type::kProbeSSBO; }
  bool IsBuffer() const { return command_type_ == Type::kBuffer; }
  bool IsClear() const { return command_type_ == Type::kClear; }
  bool IsClearColor() const { return command_type_ == Type::kClearColor; }
  bool IsClearDepth() const { return command_type_ == Type::kClearDepth; }
  bool IsClearStencil() const { return command_type_ == Type::kClearStencil; }
  bool IsPatchParameterVertices() const {
    return command_type_ == Type::kPatchParameterVertices;
  }
  bool IsEntryPoint() const { return command_type_ == Type::kEntryPoint; }

  ClearCommand* AsClear();
  ClearColorCommand* AsClearColor();
  ClearDepthCommand* AsClearDepth();
  ClearStencilCommand* AsClearStencil();
  ComputeCommand* AsCompute();
  DrawArraysCommand* AsDrawArrays();
  DrawRectCommand* AsDrawRect();
  EntryPointCommand* AsEntryPoint();
  PatchParameterVerticesCommand* AsPatchParameterVertices();
  ProbeCommand* AsProbe();
  ProbeSSBOCommand* AsProbeSSBO();
  BufferCommand* AsBuffer();

  void SetLine(size_t line) { line_ = line; }
  size_t GetLine() const { return line_; }

 protected:
  explicit Command(Type type);

  Type command_type_;
  size_t line_ = 1;
};

class DrawRectCommand : public Command {
 public:
  explicit DrawRectCommand(PipelineData data);
  ~DrawRectCommand() override;

  const PipelineData* GetPipelineData() const { return &data_; }

  void EnableOrtho() { is_ortho_ = true; }
  bool IsOrtho() const { return is_ortho_; }

  void EnablePatch() { is_patch_ = true; }
  bool IsPatch() const { return is_patch_; }

  void SetX(float x) { x_ = x; }
  float GetX() const { return x_; }

  void SetY(float y) { y_ = y; }
  float GetY() const { return y_; }

  void SetWidth(float w) { width_ = w; }
  float GetWidth() const { return width_; }

  void SetHeight(float h) { height_ = h; }
  float GetHeight() const { return height_; }

 private:
  PipelineData data_;
  bool is_ortho_ = false;
  bool is_patch_ = false;
  float x_ = 0.0;
  float y_ = 0.0;
  float width_ = 0.0;
  float height_ = 0.0;
};

class DrawArraysCommand : public Command {
 public:
  explicit DrawArraysCommand(PipelineData data);
  ~DrawArraysCommand() override;

  const PipelineData* GetPipelineData() const { return &data_; }

  void EnableIndexed() { is_indexed_ = true; }
  bool IsIndexed() const { return is_indexed_; }

  void EnableInstanced() { is_instanced_ = true; }
  bool IsInstanced() const { return is_instanced_; }

  void SetTopology(Topology topo) { topology_ = topo; }
  Topology GetTopology() const { return topology_; }

  void SetFirstVertexIndex(uint32_t idx) { first_vertex_index_ = idx; }
  uint32_t GetFirstVertexIndex() const { return first_vertex_index_; }

  void SetVertexCount(uint32_t count) { vertex_count_ = count; }
  uint32_t GetVertexCount() const { return vertex_count_; }

  void SetInstanceCount(uint32_t count) { instance_count_ = count; }
  uint32_t GetInstanceCount() const { return instance_count_; }

 private:
  PipelineData data_;
  bool is_indexed_ = false;
  bool is_instanced_ = false;
  Topology topology_ = Topology::kUnknown;
  uint32_t first_vertex_index_ = 0;
  uint32_t vertex_count_ = 0;
  uint32_t instance_count_ = 0;
};

class ComputeCommand : public Command {
 public:
  explicit ComputeCommand(PipelineData data);
  ~ComputeCommand() override;

  const PipelineData* GetPipelineData() const { return &data_; }

  void SetX(uint32_t x) { x_ = x; }
  uint32_t GetX() const { return x_; }

  void SetY(uint32_t y) { y_ = y; }
  uint32_t GetY() const { return y_; }

  void SetZ(uint32_t z) { z_ = z; }
  uint32_t GetZ() const { return z_; }

 private:
  PipelineData data_;

  uint32_t x_ = 0;
  uint32_t y_ = 0;
  uint32_t z_ = 0;
};

class Probe : public Command {
 public:
  struct Tolerance {
    Tolerance(bool percent, double val) : is_percent(percent), value(val) {}

    bool is_percent = false;
    double value = 0.0;
  };

  ~Probe() override;

  bool HasTolerances() const { return !tolerances_.empty(); }
  void SetTolerances(const std::vector<Tolerance>& t) { tolerances_ = t; }
  const std::vector<Tolerance>& GetTolerances() const { return tolerances_; }

 protected:
  explicit Probe(Type type);

 private:
  std::vector<Tolerance> tolerances_;
};

class ProbeCommand : public Probe {
 public:
  ProbeCommand();
  ~ProbeCommand() override;

  void SetWholeWindow() { is_whole_window_ = true; }
  bool IsWholeWindow() const { return is_whole_window_; }

  void SetProbeRect() { is_probe_rect_ = true; }
  bool IsProbeRect() const { return is_probe_rect_; }

  void SetRelative() { is_relative_ = true; }
  bool IsRelative() const { return is_relative_; }

  void SetIsRGBA() { color_format_ = ColorFormat::kRGBA; }
  bool IsRGBA() const { return color_format_ == ColorFormat::kRGBA; }

  void SetX(float x) { x_ = x; }
  float GetX() const { return x_; }

  void SetY(float y) { y_ = y; }
  float GetY() const { return y_; }

  void SetWidth(float w) { width_ = w; }
  float GetWidth() const { return width_; }

  void SetHeight(float h) { height_ = h; }
  float GetHeight() const { return height_; }

  void SetR(float r) { r_ = r; }
  float GetR() const { return r_; }

  void SetG(float g) { g_ = g; }
  float GetG() const { return g_; }

  void SetB(float b) { b_ = b; }
  float GetB() const { return b_; }

  void SetA(float a) { a_ = a; }
  float GetA() const { return a_; }

 private:
  enum class ColorFormat {
    kRGB = 0,
    kRGBA,
  };

  bool is_whole_window_ = false;
  bool is_probe_rect_ = false;
  bool is_relative_ = false;
  ColorFormat color_format_ = ColorFormat::kRGB;

  float x_ = 0.0;
  float y_ = 0.0;
  float width_ = 1.0;
  float height_ = 1.0;

  float r_ = 0.0;
  float g_ = 0.0;
  float b_ = 0.0;
  float a_ = 0.0;
};

class ProbeSSBOCommand : public Probe {
 public:
  enum class Comparator {
    kEqual,
    kNotEqual,
    kFuzzyEqual,
    kLess,
    kLessOrEqual,
    kGreater,
    kGreaterOrEqual
  };

  ProbeSSBOCommand();
  ~ProbeSSBOCommand() override;

  void SetComparator(Comparator comp) { comparator_ = comp; }
  Comparator GetComparator() const { return comparator_; }

  void SetDescriptorSet(uint32_t id) { descriptor_set_id_ = id; }
  uint32_t GetDescriptorSet() const { return descriptor_set_id_; }

  void SetBinding(uint32_t id) { binding_num_ = id; }
  uint32_t GetBinding() const { return binding_num_; }

  void SetOffset(uint32_t offset) { offset_ = offset; }
  uint32_t GetOffset() const { return offset_; }

  void SetDatumType(const DatumType& type) { datum_type_ = type; }
  const DatumType& GetDatumType() const { return datum_type_; }

  void SetValues(std::vector<Value>&& values) { values_ = std::move(values); }
  const std::vector<Value>& GetValues() const { return values_; }

 private:
  Comparator comparator_ = Comparator::kEqual;
  uint32_t descriptor_set_id_ = 0;
  uint32_t binding_num_ = 0;
  uint32_t offset_ = 0;
  DatumType datum_type_;
  std::vector<Value> values_;
};

class BufferCommand : public Command {
 public:
  enum class BufferType {
    kSSBO,
    kUniform,
    kPushConstant,
  };

  explicit BufferCommand(BufferType type);
  ~BufferCommand() override;

  bool IsSSBO() const { return buffer_type_ == BufferType::kSSBO; }
  bool IsUniform() const { return buffer_type_ == BufferType::kUniform; }
  bool IsPushConstant() const {
    return buffer_type_ == BufferType::kPushConstant;
  }

  void SetIsSubdata() { is_subdata_ = true; }
  bool IsSubdata() const { return is_subdata_; }

  void SetDescriptorSet(uint32_t set) { descriptor_set_ = set; }
  uint32_t GetDescriptorSet() const { return descriptor_set_; }

  void SetBinding(uint32_t num) { binding_num_ = num; }
  uint32_t GetBinding() const { return binding_num_; }

  void SetOffset(uint32_t offset) { offset_ = offset; }
  uint32_t GetOffset() const { return offset_; }

  void SetSize(uint32_t size) { size_ = size; }
  uint32_t GetSize() const { return size_; }

  void SetDatumType(const DatumType& type) { datum_type_ = type; }
  const DatumType& GetDatumType() const { return datum_type_; }

  void SetValues(std::vector<Value>&& values) {
    values_ = std::move(values);
    size_ = static_cast<uint32_t>(values_.size() * datum_type_.SizeInBytes()) /
            datum_type_.ColumnCount() / datum_type_.RowCount();
  }
  const std::vector<Value>& GetValues() const { return values_; }

 private:
  BufferType buffer_type_;
  bool is_subdata_ = false;
  uint32_t descriptor_set_ = 0;
  uint32_t binding_num_ = 0;
  uint32_t size_ = 0;
  uint32_t offset_ = 0;
  DatumType datum_type_;
  std::vector<Value> values_;
};

class ClearCommand : public Command {
 public:
  ClearCommand();
  ~ClearCommand() override;
};

class ClearColorCommand : public Command {
 public:
  ClearColorCommand();
  ~ClearColorCommand() override;

  void SetR(float r) { r_ = r; }
  float GetR() const { return r_; }

  void SetG(float g) { g_ = g; }
  float GetG() const { return g_; }

  void SetB(float b) { b_ = b; }
  float GetB() const { return b_; }

  void SetA(float a) { a_ = a; }
  float GetA() const { return a_; }

 private:
  float r_ = 0.0;
  float g_ = 0.0;
  float b_ = 0.0;
  float a_ = 0.0;
};

class ClearDepthCommand : public Command {
 public:
  ClearDepthCommand();
  ~ClearDepthCommand() override;

  void SetValue(float val) { value_ = val; }
  float GetValue() const { return value_; }

 private:
  float value_ = 0.0;
};

class ClearStencilCommand : public Command {
 public:
  ClearStencilCommand();
  ~ClearStencilCommand() override;

  void SetValue(uint32_t val) { value_ = val; }
  uint32_t GetValue() const { return value_; }

 private:
  uint32_t value_ = 0;
};

class PatchParameterVerticesCommand : public Command {
 public:
  PatchParameterVerticesCommand();
  ~PatchParameterVerticesCommand() override;

  void SetControlPointCount(uint32_t count) { control_point_count_ = count; }
  uint32_t GetControlPointCount() const { return control_point_count_; }

 private:
  uint32_t control_point_count_ = 0;
};

class EntryPointCommand : public Command {
 public:
  EntryPointCommand();
  ~EntryPointCommand() override;

  void SetShaderType(ShaderType type) { shader_type_ = type; }
  ShaderType GetShaderType() const { return shader_type_; }

  void SetEntryPointName(const std::string& name) { entry_point_name_ = name; }
  std::string GetEntryPointName() const { return entry_point_name_; }

 private:
  ShaderType shader_type_ = kShaderTypeVertex;
  std::string entry_point_name_;
};

}  // namespace amber

#endif  // SRC_COMMAND_H_
