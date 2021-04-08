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

#include "src/vulkan/vertex_buffer.h"

#include <utility>

#include "amber/value.h"
#include "gtest/gtest.h"
#include "src/format.h"
#include "src/make_unique.h"
#include "src/type_parser.h"
#include "src/vulkan/command_buffer.h"
#include "src/vulkan/command_pool.h"
#include "src/vulkan/device.h"

namespace amber {
namespace vulkan {
namespace {

class DummyDevice : public Device {
 public:
  DummyDevice()
      : Device(VkInstance(),
               VkPhysicalDevice(),
               0u,
               VkDevice(this),
               VkQueue()) {
    memory_.resize(64);
    dummyPtrs_.vkCreateBuffer = vkCreateBuffer;
    dummyPtrs_.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    dummyPtrs_.vkAllocateMemory = vkAllocateMemory;
    dummyPtrs_.vkBindBufferMemory = vkBindBufferMemory;
    dummyPtrs_.vkMapMemory = vkMapMemory;
    dummyPtrs_.vkCmdPipelineBarrier = vkCmdPipelineBarrier;
    dummyPtrs_.vkAllocateCommandBuffers = vkAllocateCommandBuffers;
    dummyPtrs_.vkCreateFence = vkCreateFence;
    dummyPtrs_.vkDestroyBufferView = vkDestroyBufferView;
    dummyPtrs_.vkFreeMemory = vkFreeMemory;
    dummyPtrs_.vkDestroyBuffer = vkDestroyBuffer;
  }
  ~DummyDevice() override {}

  const VulkanPtrs* GetPtrs() const override { return &dummyPtrs_; }

  bool HasMemoryFlags(uint32_t, const VkMemoryPropertyFlags) const override {
    return true;
  }

  void* GetMemoryPtr() { return memory_.data(); }

 private:
  VulkanPtrs dummyPtrs_;
  std::vector<uint8_t> memory_;

  static VkResult vkCreateBuffer(VkDevice,
                                 const VkBufferCreateInfo*,
                                 const VkAllocationCallbacks*,
                                 VkBuffer* pBuffer) {
    *pBuffer = VkBuffer(1);
    return VK_SUCCESS;
  }
  static void vkGetBufferMemoryRequirements(
      VkDevice,
      VkBuffer,
      VkMemoryRequirements* pMemoryRequirements) {
    pMemoryRequirements->alignment = 0;
    pMemoryRequirements->size = 0;
    pMemoryRequirements->memoryTypeBits = 0xffffffff;
  }
  static VkResult vkAllocateMemory(VkDevice,
                                   const VkMemoryAllocateInfo*,
                                   const VkAllocationCallbacks*,
                                   VkDeviceMemory*) {
    return VK_SUCCESS;
  }
  static VkResult vkBindBufferMemory(VkDevice,
                                     VkBuffer,
                                     VkDeviceMemory,
                                     VkDeviceSize) {
    return VK_SUCCESS;
  }
  static VkResult vkMapMemory(VkDevice device,
                              VkDeviceMemory,
                              VkDeviceSize,
                              VkDeviceSize,
                              VkMemoryMapFlags,
                              void** ppData) {
    DummyDevice* devicePtr = reinterpret_cast<DummyDevice*>(device);
    *ppData = devicePtr->GetMemoryPtr();
    return VK_SUCCESS;
  }
  static void vkCmdPipelineBarrier(VkCommandBuffer,
                                   VkPipelineStageFlags,
                                   VkPipelineStageFlags,
                                   VkDependencyFlags,
                                   uint32_t,
                                   const VkMemoryBarrier*,
                                   uint32_t,
                                   const VkBufferMemoryBarrier*,
                                   uint32_t,
                                   const VkImageMemoryBarrier*) {}
  static VkResult vkAllocateCommandBuffers(VkDevice,
                                           const VkCommandBufferAllocateInfo*,
                                           VkCommandBuffer*) {
    return VK_SUCCESS;
  }
  static VkResult vkCreateFence(VkDevice,
                                const VkFenceCreateInfo*,
                                const VkAllocationCallbacks*,
                                VkFence*) {
    return VK_SUCCESS;
  }
  static void vkDestroyBufferView(VkDevice,
                                  VkBufferView,
                                  const VkAllocationCallbacks*) {}
  static void vkFreeMemory(VkDevice,
                           VkDeviceMemory,
                           const VkAllocationCallbacks*) {}
  static void vkDestroyBuffer(VkDevice,
                              VkBuffer,
                              const VkAllocationCallbacks*) {}
};

class VertexBufferTest : public testing::Test {
 public:
  VertexBufferTest()
      : device_(MakeUnique<DummyDevice>()),
        commandPool_(MakeUnique<CommandPool>(device_.get())),
        commandBuffer_(
            MakeUnique<CommandBuffer>(device_.get(), commandPool_.get())),
        vertex_buffer_(MakeUnique<VertexBuffer>(device_.get())) {
    commandBuffer_->Initialize();
  }

  ~VertexBufferTest() override { vertex_buffer_.reset(); }

  Result SetData(uint8_t location, Format* format, std::vector<Value> values) {
    auto buffer = MakeUnique<Buffer>();
    buffer->SetFormat(format);
    buffer->SetData(std::move(values));

    vertex_buffer_->SetData(location, buffer.get(), InputRate::kVertex, format,
                            0, format->SizeInBytes());
    return vertex_buffer_->SendVertexData(commandBuffer_.get());
  }

  const void* GetVkBufferPtr() { return device_->GetMemoryPtr(); }

 private:
  std::unique_ptr<DummyDevice> device_;
  std::unique_ptr<CommandPool> commandPool_;
  std::unique_ptr<CommandBuffer> commandBuffer_;
  std::unique_ptr<VertexBuffer> vertex_buffer_;
};

}  // namespace

TEST_F(VertexBufferTest, R8G8B8A8_UINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(55);
  values[1].SetIntValue(3);
  values[2].SetIntValue(27);
  values[3].SetIntValue(255);

  TypeParser parser;
  auto type = parser.Parse("R8G8B8A8_UINT");
  Format fmt(type.get());
  Result r = SetData(0, &fmt, values);

  const uint8_t* ptr = static_cast<const uint8_t*>(GetVkBufferPtr());
  EXPECT_EQ(55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R16G16B16A16_UINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(55);
  values[1].SetIntValue(3);
  values[2].SetIntValue(27);
  values[3].SetIntValue(255);

  TypeParser parser;
  auto type = parser.Parse("R16G16B16A16_UINT");
  Format fmt(type.get());
  Result r = SetData(0, &fmt, values);

  const uint16_t* ptr = static_cast<const uint16_t*>(GetVkBufferPtr());
  EXPECT_EQ(55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R32G32B32A32_UINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(55);
  values[1].SetIntValue(3);
  values[2].SetIntValue(27);
  values[3].SetIntValue(255);

  TypeParser parser;
  auto type = parser.Parse("R32G32B32A32_UINT");
  Format fmt(type.get());
  Result r = SetData(0, &fmt, values);

  const uint32_t* ptr = static_cast<const uint32_t*>(GetVkBufferPtr());
  EXPECT_EQ(55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R64G64B64A64_UINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(55);
  values[1].SetIntValue(3);
  values[2].SetIntValue(27);
  values[3].SetIntValue(255);

  TypeParser parser;
  auto type = parser.Parse("R64G64B64A64_UINT");
  Format fmt(type.get());
  Result r = SetData(0, &fmt, values);

  const uint64_t* ptr = static_cast<const uint64_t*>(GetVkBufferPtr());
  EXPECT_EQ(55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R8G8B8A8_SNORM) {
  std::vector<Value> values(4);
  values[0].SetIntValue(static_cast<uint64_t>(-55));
  values[1].SetIntValue(3);
  values[2].SetIntValue(static_cast<uint64_t>(-128));
  values[3].SetIntValue(127);

  TypeParser parser;
  auto type = parser.Parse("R8G8B8A8_SNORM");
  Format fmt(type.get());
  Result r = SetData(0, &fmt, values);
  const int8_t* ptr = static_cast<const int8_t*>(GetVkBufferPtr());

  EXPECT_EQ(-55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(-128, ptr[2]);
  EXPECT_EQ(127, ptr[3]);
}

TEST_F(VertexBufferTest, R16G16B16A16_SNORM) {
  std::vector<Value> values(4);
  values[0].SetIntValue(static_cast<uint64_t>(-55));
  values[1].SetIntValue(3);
  values[2].SetIntValue(static_cast<uint64_t>(-27));
  values[3].SetIntValue(255);

  TypeParser parser;
  auto type = parser.Parse("R16G16B16A16_SNORM");
  Format fmt(type.get());
  Result r = SetData(0, &fmt, values);

  const int16_t* ptr = static_cast<const int16_t*>(GetVkBufferPtr());
  EXPECT_EQ(-55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(-27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R32G32B32A32_SINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(static_cast<uint64_t>(-55));
  values[1].SetIntValue(3);
  values[2].SetIntValue(static_cast<uint64_t>(-27));
  values[3].SetIntValue(255);

  TypeParser parser;
  auto type = parser.Parse("R32G32B32A32_SINT");
  Format fmt(type.get());
  Result r = SetData(0, &fmt, values);

  const int32_t* ptr = static_cast<const int32_t*>(GetVkBufferPtr());
  EXPECT_EQ(-55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(-27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R64G64B64A64_SINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(static_cast<uint64_t>(-55));
  values[1].SetIntValue(3);
  values[2].SetIntValue(static_cast<uint64_t>(-27));
  values[3].SetIntValue(255);

  TypeParser parser;
  auto type = parser.Parse("R64G64B64A64_SINT");
  Format fmt(type.get());
  Result r = SetData(0, &fmt, values);

  const int64_t* ptr = static_cast<const int64_t*>(GetVkBufferPtr());
  EXPECT_EQ(-55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(-27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R32G32B32_SFLOAT) {
  std::vector<Value> values(3);
  values[0].SetDoubleValue(-6.0);
  values[1].SetDoubleValue(14.0);
  values[2].SetDoubleValue(0.1171875);

  TypeParser parser;
  auto type = parser.Parse("R32G32B32_SFLOAT");
  Format fmt(type.get());
  Result r = SetData(0, &fmt, values);

  const float* ptr = static_cast<const float*>(GetVkBufferPtr());
  EXPECT_FLOAT_EQ(-6.0f, ptr[0]);
  EXPECT_FLOAT_EQ(14.0f, ptr[1]);
  EXPECT_FLOAT_EQ(0.1171875f, ptr[2]);
}

TEST_F(VertexBufferTest, R64G64B64_SFLOAT) {
  std::vector<Value> values(3);
  values[0].SetDoubleValue(-6.0);
  values[1].SetDoubleValue(14.0);
  values[2].SetDoubleValue(0.1171875);

  TypeParser parser;
  auto type = parser.Parse("R64G64B64_SFLOAT");
  Format fmt(type.get());
  Result r = SetData(0, &fmt, values);

  const double* ptr = static_cast<const double*>(GetVkBufferPtr());
  EXPECT_DOUBLE_EQ(-6.0, ptr[0]);
  EXPECT_DOUBLE_EQ(14.0, ptr[1]);
  EXPECT_DOUBLE_EQ(0.1171875, ptr[2]);
}

}  // namespace vulkan
}  // namespace amber
