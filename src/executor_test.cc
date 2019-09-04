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

#include "src/executor.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/engine.h"
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
  Result Initialize(EngineConfig*,
                    Delegate*,
                    const std::vector<std::string>& features,
                    const std::vector<std::string>& instance_exts,
                    const std::vector<std::string>& device_exts) override {
    features_ = features;
    instance_extensions_ = instance_exts;
    device_extensions_ = device_exts;
    return {};
  }

  const std::vector<std::string>& GetFeatures() const { return features_; }
  const std::vector<std::string>& GetDeviceExtensions() const {
    return device_extensions_;
  }
  uint32_t GetFenceTimeoutMs() { return GetEngineData().fence_timeout_ms; }

  Result CreatePipeline(Pipeline*) override { return {}; }

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

 private:
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

  std::vector<std::string> features_;
  std::vector<std::string> instance_extensions_;
  std::vector<std::string> device_extensions_;

  ClearColorCommand* last_clear_color_ = nullptr;
};

class VkScriptExecutorTest : public testing::Test {
 public:
  VkScriptExecutorTest() = default;
  ~VkScriptExecutorTest() = default;

  std::unique_ptr<Engine> MakeEngine() { return MakeUnique<EngineStub>(); }
  std::unique_ptr<Engine> MakeAndInitializeEngine(
      const std::vector<std::string>& features,
      const std::vector<std::string>& instance_extensions,
      const std::vector<std::string>& device_extensions) {
    auto engine = MakeUnique<EngineStub>();
    engine->Initialize(nullptr, nullptr, features, instance_extensions,
                       device_extensions);
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
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto script = parser.GetScript();
  auto engine = MakeAndInitializeEngine(script->GetRequiredFeatures(),
                                        script->GetRequiredInstanceExtensions(),
                                        script->GetRequiredDeviceExtensions());

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());

  const auto& features = ToStub(engine.get())->GetFeatures();
  ASSERT_EQ(2U, features.size());
  EXPECT_EQ("robustBufferAccess", features[0]);
  EXPECT_EQ("logicOp", features[1]);

  const auto& extensions = ToStub(engine.get())->GetDeviceExtensions();
  ASSERT_EQ(static_cast<size_t>(0U), extensions.size());
}

TEST_F(VkScriptExecutorTest, ExecutesRequiredExtensions) {
  std::string input = R"(
[require]
VK_KHR_storage_buffer_storage_class
VK_KHR_variable_pointers)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto script = parser.GetScript();
  auto engine = MakeAndInitializeEngine(script->GetRequiredFeatures(),
                                        script->GetRequiredInstanceExtensions(),
                                        script->GetRequiredDeviceExtensions());

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());

  const auto& features = ToStub(engine.get())->GetFeatures();
  ASSERT_EQ(static_cast<size_t>(0U), features.size());

  const auto& extensions = ToStub(engine.get())->GetDeviceExtensions();
  ASSERT_EQ(2U, extensions.size());
  EXPECT_EQ("VK_KHR_storage_buffer_storage_class", extensions[0]);
  EXPECT_EQ("VK_KHR_variable_pointers", extensions[1]);
}

TEST_F(VkScriptExecutorTest, ExecutesRequiredFrameBuffers) {
  std::string input = R"(
[require]
framebuffer R32G32B32A32_SFLOAT
depthstencil D24_UNORM_S8_UINT)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto script = parser.GetScript();
  auto engine = MakeAndInitializeEngine(script->GetRequiredFeatures(),
                                        script->GetRequiredInstanceExtensions(),
                                        script->GetRequiredDeviceExtensions());

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());

  const auto& features = ToStub(engine.get())->GetFeatures();
  ASSERT_EQ(static_cast<size_t>(0U), features.size());

  const auto& extensions = ToStub(engine.get())->GetDeviceExtensions();
  ASSERT_EQ(static_cast<size_t>(0U), extensions.size());
}

TEST_F(VkScriptExecutorTest, ExecutesRequiredFenceTimeout) {
  std::string input = R"(
[require]
fence_timeout 12345)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto script = parser.GetScript();
  auto engine = MakeAndInitializeEngine(script->GetRequiredFeatures(),
                                        script->GetRequiredInstanceExtensions(),
                                        script->GetRequiredDeviceExtensions());

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());

  const auto& features = ToStub(engine.get())->GetFeatures();
  ASSERT_EQ(static_cast<size_t>(0U), features.size());

  const auto& extensions = ToStub(engine.get())->GetDeviceExtensions();
  ASSERT_EQ(static_cast<size_t>(0U), extensions.size());

  EXPECT_EQ(12345U, ToStub(engine.get())->GetFenceTimeoutMs());
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
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto script = parser.GetScript();
  auto engine = MakeAndInitializeEngine(script->GetRequiredFeatures(),
                                        script->GetRequiredInstanceExtensions(),
                                        script->GetRequiredDeviceExtensions());

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());

  const auto& features = ToStub(engine.get())->GetFeatures();
  ASSERT_EQ(2U, features.size());
  EXPECT_EQ("robustBufferAccess", features[0]);
  EXPECT_EQ("logicOp", features[1]);

  const auto& extensions = ToStub(engine.get())->GetDeviceExtensions();
  ASSERT_EQ(2U, extensions.size());
  EXPECT_EQ("VK_KHR_storage_buffer_storage_class", extensions[0]);
  EXPECT_EQ("VK_KHR_variable_pointers", extensions[1]);

  EXPECT_EQ(12345U, ToStub(engine.get())->GetFenceTimeoutMs());
}

TEST_F(VkScriptExecutorTest, ClearCommand) {
  std::string input = R"(
[test]
clear)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());
  EXPECT_TRUE(ToStub(engine.get())->DidClearCommand());
}

TEST_F(VkScriptExecutorTest, ClearCommandFailure) {
  std::string input = R"(
[test]
clear)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailClearCommand();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("clear command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, ClearColorCommand) {
  std::string input = R"(
[test]
clear color 244 123 123 13)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
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
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailClearColorCommand();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("clear color command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, ClearDepthCommand) {
  std::string input = R"(
[test]
clear depth 24)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidClearDepthCommand());
}

TEST_F(VkScriptExecutorTest, ClearDepthCommandFailure) {
  std::string input = R"(
[test]
clear depth 24)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailClearDepthCommand();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("clear depth command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, ClearStencilCommand) {
  std::string input = R"(
[test]
clear stencil 24)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidClearStencilCommand());
}

TEST_F(VkScriptExecutorTest, ClearStencilCommandFailure) {
  std::string input = R"(
[test]
clear stencil 24)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailClearStencilCommand();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("clear stencil command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, DrawRectCommand) {
  std::string input = R"(
[test]
draw rect 2 4 10 20)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidDrawRectCommand());
}

TEST_F(VkScriptExecutorTest, DrawRectCommandFailure) {
  std::string input = R"(
[test]
draw rect 2 4 10 20)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailDrawRectCommand();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("draw rect command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, DrawArraysCommand) {
  std::string input = R"(
[test]
draw arrays TRIANGLE_LIST 0 0)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidDrawArraysCommand());
}

TEST_F(VkScriptExecutorTest, DrawArraysCommandFailure) {
  std::string input = R"(
[test]
draw arrays TRIANGLE_LIST 0 0)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailDrawArraysCommand();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("draw arrays command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, ComputeCommand) {
  std::string input = R"(
[test]
compute 2 3 4)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidComputeCommand());
}

TEST_F(VkScriptExecutorTest, ComputeCommandFailure) {
  std::string input = R"(
[test]
compute 2 3 4)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailComputeCommand();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("compute command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, EntryPointCommand) {
  std::string input = R"(
[test]
vertex entrypoint main)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidEntryPointCommand());
}

TEST_F(VkScriptExecutorTest, EntryPointCommandFailure) {
  std::string input = R"(
[test]
vertex entrypoint main)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailEntryPointCommand();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("entrypoint command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, PatchParameterVerticesCommand) {
  std::string input = R"(
[test]
patch parameter vertices 10)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidPatchParameterVerticesCommand());
}

TEST_F(VkScriptExecutorTest, PatchParameterVerticesCommandFailure) {
  std::string input = R"(
[test]
patch parameter vertices 10)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailPatchParameterVerticesCommand();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
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

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
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

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("probe command failed", r.Error());
}

TEST_F(VkScriptExecutorTest, BufferCommand) {
  std::string input = R"(
[test]
ssbo 0 24)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_TRUE(r.IsSuccess());
  ASSERT_TRUE(ToStub(engine.get())->DidBufferCommand());
}

TEST_F(VkScriptExecutorTest, BufferCommandFailure) {
  std::string input = R"(
[test]
ssbo 0 24)";

  Parser parser;
  parser.SkipValidationForTest();
  ASSERT_TRUE(parser.Parse(input).IsSuccess());

  auto engine = MakeEngine();
  ToStub(engine.get())->FailBufferCommand();
  auto script = parser.GetScript();

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
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

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
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

  Options options;
  Executor ex;
  Result r = ex.Execute(engine.get(), script.get(), ShaderMap(), &options);
  ASSERT_FALSE(r.IsSuccess());
  EXPECT_EQ("probe ssbo command failed", r.Error());
}

}  // namespace vkscript
}  // namespace amber
