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

#ifndef SRC_VULKAN_TLAS_DESCRIPTOR_H_
#define SRC_VULKAN_TLAS_DESCRIPTOR_H_

#include <memory>
#include <vector>

#include "src/vulkan/descriptor.h"
#include "src/vulkan/tlas.h"
#include "src/vulkan/transfer_image.h"

namespace amber {
namespace vulkan {

class TLASDescriptor : public Descriptor {
 public:
  TLASDescriptor(amber::TLAS* tlas,
                 DescriptorType type,
                 Device* device,
                 BlasesMap* blases,
                 TlasesMap* tlases,
                 uint32_t desc_set,
                 uint32_t binding);
  ~TLASDescriptor() override;

  void UpdateDescriptorSetIfNeeded(VkDescriptorSet descriptor_set) override;

  Result CreateResourceIfNeeded() override;

  void AddAmberTLAS(amber::TLAS* tlas) { amber_tlases_.push_back(tlas); }
  uint32_t GetDescriptorCount() override {
    return static_cast<uint32_t>(amber_tlases_.size());
  }
  TLASDescriptor* AsTLASDescriptor() override { return this; }

 private:
  std::vector<amber::TLAS*> amber_tlases_;
  BlasesMap* blases_;
  TlasesMap* tlases_;
};

}  // namespace vulkan
}  // namespace amber

#endif  // SRC_VULKAN_TLAS_DESCRIPTOR_H_
