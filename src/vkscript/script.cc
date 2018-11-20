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

#include "src/vkscript/script.h"

#include "src/make_unique.h"
#include "src/vkscript/nodes.h"

namespace amber {

const vkscript::Script* ToVkScript(const amber::Script* s) {
  return static_cast<const vkscript::Script*>(s);
}

namespace vkscript {

Script::Script() : amber::Script(ScriptType::kVkScript) {}

Script::~Script() = default;

void Script::AddRequireNode(std::unique_ptr<RequireNode> node) {
  std::unique_ptr<Node> tn(node.release());
  test_nodes_.push_back(std::move(tn));
}

void Script::AddShader(ShaderType type, std::vector<uint32_t> shader) {
  test_nodes_.push_back(MakeUnique<ShaderNode>(type, std::move(shader)));
}

void Script::AddIndexBuffer(std::unique_ptr<Buffer> buffer) {
  test_nodes_.push_back(MakeUnique<IndicesNode>(std::move(buffer)));
}

void Script::AddVertexData(std::unique_ptr<VertexDataNode> node) {
  std::unique_ptr<Node> tn(node.release());
  test_nodes_.push_back(std::move(tn));
}

void Script::SetTestCommands(std::vector<std::unique_ptr<Command>> cmds) {
  test_nodes_.push_back(MakeUnique<TestNode>(std::move(cmds)));
}

}  // namespace vkscript
}  // namespace amber
