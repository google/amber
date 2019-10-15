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
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "amber/shader_info.h"
#include "amber/value.h"
#include "src/buffer.h"
#include "src/command_data.h"
#include "src/pipeline_data.h"

namespace amber {

class BufferCommand;
class ClearColorCommand;
class ClearCommand;
class ClearDepthCommand;
class ClearStencilCommand;
class CompareBufferCommand;
class ComputeCommand;
class CopyCommand;
class DrawArraysCommand;
class DrawRectCommand;
class EntryPointCommand;
class PatchParameterVerticesCommand;
class Pipeline;
class ProbeCommand;
class ProbeSSBOCommand;
class RepeatCommand;

/// Base class for all commands.
class Command {
 public:
  enum class Type : uint8_t {
    kClear = 0,
    kClearColor,
    kClearDepth,
    kClearStencil,
    kCompute,
    kCompareBuffer,
    kCopy,
    kDrawArrays,
    kDrawRect,
    kEntryPoint,
    kPatchParameterVertices,
    kPipelineProperties,
    kProbe,
    kProbeSSBO,
    kBuffer,
    kRepeat
  };

  virtual ~Command();

  Command::Type GetType() const { return command_type_; }

  bool IsDrawRect() const { return command_type_ == Type::kDrawRect; }
  bool IsDrawArrays() const { return command_type_ == Type::kDrawArrays; }
  bool IsCompareBuffer() const { return command_type_ == Type::kCompareBuffer; }
  bool IsCompute() const { return command_type_ == Type::kCompute; }
  bool IsCopy() const { return command_type_ == Type::kCopy; }
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
  bool IsRepeat() { return command_type_ == Type::kRepeat; }

  ClearCommand* AsClear();
  ClearColorCommand* AsClearColor();
  ClearDepthCommand* AsClearDepth();
  ClearStencilCommand* AsClearStencil();
  CompareBufferCommand* AsCompareBuffer();
  ComputeCommand* AsCompute();
  CopyCommand* AsCopy();
  DrawArraysCommand* AsDrawArrays();
  DrawRectCommand* AsDrawRect();
  EntryPointCommand* AsEntryPoint();
  PatchParameterVerticesCommand* AsPatchParameterVertices();
  ProbeCommand* AsProbe();
  ProbeSSBOCommand* AsProbeSSBO();
  BufferCommand* AsBuffer();
  RepeatCommand* AsRepeat();

  virtual std::string ToString() const = 0;

  /// Sets the input file line number this command is declared on.
  void SetLine(size_t line) { line_ = line; }
  /// Returns the input file line this command was declared on.
  size_t GetLine() const { return line_; }

 protected:
  explicit Command(Type type);

  Type command_type_;
  size_t line_ = 1;
};

/// Base class for commands which contain a pipeline.
class PipelineCommand : public Command {
 public:
  ~PipelineCommand() override;

  Pipeline* GetPipeline() const { return pipeline_; }

 protected:
  explicit PipelineCommand(Type type, Pipeline* pipeline);

  Pipeline* pipeline_ = nullptr;
};

/// Command to draw a rectangle on screen.
class DrawRectCommand : public PipelineCommand {
 public:
  explicit DrawRectCommand(Pipeline* pipeline, PipelineData data);
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

  std::string ToString() const override { return "DrawRectCommand"; }

 private:
  PipelineData data_;
  bool is_ortho_ = false;
  bool is_patch_ = false;
  float x_ = 0.0;
  float y_ = 0.0;
  float width_ = 0.0;
  float height_ = 0.0;
};

/// Command to draw from a vertex and index buffer.
class DrawArraysCommand : public PipelineCommand {
 public:
  explicit DrawArraysCommand(Pipeline* pipeline, PipelineData data);
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

  std::string ToString() const override { return "DrawArraysCommand"; }

 private:
  PipelineData data_;
  bool is_indexed_ = false;
  bool is_instanced_ = false;
  Topology topology_ = Topology::kUnknown;
  uint32_t first_vertex_index_ = 0;
  uint32_t vertex_count_ = 0;
  uint32_t instance_count_ = 0;
};

/// A command to compare two buffers.
class CompareBufferCommand : public Command {
 public:
  enum class Comparator { kEq, kRmse, kHistogramEmd };

  CompareBufferCommand(Buffer* buffer_1, Buffer* buffer_2);
  ~CompareBufferCommand() override;

  Buffer* GetBuffer1() const { return buffer_1_; }
  Buffer* GetBuffer2() const { return buffer_2_; }

  void SetComparator(Comparator type) { comparator_ = type; }
  Comparator GetComparator() const { return comparator_; }

  void SetTolerance(float tolerance) { tolerance_ = tolerance; }
  float GetTolerance() const { return tolerance_; }

  std::string ToString() const override { return "CompareBufferCommand"; }

 private:
  Buffer* buffer_1_;
  Buffer* buffer_2_;
  float tolerance_ = 0.0;
  Comparator comparator_ = Comparator::kEq;
};

/// Command to execute a compute command.
class ComputeCommand : public PipelineCommand {
 public:
  explicit ComputeCommand(Pipeline* pipeline);
  ~ComputeCommand() override;

  void SetX(uint32_t x) { x_ = x; }
  uint32_t GetX() const { return x_; }

  void SetY(uint32_t y) { y_ = y; }
  uint32_t GetY() const { return y_; }

  void SetZ(uint32_t z) { z_ = z; }
  uint32_t GetZ() const { return z_; }

  std::string ToString() const override { return "ComputeCommand"; }

 private:
  uint32_t x_ = 0;
  uint32_t y_ = 0;
  uint32_t z_ = 0;
};

/// Command to copy data from one buffer to another.
class CopyCommand : public Command {
 public:
  CopyCommand(Buffer* buffer_from, Buffer* buffer_to);
  ~CopyCommand() override;

  Buffer* GetBufferFrom() const { return buffer_from_; }
  Buffer* GetBufferTo() const { return buffer_to_; }

  std::string ToString() const override { return "CopyCommand"; }

 private:
  Buffer* buffer_from_;
  Buffer* buffer_to_;
};

/// Base class for probe commands.
class Probe : public Command {
 public:
  /// Wrapper around tolerance information for the probe.
  struct Tolerance {
    Tolerance(bool percent, double val) : is_percent(percent), value(val) {}

    bool is_percent = false;
    double value = 0.0;
  };

  ~Probe() override;

  Buffer* GetBuffer() const { return buffer_; }

  bool HasTolerances() const { return !tolerances_.empty(); }
  void SetTolerances(const std::vector<Tolerance>& t) { tolerances_ = t; }
  const std::vector<Tolerance>& GetTolerances() const { return tolerances_; }

 protected:
  explicit Probe(Type type, Buffer* buffer);

 private:
  Buffer* buffer_;
  std::vector<Tolerance> tolerances_;
};

/// Command to probe an image buffer.
class ProbeCommand : public Probe {
 public:
  explicit ProbeCommand(Buffer* buffer);
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

  // Colours are stored in the range 0.0 - 1.0
  void SetR(float r) { r_ = r; }
  float GetR() const { return r_; }

  void SetG(float g) { g_ = g; }
  float GetG() const { return g_; }

  void SetB(float b) { b_ = b; }
  float GetB() const { return b_; }

  void SetA(float a) { a_ = a; }
  float GetA() const { return a_; }

  std::string ToString() const override { return "ProbeCommand"; }

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

/// Command to probe a data buffer.
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

  explicit ProbeSSBOCommand(Buffer* buffer);
  ~ProbeSSBOCommand() override;

  void SetComparator(Comparator comp) { comparator_ = comp; }
  Comparator GetComparator() const { return comparator_; }

  void SetDescriptorSet(uint32_t id) { descriptor_set_id_ = id; }
  uint32_t GetDescriptorSet() const { return descriptor_set_id_; }

  void SetBinding(uint32_t id) { binding_num_ = id; }
  uint32_t GetBinding() const { return binding_num_; }

  void SetOffset(uint32_t offset) { offset_ = offset; }
  uint32_t GetOffset() const { return offset_; }

  void SetFormat(Format* fmt) { format_ = fmt; }
  Format* GetFormat() const { return format_; }

  void SetValues(std::vector<Value>&& values) { values_ = std::move(values); }
  const std::vector<Value>& GetValues() const { return values_; }

  std::string ToString() const override { return "ProbeSSBOCommand"; }

 private:
  Comparator comparator_ = Comparator::kEqual;
  uint32_t descriptor_set_id_ = 0;
  uint32_t binding_num_ = 0;
  uint32_t offset_ = 0;
  Format* format_;
  std::vector<Value> values_;
};

/// Command to set the size of a buffer, or update a buffers contents.
class BufferCommand : public PipelineCommand {
 public:
  enum class BufferType {
    kSSBO,
    kUniform,
    kPushConstant,
  };

  explicit BufferCommand(BufferType type, Pipeline* pipeline);
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

  void SetValues(std::vector<Value>&& values) { values_ = std::move(values); }
  const std::vector<Value>& GetValues() const { return values_; }

  void SetBuffer(Buffer* buffer) { buffer_ = buffer; }
  Buffer* GetBuffer() const { return buffer_; }

  std::string ToString() const override { return "BufferCommand"; }

 private:
  Buffer* buffer_ = nullptr;
  BufferType buffer_type_;
  bool is_subdata_ = false;
  uint32_t descriptor_set_ = 0;
  uint32_t binding_num_ = 0;
  uint32_t offset_ = 0;
  std::vector<Value> values_;
};

/// Command to clear the colour attachments.
class ClearCommand : public PipelineCommand {
 public:
  explicit ClearCommand(Pipeline* pipeline);
  ~ClearCommand() override;

  std::string ToString() const override { return "ClearCommand"; }
};

/// Command to set the colour for the clear command.
class ClearColorCommand : public PipelineCommand {
 public:
  explicit ClearColorCommand(Pipeline* pipeline);
  ~ClearColorCommand() override;

  // Colours are stored in the range 0.0 - 1.0
  void SetR(float r) { r_ = r; }
  float GetR() const { return r_; }

  void SetG(float g) { g_ = g; }
  float GetG() const { return g_; }

  void SetB(float b) { b_ = b; }
  float GetB() const { return b_; }

  void SetA(float a) { a_ = a; }
  float GetA() const { return a_; }

  std::string ToString() const override { return "ClearColorCommand"; }

 private:
  float r_ = 0.0;
  float g_ = 0.0;
  float b_ = 0.0;
  float a_ = 0.0;
};

/// Command to set the depth value for the clear command.
class ClearDepthCommand : public PipelineCommand {
 public:
  explicit ClearDepthCommand(Pipeline* pipeline);
  ~ClearDepthCommand() override;

  void SetValue(float val) { value_ = val; }
  float GetValue() const { return value_; }

  std::string ToString() const override { return "ClearDepthCommand"; }

 private:
  float value_ = 0.0;
};

/// Command to set the stencil value for the clear command.
class ClearStencilCommand : public PipelineCommand {
 public:
  explicit ClearStencilCommand(Pipeline* pipeline);
  ~ClearStencilCommand() override;

  void SetValue(uint32_t val) { value_ = val; }
  uint32_t GetValue() const { return value_; }

  std::string ToString() const override { return "ClearStencilCommand"; }

 private:
  uint32_t value_ = 0;
};

/// Command to set the patch parameter vertices.
class PatchParameterVerticesCommand : public PipelineCommand {
 public:
  explicit PatchParameterVerticesCommand(Pipeline* pipeline);
  ~PatchParameterVerticesCommand() override;

  void SetControlPointCount(uint32_t count) { control_point_count_ = count; }
  uint32_t GetControlPointCount() const { return control_point_count_; }

  std::string ToString() const override {
    return "PatchParameterVerticesCommand";
  }

 private:
  uint32_t control_point_count_ = 0;
};

/// Command to set the entry point to use for a given shader type.
class EntryPointCommand : public PipelineCommand {
 public:
  explicit EntryPointCommand(Pipeline* pipeline);
  ~EntryPointCommand() override;

  void SetShaderType(ShaderType type) { shader_type_ = type; }
  ShaderType GetShaderType() const { return shader_type_; }

  void SetEntryPointName(const std::string& name) { entry_point_name_ = name; }
  std::string GetEntryPointName() const { return entry_point_name_; }

  std::string ToString() const override { return "EntryPointCommand"; }

 private:
  ShaderType shader_type_ = kShaderTypeVertex;
  std::string entry_point_name_;
};

/// Command to repeat the given set of commands a number of times.
class RepeatCommand : public Command {
 public:
  explicit RepeatCommand(uint32_t count);
  ~RepeatCommand() override;

  uint32_t GetCount() const { return count_; }

  void SetCommands(std::vector<std::unique_ptr<Command>> cmds) {
    commands_ = std::move(cmds);
  }

  const std::vector<std::unique_ptr<Command>>& GetCommands() const {
    return commands_;
  }

  std::string ToString() const override { return "RepeatCommand"; }

 private:
  uint32_t count_ = 0;
  std::vector<std::unique_ptr<Command>> commands_;
};

}  // namespace amber

#endif  // SRC_COMMAND_H_
