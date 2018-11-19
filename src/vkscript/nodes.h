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

#ifndef SRC_VKSCRIPT_NODES_H_
#define SRC_VKSCRIPT_NODES_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "src/buffer.h"
#include "src/command.h"
#include "src/feature.h"
#include "src/format.h"
#include "src/tokenizer.h"
#include "src/vkscript/section_parser.h"

namespace amber {
namespace vkscript {

class IndicesNode;
class RequireNode;
class ShaderNode;
class TestNode;
class VertexDataNode;

class Node {
 public:
  virtual ~Node();

  bool IsIndices() const { return node_type_ == NodeType::kIndices; }
  bool IsRequire() const { return node_type_ == NodeType::kRequire; }
  bool IsShader() const { return node_type_ == NodeType::kShader; }
  bool IsTest() const { return node_type_ == NodeType::kTest; }
  bool IsVertexData() const { return node_type_ == NodeType::kVertexData; }

  IndicesNode* AsIndices();
  RequireNode* AsRequire();
  ShaderNode* AsShader();
  TestNode* AsTest();
  VertexDataNode* AsVertexData();

 protected:
  Node(NodeType type);

 private:
  NodeType node_type_;
};

class ShaderNode : public Node {
 public:
  ShaderNode(ShaderType type, std::vector<uint32_t> shader);
  ~ShaderNode() override;

  ShaderType GetShaderType() const { return type_; }
  const std::vector<uint32_t>& GetData() const { return shader_; }

 private:
  ShaderType type_;
  std::vector<uint32_t> shader_;
};

class RequireNode : public Node {
 public:
  class Requirement {
   public:
    Requirement(Feature feature);
    Requirement(Feature feature, std::unique_ptr<Format> format);
    Requirement(Requirement&&);
    ~Requirement();

    Feature GetFeature() const { return feature_; }
    const Format* GetFormat() const { return format_.get(); }

   private:
    Feature feature_;
    std::unique_ptr<Format> format_;
  };

  RequireNode();
  ~RequireNode() override;

  void AddRequirement(Feature feature);
  void AddRequirement(Feature feature, std::unique_ptr<Format> format);

  const std::vector<Requirement>& Requirements() const { return requirements_; }

  void AddExtension(const std::string& ext) { extensions_.push_back(ext); }
  const std::vector<std::string>& Extensions() const { return extensions_; }

 private:
  std::vector<Requirement> requirements_;
  std::vector<std::string> extensions_;
};

class IndicesNode : public Node {
 public:
  IndicesNode(std::unique_ptr<Buffer> buffer);
  ~IndicesNode() override;

  const std::vector<Value>& Indices() const { return buffer_->GetData(); }

 private:
  std::unique_ptr<Buffer> buffer_;
};

class VertexDataNode : public Node {
 public:
  struct Header {
    uint8_t location;
    std::unique_ptr<Format> format;
  };

  class Cell {
   public:
    Cell();
    Cell(const Cell&);
    ~Cell();

    size_t size() const { return data_.size(); }
    void AppendValue(Value&& v) { data_.emplace_back(std::move(v)); }
    const Value& GetValue(size_t idx) const { return data_[idx]; }

   private:
    std::vector<Value> data_;
  };

  VertexDataNode();
  ~VertexDataNode() override;

  const std::vector<Header>& GetHeaders() const { return headers_; }
  void SetHeaders(std::vector<Header> headers) {
    headers_ = std::move(headers);
  }

  void AddRow(std::vector<Cell> row) { rows_.push_back(std::move(row)); }

  const std::vector<std::vector<Cell>>& GetRows() const { return rows_; }

 private:
  std::vector<Header> headers_;
  std::vector<std::vector<Cell>> rows_;
};

class TestNode : public Node {
 public:
  TestNode(std::vector<std::unique_ptr<Command>> cmds);
  ~TestNode() override;

  const std::vector<std::unique_ptr<Command>>& GetCommands() const {
    return commands_;
  }

 private:
  std::vector<std::unique_ptr<Command>> commands_;
};

}  // namespace vkscript
}  // namespace amber

#endif  // SRC_VKSCRIPT_NODES_H_
