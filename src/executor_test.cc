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

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/engine.h"
#include "src/executor.h"
#include "src/make_unique.h"
#include "src/vkscript/parser.h"

namespace amber {
namespace vkscript {
namespace {

class EngineStub : public Engine {
 public:
  EngineStub() : Engine() {}
  ~EngineStub() override = default;

  // Engine
  Result Initialize(const std::vector<Feature>& features,
                    const std::vector<std::string>& extensions) override {
    features_ = features;
    extensions_ = extensions;
    return {};
  }

  Result InitializeWithConfig(EngineConfig*,
                              const std::vector<Feature>&,
                              const std::vector<std::string>&) override {
    return {};
  }

  Result Shutdown() override { return {}; }

  const std::vector<Feature>& GetFeatures() const { return features_; }
  const std::vector<std::string>& GetExtensions() const { return extensions_; }
  FormatType GetColorFrameFormat() const { return color_frame_format_; }
  FormatType GetDepthFrameFormat() const { return depth_frame_format_; }
  uint32_t GetFenceTimeoutMs() { return GetEngineData().fence_timeout_ms; }

  Result CreatePipeline(PipelineType) override { return {}; }

  void FailShaderCommand() { fail_shader_command_ = true; }
  const std::vector<ShaderType>& GetShaderTypesSeen() const {
    return shaders_seen_;
  }
  Result SetShader(ShaderType type, const std::vector<uint32_t>&) override {
    if (fail_shader_command_)
      return Result("shader command failed");

    shaders_seen_.push_back(type);
    return {};
  }

  uint8_t GetBufferCallCount() const { return buffer_call_count_; }
  BufferType GetBufferType(size_t idx) const { return buffer_types_[idx]; }
  uint8_t GetBufferLocation(size_t idx) const { return buffer_locations_[idx]; }
  Format* GetBufferFormat(size_t idx) { return &(buffer_formats_[idx]); }
  const std::vector<Value>& GetBufferValues(size_t idx) const {
    return buffer_values_[idx];
  }
  Result SetBuffer(BufferType type,
                   uint8_t location,
                   const Format& format,
                   const std::vector<Value>& data) override {
    if (type == BufferType::kColor || type == BufferType::kDepth) {
      if (type == BufferType::kColor)
        color_frame_format_ = format.GetFormatType();
      else if (type == BufferType::kDepth)
        depth_frame_format_ = format.GetFormatType();
      return {};
    }

    ++buffer_call_count_;
    buffer_types_.push_back(type);
    buffer_locations_.push_back(location);
    buffer_formats_.push_back(format);
    buffer_values_.push_back(data);
    return {};
  }

  void FailClearColorCommand() { fail_clear_color_command_ = true; }
  bool DidClearColorCommand() { return did_clear_color_command_ = true; }
  ClearColorCommand* GetLastClearColorCommand() { return last_clear_color_; }
  Result DoClearColor(const ClearColorCommand* cmd) override {
    did_clear_color_command_ = true;

    if (fail_clear_color_command_)
      return Result("clear color command failed");

    last_clear_color_ = const_cast<ClearColorCommand*>(cmd);
    return {};
  }

  void FailClearStencilCommand() { fail_clear_stencil_command_ = true; }
  bool DidClearStencilCommand() const { return did_clear_stencil_command_; }
  Result DoClearStencil(const ClearStencilCommand*) override {
    did_clear_stencil_command_ = true;

    if (fail_clear_stencil_command_)
      return Result("clear stencil command failed");

    return {};
  }

  void FailClearDepthCommand() { fail_clear_depth_command_ = true; }
  bool DidClearDepthCommand() const { return did_clear_depth_command_; }
  Result DoClearDepth(const ClearDepthCommand*) override {
    did_clear_depth_command_ = true;

    if (fail_clear_depth_command_)
      return Result("clear depth command failed");

    return {};
  }

  void FailClearCommand() { fail_clear_command_ = true; }
  bool DidClearCommand() const { return did_clear_command_; }
  Result DoClear(const ClearCommand*) override {
    did_clear_command_ = true;

    if (fail_clear_command_)
      return Result("clear command failed");
    return {};
  }

  void FailDrawRectCommand() { fail_draw_rect_command_ = true; }
  bool DidDrawRectCommand() const { return did_draw_rect_command_; }
  Result DoDrawRect(const DrawRectCommand*) override {
    did_draw_rect_command_ = true;

    if (fail_draw_rect_command_)
      return Result("draw rect command failed");
    return {};
  }

  void FailDrawArraysCommand() { fail_draw_arrays_command_ = true; }
  bool DidDrawArraysCommand() const { return did_draw_arrays_command_; }
  Result DoDrawArrays(const DrawArraysCommand*) override {
    did_draw_arrays_command_ = true;

    if (fail_draw_arrays_command_)
      return Result("draw arrays command failed");
    return {};
  }

  void FailComputeCommand() { fail_compute_command_ = true; }
  bool DidComputeCommand() const { return did_compute_command_; }
  Result DoCompute(const ComputeCommand*) override {
    did_compute_command_ = true;

    if (fail_compute_command_)
      return Result("compute command failed");
    return {};
  }

  void FailEntryPointCommand() { fail_entry_point_command_ = true; }
  bool DidEntryPointCommand() const { return did_entry_point_command_; }
  Result DoEntryPoint(const EntryPointCommand*) override {
    did_entry_point_command_ = true;

    if (fail_entry_point_command_)
      return Result("entrypoint command failed");
    return {};
  }

  void FailPatchParameterVerticesCommand() { fail_patch_command_ = true; }
  bool DidPatchParameterVerticesCommand() const { return did_patch_command_; }
  Result DoPatchParameterVertices(
      const PatchParameterVerticesCommand*) override {
    did_patch_command_ = true;

    if (fail_patch_command_)
      return Result("patch command failed");
    return {};
  }

  void FailBufferCommand() { fail_buffer_command_ = true; }
  bool DidBufferCommand() const { return did_buffer_command_; }
  Result DoBuffer(const BufferCommand*) override {
    did_buffer_command_ = true;

    if (fail_buffer_command_)
      return Result("buffer command failed");
    return {};
  }

  Result DoProcessCommands() override { return {}; }
  Result GetFrameBufferInfo(ResourceInfo*) override { return {}; }
  Result GetDescriptorInfo(const uint32_t,
                           const uint32_t,
                           ResourceInfo*) override {
    return {};
  }

 private:
  bool fail_shader_command_ = false;
  bool fail_clear_command_ = false;
  bool fail_clear_color_command_ = false;
  bool fail_clear_stencil_command_ = false;
  bool fail_clear_depth_command_ = false;
  bool fail_draw_rect_command_ = false;
  bool fail_draw_arrays_command_ = false;
  bool fail_compute_command_ = false;
  bool fail_entry_point_command_ = false;
  bool fail_patch_command_ = false;
  bool fail_buffer_command_ = false;

  bool did_clear_command_ = false;
  bool did_clear_color_command_ = false;
  bool did_clear_stencil_command_ = false;
  bool did_clear_depth_command_ = false;
  bool did_draw_rect_command_ = false;
  bool did_draw_arrays_command_ = false;
  bool did_compute_command_ = false;
  bool did_entry_point_command_ = false;
  bool did_patch_command_ = false;
  bool did_buffer_command_ = false;

  uint8_t buffer_call_count_ = 0;
  std::vector<uint8_t> buffer_locations_;
  std::vector<BufferType> buffer_types_;
  std::vector<Format> buffer_formats_;
  std::vector<std::vector<Value>> buffer_values_;

  std::vector<ShaderType> shaders_seen_;
  FormatType color_frame_format_ = FormatType::kUnknown;
  FormatType depth_frame_format_ = FormatType::kUnknown;
  std::vector<Feature> features_;
  std::vector<std::string> extensions_;

  ClearColorCommand* last_clear_color_ = nullptr;
};

class VkScriptExecutorTest : public testing::Test {
 public:
  VkScriptExecutorTest() = default;
  ~VkScriptExecutorTest() = default;

  std::unique_ptr<Engine> MakeEngine() { return MakeUnique<EngineStub>(); }
  std::unique_ptr<Engine> MakeAndInitializeEngine(
      const std::vector<Feature>& features,
      const std::vector<std::string>& extensions) {
    auto engine = MakeUnique<EngineStub>();
    engine->Initialize(features, extensions);
    return std::move(engine);
  }
  EngineStub* ToStub(Engine* engine) {
    return static_cast<EngineStub*>(engine);
  }
};

}  // namespace

TEST_F(VkScriptExecutorTest, ExecutesRequiredFeatures) {
  std::string input = R"(
[require]
robustBufferAccess
logicOp)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto script = parser.GetScript();
  auto engine = MakeAndInitializeEngine(script->RequiredFeatures(),
                                        script->RequiredExtensions());

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());

  const auto& features = ToStub(engine.get())->GetFeatures();
  ASSERT_EQ(2U, features.size());
  EXPECT_EQ(Feature::kRobustBufferAccess, features[0]);
  EXPECT_EQ(Feature::kLogicOp, features[1]);

  const auto& extensions = ToStub(engine.get())->GetExtensions();
  ASSERT_EQ(static_cast<size_t>(0U), extensions.size());

  EXPECT_EQ(100U, ToStub(engine.get())->GetFenceTimeoutMs());

  auto color_frame_format = ToStub(engine.get())->GetColorFrameFormat();
  auto depth_frame_format = ToStub(engine.get())->GetDepthFrameFormat();
  EXPECT_EQ(FormatType::kUnknown, color_frame_format);
  EXPECT_EQ(FormatType::kUnknown, depth_frame_format);
}

TEST_F(VkScriptExecutorTest, ExecutesRequiredExtensions) {
  std::string input = R"(
[require]
VK_KHR_storage_buffer_storage_class
VK_KHR_variable_pointers)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto script = parser.GetScript();
  auto engine = MakeAndInitializeEngine(script->RequiredFeatures(),
                                        script->RequiredExtensions());

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());

  const auto& features = ToStub(engine.get())->GetFeatures();
  ASSERT_EQ(static_cast<size_t>(0U), features.size());

  const auto& extensions = ToStub(engine.get())->GetExtensions();
  ASSERT_EQ(2U, extensions.size());
  EXPECT_EQ("VK_KHR_storage_buffer_storage_class", extensions[0]);
  EXPECT_EQ("VK_KHR_variable_pointers", extensions[1]);

  EXPECT_EQ(100U, ToStub(engine.get())->GetFenceTimeoutMs());

  auto color_frame_format = ToStub(engine.get())->GetColorFrameFormat();
  auto depth_frame_format = ToStub(engine.get())->GetDepthFrameFormat();
  EXPECT_EQ(FormatType::kUnknown, color_frame_format);
  EXPECT_EQ(FormatType::kUnknown, depth_frame_format);
}

TEST_F(VkScriptExecutorTest, ExecutesRequiredFrameBuffers) {
  std::string input = R"(
[require]
framebuffer R32G32B32A32_SFLOAT
depthstencil D24_UNORM_S8_UINT)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto script = parser.GetScript();
  auto engine = MakeAndInitializeEngine(script->RequiredFeatures(),
                                        script->RequiredExtensions());

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());

  const auto& features = ToStub(engine.get())->GetFeatures();
  ASSERT_EQ(static_cast<size_t>(0U), features.size());

  const auto& extensions = ToStub(engine.get())->GetExtensions();
  ASSERT_EQ(static_cast<size_t>(0U), extensions.size());

  EXPECT_EQ(100U, ToStub(engine.get())->GetFenceTimeoutMs());

  auto color_frame_format = ToStub(engine.get())->GetColorFrameFormat();
  auto depth_frame_format = ToStub(engine.get())->GetDepthFrameFormat();
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT, color_frame_format);
  EXPECT_EQ(FormatType::kD24_UNORM_S8_UINT, depth_frame_format);
}

TEST_F(VkScriptExecutorTest, ExecutesRequiredFenceTimeout) {
  std::string input = R"(
[require]
fence_timeout 12345)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto script = parser.GetScript();
  auto engine = MakeAndInitializeEngine(script->RequiredFeatures(),
                                        script->RequiredExtensions());

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());

  const auto& features = ToStub(engine.get())->GetFeatures();
  ASSERT_EQ(static_cast<size_t>(0U), features.size());

  const auto& extensions = ToStub(engine.get())->GetExtensions();
  ASSERT_EQ(static_cast<size_t>(0U), extensions.size());

  EXPECT_EQ(12345U, ToStub(engine.get())->GetFenceTimeoutMs());

  auto color_frame_format = ToStub(engine.get())->GetColorFrameFormat();
  auto depth_frame_format = ToStub(engine.get())->GetDepthFrameFormat();
  EXPECT_EQ(FormatType::kUnknown, color_frame_format);
  EXPECT_EQ(FormatType::kUnknown, depth_frame_format);
}

TEST_F(VkScriptExecutorTest, ExecutesRequiredAll) {
  std::string input = R"(
[require]
robustBufferAccess
logicOp
VK_KHR_storage_buffer_storage_class
VK_KHR_variable_pointers
framebuffer R32G32B32A32_SFLOAT
depthstencil D24_UNORM_S8_UINT
fence_timeout 12345)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto script = parser.GetScript();
  auto engine = MakeAndInitializeEngine(script->RequiredFeatures(),
                                        script->RequiredExtensions());

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());

  const auto& features = ToStub(engine.get())->GetFeatures();
  ASSERT_EQ(2U, features.size());
  EXPECT_EQ(Feature::kRobustBufferAccess, features[0]);
  EXPECT_EQ(Feature::kLogicOp, features[1]);

  const auto& extensions = ToStub(engine.get())->GetExtensions();
  ASSERT_EQ(2U, extensions.size());
  EXPECT_EQ("VK_KHR_storage_buffer_storage_class", extensions[0]);
  EXPECT_EQ("VK_KHR_variable_pointers", extensions[1]);

  EXPECT_EQ(12345U, ToStub(engine.get())->GetFenceTimeoutMs());

  auto color_frame_format = ToStub(engine.get())->GetColorFrameFormat();
  auto depth_frame_format = ToStub(engine.get())->GetDepthFrameFormat();
  EXPECT_EQ(FormatType::kR32G32B32A32_SFLOAT, color_frame_format);
  EXPECT_EQ(FormatType::kD24_UNORM_S8_UINT, depth_frame_format);
}

#if AMBER_ENABLE_SHADERC
TEST_F(VkScriptExecutorTest, ExecutesShaders) {
  std::string input = R"(
[vertex shader passthrough]
[fragment shader]
#version 430
void main() {}
)";

  Parser parser;
  Result r = parser.Parse(input);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());

  auto shader_types = ToStub(engine.get())->GetShaderTypesSeen();
  ASSERT_EQ(2U, shader_types.size());
  EXPECT_EQ(ShaderType::kVertex, shader_types[0]);
  EXPECT_EQ(ShaderType::kFragment, shader_types[1]);
}

TEST_F(VkScriptExecutorTest, ShaderFailure) {
  std::string input = R"(
[vertex shader passthrough]
[fragment shader]
#version 430
void main() {}
)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailShaderCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("shader command failed", r.Error());
}
#endif  // AMBER_ENABLE_SHADERC

TEST_F(VkScriptExecutorTest, ClearCommand) {
  std::string input = R"(
[test]
clear)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_TRUE(ToStub(engine.get())->DidClearCommand());
}

TEST_F(VkScriptExecutorTest, ClearCommandFailure) {
  std::string input = R"(
[test]
clear)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailClearCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("clear command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, ClearColorCommand) {
  std::string input = R"(
[test]
clear color 244 123 123 13)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidClearColorCommand());

  auto* cmd = ToStub(engine.get())->GetLastClearColorCommand();
  ASSERT_TRUE(cmd != nullptr);
  ASSERT_TRUE(cmd->IsClearColor());

  EXPECT_EQ(244U, cmd->GetR());
  EXPECT_EQ(123U, cmd->GetG());
  EXPECT_EQ(123U, cmd->GetB());
  EXPECT_EQ(13U, cmd->GetA());
}

TEST_F(VkScriptExecutorTest, ClearColorCommandFailure) {
  std::string input = R"(
[test]
clear color 123 123 123 123)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailClearColorCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("clear color command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, ClearDepthCommand) {
  std::string input = R"(
[test]
clear depth 24)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidClearDepthCommand());
}

TEST_F(VkScriptExecutorTest, ClearDepthCommandFailure) {
  std::string input = R"(
[test]
clear depth 24)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailClearDepthCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("clear depth command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, ClearStencilCommand) {
  std::string input = R"(
[test]
clear stencil 24)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidClearStencilCommand());
}

TEST_F(VkScriptExecutorTest, ClearStencilCommandFailure) {
  std::string input = R"(
[test]
clear stencil 24)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailClearStencilCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("clear stencil command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, DrawRectCommand) {
  std::string input = R"(
[test]
draw rect 2 4 10 20)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidDrawRectCommand());
}

TEST_F(VkScriptExecutorTest, DrawRectCommandFailure) {
  std::string input = R"(
[test]
draw rect 2 4 10 20)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailDrawRectCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("draw rect command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, DrawArraysCommand) {
  std::string input = R"(
[test]
draw arrays TRIANGLE_LIST 0 0)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidDrawArraysCommand());
}

TEST_F(VkScriptExecutorTest, DrawArraysCommandFailure) {
  std::string input = R"(
[test]
draw arrays TRIANGLE_LIST 0 0)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailDrawArraysCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("draw arrays command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, ComputeCommand) {
  std::string input = R"(
[test]
compute 2 3 4)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidComputeCommand());
}

TEST_F(VkScriptExecutorTest, ComputeCommandFailure) {
  std::string input = R"(
[test]
compute 2 3 4)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailComputeCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("compute command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, EntryPointCommand) {
  std::string input = R"(
[test]
vertex entrypoint main)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidEntryPointCommand());
}

TEST_F(VkScriptExecutorTest, EntryPointCommandFailure) {
  std::string input = R"(
[test]
vertex entrypoint main)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailEntryPointCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("entrypoint command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, PatchParameterVerticesCommand) {
  std::string input = R"(
[test]
patch parameter vertices 10)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidPatchParameterVerticesCommand());
}

TEST_F(VkScriptExecutorTest, PatchParameterVerticesCommandFailure) {
  std::string input = R"(
[test]
patch parameter vertices 10)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailPatchParameterVerticesCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("patch command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, DISABLED_ProbeCommand) {
  std::string input = R"(
[test]
probe rect rgba 2 3 40 40 0.2 0.4 0.4 0.3)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  // ASSERT_TRUE(ToStub(engine.get())->DidProbeCommand());
}

TEST_F(VkScriptExecutorTest, DISABLED_ProbeCommandFailure) {
  std::string input = R"(
[test]
probe rect rgba 2 3 40 40 0.2 0.4 0.4 0.3)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  // ToStub(engine.get())->FailProbeCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("probe command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, BufferCommand) {
  std::string input = R"(
[test]
ssbo 0 24)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidBufferCommand());
}

TEST_F(VkScriptExecutorTest, BufferCommandFailure) {
  std::string input = R"(
[test]
ssbo 0 24)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailBufferCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("buffer command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, DISABLED_ProbeSSBOCommand) {
  std::string input = R"(
[test]
probe ssbo vec3 0 2 <= 2 3 4)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());
  // ASSERT_TRUE(ToStub(engine.get())->DidProbeSSBOCommand());
}

TEST_F(VkScriptExecutorTest, DISABLED_ProbeSSBOCommandFailure) {
  std::string input = R"(
[test]
probe ssbo vec3 0 2 <= 2 3 4)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  // ToStub(engine.get())->FailProbeSSBOCommand();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("probe ssbo command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, VertexData) {
  std::string input = R"(
[vertex data]
9/R32G32B32_SFLOAT  1/R8G8B8_UNORM
-1    -1 0.25       255 128 64
0.25  -1 0.25       255 0 0
)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());
  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());

  auto stub = ToStub(engine.get());
  ASSERT_EQ(2U, stub->GetBufferCallCount());

  EXPECT_EQ(FormatType::kR32G32B32_SFLOAT,
            stub->GetBufferFormat(0)->GetFormatType());
  EXPECT_EQ(BufferType::kVertex, stub->GetBufferType(0));
  EXPECT_EQ(9U, stub->GetBufferLocation(0));

  const auto& data1 = stub->GetBufferValues(0);
  std::vector<float> results1 = {-1, -1, 0.25, 0.25, -1, 0.25};
  ASSERT_EQ(results1.size(), data1.size());
  for (size_t i = 0; i < results1.size(); ++i) {
    ASSERT_TRUE(data1[i].IsFloat());
    EXPECT_FLOAT_EQ(results1[i], data1[i].AsFloat());
  }

  EXPECT_EQ(FormatType::kR8G8B8_UNORM,
            stub->GetBufferFormat(1)->GetFormatType());
  EXPECT_EQ(BufferType::kVertex, stub->GetBufferType(1));
  EXPECT_EQ(1U, stub->GetBufferLocation(1));

  const auto& data2 = stub->GetBufferValues(1);
  std::vector<uint8_t> results2 = {255, 128, 64, 255, 0, 0};
  ASSERT_EQ(results2.size(), data2.size());
  for (size_t i = 0; i < results2.size(); ++i) {
    ASSERT_TRUE(data2[i].IsInteger());
    EXPECT_EQ(results2[i], data2[i].AsUint8());
  }
}

TEST_F(VkScriptExecutorTest, IndexBuffer) {
  std::string input = R"(
[indices]
1 2 3 4 5 6
)";

  Parser parser;
  ASSERT_TRUE(parser.Parse(input).IsSuccess());
  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap());
  ASSERT_TRUE(r.IsSuccess());

  auto stub = ToStub(engine.get());
  ASSERT_EQ(1U, stub->GetBufferCallCount());

  EXPECT_EQ(BufferType::kIndex, stub->GetBufferType(0));

  const auto& data = stub->GetBufferValues(0);
  std::vector<uint8_t> results = {1, 2, 3, 4, 5, 6};
  ASSERT_EQ(results.size(), data.size());
  for (size_t i = 0; i < results.size(); ++i) {
    ASSERT_TRUE(data[i].IsInteger());
    EXPECT_EQ(results[i], data[i].AsUint8());
  }
}

}  // namespace vkscript
}  // namespace amber
