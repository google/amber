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
#include <string>
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
class VertexDataNode;

class Node {
 public:
  virtual ~Node();

  bool IsIndices() const { return node_type_ == NodeType::kIndices; }
  bool IsRequire() const { return node_type_ == NodeType::kRequire; }
  bool IsVertexData() const { return node_type_ == NodeType::kVertexData; }

  IndicesNode* AsIndices();
  RequireNode* AsRequire();
  VertexDataNode* AsVertexData();

 protected:
  explicit Node(NodeType type);

 private:
  NodeType node_type_;
};

class RequireNode : public Node {
 public:
  class Requirement {
   public:
    explicit Requirement(Feature feature);
    Requirement(Feature feature, std::unique_ptr<Format> format);
    Requirement(Feature feature, uint32_t value);
    Requirement(Requirement&&);
    ~Requirement();

    Feature GetFeature() const { return feature_; }
    const Format* GetFormat() const { return format_.get(); }
    uint32_t GetUint32Value() const { return uint32_value_; }

   private:
    Feature feature_;
    std::unique_ptr<Format> format_;
    uint32_t uint32_value_;
  };

  RequireNode();
  ~RequireNode() override;

  void AddRequirement(Feature feature);
  void AddRequirement(Feature feature, std::unique_ptr<Format> format);
  void AddRequirement(Feature feature, uint32_t value);

  const std::vector<Requirement>& Requirements() const { return requirements_; }

  void AddExtension(const std::string& ext) { extensions_.push_back(ext); }
  const std::vector<std::string>& Extensions() const { return extensions_; }

 private:
  std::vector<Requirement> requirements_;
  std::vector<std::string> extensions_;
};

class IndicesNode : public Node {
 public:
  explicit IndicesNode(std::unique_ptr<Buffer> buffer);
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

  VertexDataNode();
  ~VertexDataNode() override;

  void SetSegment(Header&& header, std::vector<Value>&& data);
  size_t SegmentCount() const { return data_.size(); }

  const Header& GetHeader(size_t idx) const { return data_[idx].header; }
  const std::vector<Value>& GetSegment(size_t idx) const {
    return data_[idx].buffer;
  }

 private:
  struct NodeData {
    Header header;
    std::vector<Value> buffer;
  };
  std::vector<NodeData> data_;
};

}  // namespace vkscript
}  // namespace amber

#endif  // SRC_VKSCRIPT_NODES_H_
