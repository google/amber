// Copyright 2019 The Amber Authors.
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

#ifndef SRC_VULKAN_PUSH_CONSTANT_H_
#define SRC_VULKAN_PUSH_CONSTANT_H_

#include <memory>
#include <vector>

#include "amber/result.h"
#include "amber/vulkan_header.h"
#include "src/command.h"
#include "src/vulkan/resource.h"

namespace amber {
namespace vulkan {

class CommandBuffer;
class Device;

/// Class to handle push constants.
class PushConstant {
 public:
  explicit PushConstant(Device* device);
  ~PushConstant();

  /// Retrieves a `VkPushConstantRange` class describing our push constant
  /// requirements.
  VkPushConstantRange GetVkPushConstantRange();

  Result RecordPushConstantVkCommand(CommandBuffer* command,
                                     VkPipelineLayout pipeline_layout);

  /// Adds data into the push constant buffer.
  Result AddBufferData(const BufferCommand* command);

 private:
  Result UpdateMemoryWithInput(const BufferInput& input);

  Device* device_;

  /// Keeps the information of what and how to conduct push constant.
  /// These are applied from lowest index to highest index, so that
  /// if address ranges overlap, then the later values take effect.
  std::vector<BufferInput> push_constant_data_;
  std::vector<uint8_t> memory_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_PUSH_CONSTANT_H_
