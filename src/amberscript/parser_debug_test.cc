// Copyright 2020 The Amber Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or parseried.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <sstream>

#include "gtest/gtest.h"
#include "src/amberscript/parser.h"
#include "src/shader_data.h"

namespace amber {
namespace amberscript {

namespace {
class ThreadEventRecorder : public debug::Thread {
  std::stringstream& events;
  std::string indent = "  ";

 public:
  explicit ThreadEventRecorder(std::stringstream& ev) : events(ev) {}

  void StepOver() override { events << indent << "STEP_OVER" << std::endl; }
  void StepIn() override { events << indent << "STEP_IN" << std::endl; }
  void StepOut() override { events << indent << "STEP_OUT" << std::endl; }
  void Continue() override { events << indent << "CONTINUE" << std::endl; }
  void ExpectLocation(const debug::Location& location,
                      const std::string& line) override {
    events << indent << "EXPECT LOCATION \"" << location.file << "\" "
           << location.line;
    if (!line.empty()) {
      events << " \"" << line << "\"";
    }
    events << std::endl;
  }
  void ExpectCallstack(
      const std::vector<debug::StackFrame>& callstack) override {
    events << indent << "EXPECT CALLSTACK";
    for (auto& frame : callstack) {
      events << indent << "  " << frame.name << " " << frame.location.file
             << ":" << frame.location.line << " " << std::endl;
    }
    events << std::endl;
  }
  void ExpectLocal(const std::string& name, int64_t value) override {
    events << indent << "EXPECT LOCAL \"" << name << "\" EQ " << value
           << std::endl;
  }
  void ExpectLocal(const std::string& name, double value) override {
    events << indent << "EXPECT LOCAL \"" << name << "\" EQ " << value
           << std::endl;
  }
  void ExpectLocal(const std::string& name, const std::string& value) override {
    events << indent << "EXPECT LOCAL \"" << name << "\" EQ \"" << value << "\""
           << std::endl;
  }
};

class EventRecorder : public debug::Events {
 public:
  std::stringstream events;

  void record(const std::shared_ptr<const debug::ThreadScript>& script) {
    ThreadEventRecorder thread{events};
    script->Run(&thread);
  }
  void BreakOnComputeGlobalInvocation(
      uint32_t x,
      uint32_t y,
      uint32_t z,
      const std::shared_ptr<const debug::ThreadScript>& script) override {
    events << "THREAD GLOBAL_INVOCATION_ID " << x << " " << y << " " << z
           << std::endl;
    record(script);
    events << "END" << std::endl;
  }
  void BreakOnVertexIndex(
      uint32_t index,
      const std::shared_ptr<const debug::ThreadScript>& script) override {
    events << "THREAD VERTEX_INDEX " << index << std::endl;
    record(script);
    events << "END" << std::endl;
  }
  void BreakOnFragmentWindowSpacePosition(
      uint32_t x,
      uint32_t y,
      const std::shared_ptr<const debug::ThreadScript>& script) override {
    events << "THREAD FRAGMENT_WINDOW_SPACE_POSITION " << x << " " << y
           << std::endl;
    record(script);
    events << "END" << std::endl;
  }
};
}  // namespace

using AmberScriptParserTest = testing::Test;

TEST_F(AmberScriptParserTest, DebugEventsScript) {
  std::string dbg = R"(THREAD GLOBAL_INVOCATION_ID 1 2 3
  EXPECT LOCATION "compute.hlsl" 2
  STEP_IN
  EXPECT LOCAL "one" EQ 1
  STEP_OUT
  EXPECT LOCAL "pi" EQ 3.14
  STEP_OVER
  EXPECT LOCAL "cat" EQ "meow"
  CONTINUE
END
THREAD VERTEX_INDEX 2
  EXPECT LOCATION "vertex.hlsl" 2 "  dog:woof cat:meow duck:quack"
END
THREAD FRAGMENT_WINDOW_SPACE_POSITION 4 5
  EXPECT LOCATION "fragment.hlsl" 42
  CONTINUE
END
)";

  std::string in = R"(
SHADER compute dbg_compute GLSL
void main() {}
END

PIPELINE compute my_pipeline
  ATTACH dbg_compute
END

DEBUG my_pipeline 2 4 5
)" + dbg + "END";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());

  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsCompute());
  auto* compute = cmd->AsCompute();
  EXPECT_EQ(2U, compute->GetX());
  EXPECT_EQ(4U, compute->GetY());
  EXPECT_EQ(5U, compute->GetZ());

  EventRecorder event_recorder;
  compute->GetDebugScript()->Run(&event_recorder);
  EXPECT_EQ(dbg, event_recorder.events.str());

  auto& shaders = compute->GetPipeline()->GetShaders();
  ASSERT_EQ(1U, shaders.size());

  EXPECT_EQ(true, shaders[0].GetEmitDebugInfo());
}

TEST_F(AmberScriptParserTest, DebugEmitDebugInfoVertex) {
  std::string dbg = R"()";

  std::string in = R"(
SHADER vertex dbg_vertex GLSL
void main() {}
END

SHADER fragment dbg_fragment GLSL
void main() {}
END

BUFFER position_buf DATA_TYPE R8G8_SNORM DATA
 1 1 2 2 3 3
END

PIPELINE graphics my_pipeline
  ATTACH dbg_vertex
  ATTACH dbg_fragment
  VERTEX_DATA position_buf LOCATION 0
END

DEBUG my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 0 COUNT 1
  THREAD VERTEX_INDEX 100
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());
  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsDrawArrays());
  auto* draw = cmd->AsDrawArrays();

  for (auto& shader : draw->GetPipeline()->GetShaders()) {
    bool expect_debug_info = shader.GetShaderType() == kShaderTypeVertex;
    EXPECT_EQ(expect_debug_info, shader.GetEmitDebugInfo())
        << "Emit debug info for shader type " << shader.GetShaderType();
  }
}

TEST_F(AmberScriptParserTest, DebugEmitDebugInfoFragment) {
  std::string dbg = R"()";

  std::string in = R"(
SHADER vertex dbg_vertex GLSL
void main() {}
END

SHADER fragment dbg_fragment GLSL
void main() {}
END

BUFFER position_buf DATA_TYPE R8G8_SNORM DATA
 1 1 2 2 3 3
END

PIPELINE graphics my_pipeline
  ATTACH dbg_vertex
  ATTACH dbg_fragment
  VERTEX_DATA position_buf LOCATION 0
END

DEBUG my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 0 COUNT 1
  THREAD FRAGMENT_WINDOW_SPACE_POSITION 1 2
  END
END)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());
  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsDrawArrays());
  auto* draw = cmd->AsDrawArrays();

  for (auto& shader : draw->GetPipeline()->GetShaders()) {
    bool expect_debug_info = shader.GetShaderType() == kShaderTypeFragment;
    EXPECT_EQ(expect_debug_info, shader.GetEmitDebugInfo())
        << "Emit debug info for shader type " << shader.GetShaderType();
  }
}

TEST_F(AmberScriptParserTest, DebugEmitNoDebugInfo) {
  std::string dbg = R"()";

  std::string in = R"(
SHADER vertex dbg_vertex GLSL
void main() {}
END

SHADER fragment dbg_fragment GLSL
void main() {}
END

BUFFER position_buf DATA_TYPE R8G8_SNORM DATA
 1 1 2 2 3 3
END

PIPELINE graphics my_pipeline
  ATTACH dbg_vertex
  ATTACH dbg_fragment
  VERTEX_DATA position_buf LOCATION 0
END

RUN my_pipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 0 COUNT 1
)";

  Parser parser;
  Result r = parser.Parse(in);
  ASSERT_TRUE(r.IsSuccess()) << r.Error();

  auto script = parser.GetScript();
  const auto& commands = script->GetCommands();
  ASSERT_EQ(1U, commands.size());
  auto* cmd = commands[0].get();
  ASSERT_TRUE(cmd->IsDrawArrays());
  auto* draw = cmd->AsDrawArrays();

  for (auto& shader : draw->GetPipeline()->GetShaders()) {
    EXPECT_EQ(false, shader.GetEmitDebugInfo());
  }
}

}  // namespace amberscript
}  // namespace amber
