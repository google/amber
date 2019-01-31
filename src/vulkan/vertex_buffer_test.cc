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
#include "src/vulkan/buffer.h"

namespace amber {
namespace vulkan {
namespace {

template <typename T>
void ExpectBitsEQ(const uint8_t* actual, T expected) {
  const T* ptr = reinterpret_cast<const T*>(actual);
  EXPECT_EQ(*ptr, expected);
}

const VkPhysicalDeviceMemoryProperties kMemoryProperties =
    VkPhysicalDeviceMemoryProperties();

class BufferForTest : public Buffer {
 public:
  BufferForTest(Device* device,
                size_t size_in_bytes,
                const VkPhysicalDeviceMemoryProperties& properties)
      : Buffer(device, size_in_bytes, properties) {
    memory_.resize(4096);
    memory_ptr_ = memory_.data();
  }
  ~BufferForTest() = default;

  void* HostAccessibleMemoryPtr() const override { return memory_ptr_; }

  Result CopyToDevice(VkCommandBuffer) override { return Result(); }

 private:
  std::vector<uint8_t> memory_;
  void* memory_ptr_ = nullptr;
};

class VertexBufferTest : public testing::Test {
 public:
  VertexBufferTest() {
    vertex_buffer_ = MakeUnique<VertexBuffer>(nullptr);

    std::unique_ptr<Buffer> buffer =
        MakeUnique<BufferForTest>(nullptr, 0U, kMemoryProperties);
    buffer_memory_ = buffer->HostAccessibleMemoryPtr();
    vertex_buffer_->SetBufferForTest(std::move(buffer));
  }

  ~VertexBufferTest() = default;

  Result SetData(uint8_t location,
                 const Format& format,
                 const std::vector<Value>& values) {
    vertex_buffer_->SetData(location, format, values);
    VkCommandBuffer null_cmd_buf = VK_NULL_HANDLE;
    return vertex_buffer_->SendVertexData(null_cmd_buf, kMemoryProperties);
  }

  const void* GetVkBufferPtr() const { return buffer_memory_; }

 private:
  std::unique_ptr<VertexBuffer> vertex_buffer_;
  const void* buffer_memory_ = nullptr;
};

}  // namespace

void VertexBuffer::SetBufferForTest(std::unique_ptr<Buffer> buffer) {
  buffer_ = std::move(buffer);
}

TEST_F(VertexBufferTest, R8G8B8A8_UINT) {
  std::vector<Value> values(4);
  values[0].SetIntValue(55);
  values[1].SetIntValue(3);
  values[2].SetIntValue(27);
  values[3].SetIntValue(255);

  Format format;
  format.SetFormatType(FormatType::kR8G8B8A8_UINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kUInt, 8);
  format.AddComponent(FormatComponentType::kG, FormatMode::kUInt, 8);
  format.AddComponent(FormatComponentType::kB, FormatMode::kUInt, 8);
  format.AddComponent(FormatComponentType::kA, FormatMode::kUInt, 8);

  Result r = SetData(0, format, values);
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

  Format format;
  format.SetFormatType(FormatType::kR16G16B16A16_UINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kUInt, 16);
  format.AddComponent(FormatComponentType::kG, FormatMode::kUInt, 16);
  format.AddComponent(FormatComponentType::kB, FormatMode::kUInt, 16);
  format.AddComponent(FormatComponentType::kA, FormatMode::kUInt, 16);

  Result r = SetData(0, format, values);
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

  Format format;
  format.SetFormatType(FormatType::kR32G32B32A32_UINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kUInt, 32);
  format.AddComponent(FormatComponentType::kG, FormatMode::kUInt, 32);
  format.AddComponent(FormatComponentType::kB, FormatMode::kUInt, 32);
  format.AddComponent(FormatComponentType::kA, FormatMode::kUInt, 32);

  Result r = SetData(0, format, values);
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

  Format format;
  format.SetFormatType(FormatType::kR64G64B64A64_UINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kUInt, 64);
  format.AddComponent(FormatComponentType::kG, FormatMode::kUInt, 64);
  format.AddComponent(FormatComponentType::kB, FormatMode::kUInt, 64);
  format.AddComponent(FormatComponentType::kA, FormatMode::kUInt, 64);

  Result r = SetData(0, format, values);
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

  Format format;
  format.SetFormatType(FormatType::kR8G8B8A8_SNORM);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSNorm, 8);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSNorm, 8);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSNorm, 8);
  format.AddComponent(FormatComponentType::kA, FormatMode::kSNorm, 8);

  Result r = SetData(0, format, values);
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

  Format format;
  format.SetFormatType(FormatType::kR16G16B16A16_SNORM);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSNorm, 16);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSNorm, 16);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSNorm, 16);
  format.AddComponent(FormatComponentType::kA, FormatMode::kSNorm, 16);

  Result r = SetData(0, format, values);
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

  Format format;
  format.SetFormatType(FormatType::kR32G32B32A32_SINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSInt, 32);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSInt, 32);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSInt, 32);
  format.AddComponent(FormatComponentType::kA, FormatMode::kSInt, 32);

  Result r = SetData(0, format, values);
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

  Format format;
  format.SetFormatType(FormatType::kR64G64B64A64_SINT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSInt, 64);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSInt, 64);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSInt, 64);
  format.AddComponent(FormatComponentType::kA, FormatMode::kSInt, 64);

  Result r = SetData(0, format, values);
  const int64_t* ptr = static_cast<const int64_t*>(GetVkBufferPtr());
  EXPECT_EQ(-55, ptr[0]);
  EXPECT_EQ(3, ptr[1]);
  EXPECT_EQ(-27, ptr[2]);
  EXPECT_EQ(255, ptr[3]);
}

TEST_F(VertexBufferTest, R16G11B10_SFLOAT) {
  std::vector<Value> values(3);

  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  uint64_t expected = 50688UL;
  values[0].SetDoubleValue(-6.0);

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  expected |= 1200UL << 16UL;
  values[1].SetDoubleValue(14.0);

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4
  //                                                 = 0.1171875
  expected |= 380UL << (16UL + 11UL);
  values[2].SetDoubleValue(0.1171875);

  Format format;
  format.SetFormatType(FormatType::kR8G8B8_UNORM);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 16);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 11);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 10);

  Result r = SetData(0, format, values);
  const uint64_t* ptr = static_cast<const uint64_t*>(GetVkBufferPtr());
  EXPECT_EQ(expected, *ptr);
}

TEST_F(VertexBufferTest, R10G16B11_SFLOAT) {
  std::vector<Value> values(3);

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4
  //                                                 = 0.1171875
  uint64_t expected = 380UL;
  values[0].SetDoubleValue(0.1171875);

  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  expected |= 50688UL << 10UL;
  values[1].SetDoubleValue(-6.0);

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  expected |= 1200UL << (16UL + 10UL);
  values[2].SetDoubleValue(14.0);

  Format format;
  format.SetFormatType(FormatType::kR8G8B8_UNORM);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 10);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 16);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 11);

  Result r = SetData(0, format, values);
  const uint64_t* ptr = static_cast<const uint64_t*>(GetVkBufferPtr());
  EXPECT_EQ(expected, *ptr);
}

TEST_F(VertexBufferTest, R11G16B10_SFLOAT) {
  std::vector<Value> values(3);

  // 11 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  18 /       48 -->   0 / 130 / 12582912 = 1.11(2) * 2^3 = 14
  uint64_t expected = 1200UL;
  values[0].SetDoubleValue(14.0);

  // 16 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     1 /  17 /      512 -->   1 / 129 /  4194304 = -1.1(2) * 2^2 = -6
  expected |= 50688UL << 11UL;
  values[1].SetDoubleValue(-6.0);

  // 10 bits float to float
  //   Sig / Exp / Mantissa     Sig / Exp / Mantissa
  //     0 /  11 /       28 -->   1 / 123 / 14680064 = 1.111(2) * 2^-4
  //                                                 = 0.1171875
  expected |= 380UL << (16UL + 11UL);
  values[2].SetDoubleValue(0.1171875);

  Format format;
  format.SetFormatType(FormatType::kR8G8B8_UNORM);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 11);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 16);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 10);

  Result r = SetData(0, format, values);
  const uint64_t* ptr = static_cast<const uint64_t*>(GetVkBufferPtr());
  EXPECT_EQ(expected, *ptr);
}

TEST_F(VertexBufferTest, R32G32B32_SFLOAT) {
  std::vector<Value> values(3);
  values[0].SetDoubleValue(-6.0);
  values[1].SetDoubleValue(14.0);
  values[2].SetDoubleValue(0.1171875);

  Format format;
  format.SetFormatType(FormatType::kR32G32B32A32_SFLOAT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 32);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 32);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 32);

  Result r = SetData(0, format, values);
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

  Format format;
  format.SetFormatType(FormatType::kR64G64B64A64_SFLOAT);
  format.AddComponent(FormatComponentType::kR, FormatMode::kSFloat, 64);
  format.AddComponent(FormatComponentType::kG, FormatMode::kSFloat, 64);
  format.AddComponent(FormatComponentType::kB, FormatMode::kSFloat, 64);

  Result r = SetData(0, format, values);
  const double* ptr = static_cast<const double*>(GetVkBufferPtr());
  EXPECT_DOUBLE_EQ(-6.0, ptr[0]);
  EXPECT_DOUBLE_EQ(14.0, ptr[1]);
  EXPECT_DOUBLE_EQ(0.1171875, ptr[2]);
}

}  // namespace vulkan
}  // namespace amber
