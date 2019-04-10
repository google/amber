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

#ifndef SRC_BUFFER_H_
#define SRC_BUFFER_H_

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "amber/result.h"
#include "amber/value.h"
#include "src/datum_type.h"
#include "src/format.h"

namespace amber {

class DataBuffer;
class FormatBuffer;

/// Types of buffers which can be created.
enum class BufferType : int8_t {
  /// Unknown buffer type
  kUnknown = -1,
  /// A color buffer.
  kColor = 0,
  /// A depth/stencil buffer.
  kDepth,
  /// An index buffer.
  kIndex,
  /// A sampled buffer.
  kSampled,
  /// A storage buffer.
  kStorage,
  /// A uniform buffer.
  kUniform,
  /// A push constant buffer.
  kPushConstant,
  /// A vertex buffer.
  kVertex
};

/// A buffer stores data. The buffer maybe provided from the input script, or
/// maybe created as needed. A buffer must have a unique name.
class Buffer {
 public:
  /// Create a buffer of |type_|.
  explicit Buffer(BufferType type);

  virtual ~Buffer();

  /// Returns |true| if this is a buffer described by a |DatumType|.
  virtual bool IsDataBuffer() const { return false; }
  /// Returns |true| if this is a buffer described by a |Format|.
  virtual bool IsFormatBuffer() const { return false; }

  virtual Format* GetFormat() = 0;

  /// Converts the buffer to a |DataBuffer|. Note, |IsDataBuffer| must be true
  /// for this method to be used.
  DataBuffer* AsDataBuffer();
  /// Converts the buffer to a |FormatBuffer|. Note, |IsFormatBuffer| must be
  /// true for this method to be used.
  FormatBuffer* AsFormatBuffer();

  /// Returns the BufferType of this buffer.
  BufferType GetBufferType() const { return buffer_type_; }
  void SetBufferType(BufferType type) { buffer_type_ = type; }

  /// Set the location binding value for the buffer.
  void SetLocation(uint8_t loc) { location_ = loc; }
  /// Get the location binding value for the buffer.
  uint8_t GetLocation() const { return location_; }

  /// Sets the buffer |name|.
  void SetName(const std::string& name) { name_ = name; }
  /// Returns the name of the buffer.
  std::string GetName() const { return name_; }

  /// Sets the number of items in the buffer.
  void SetSize(uint32_t size) { size_ = size; }

  /// Returns the number of items in the buffer.
  uint32_t GetSize() const { return size_; }

  uint32_t GetWidth() const { return width_; }
  void SetWidth(uint32_t width) { width_ = width; }
  uint32_t GetHeight() const { return height_; }
  void SetHeight(uint32_t height) { height_ = height; }

  /// Returns the number of bytes needed for the data in the buffer.
  virtual uint32_t GetSizeInBytes() const { return size_; }

  /// Sets the data into the buffer. The size will also be updated to be the
  /// size of the data provided.
  virtual Result SetData(std::vector<Value>&& data) = 0;

  std::vector<uint8_t>* ValuePtr() { return &values_; }
  const std::vector<uint8_t>* ValuePtr() const { return &values_; }

  template <typename T>
  const T* GetValues() const {
    return reinterpret_cast<const T*>(values_.data());
  }

  /// Copies the buffer values to an other one
  Result CopyTo(Buffer* buffer) const;

  /// Succeeds only if both buffer contents are equal
  Result IsEqual(Buffer* buffer) const;

 protected:
  /// Creates an un-typed buffer.
  Buffer();

  std::vector<uint8_t> values_;

 private:
  BufferType buffer_type_ = BufferType::kUnknown;
  std::string name_;
  uint32_t size_ = 0;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  uint8_t location_ = 0;
};

/// A buffer class where the data is described by a |DatumType| object.
class DataBuffer : public Buffer {
 public:
  DataBuffer();
  explicit DataBuffer(BufferType type);
  ~DataBuffer() override;

  // Buffer
  bool IsDataBuffer() const override { return true; }
  uint32_t GetSizeInBytes() const override {
    return GetSize() * datum_type_.SizeInBytes();
  }
  Result SetData(std::vector<Value>&& data) override;
  Format* GetFormat() override {
    if (format_)
      return format_.get();

    auto fmt = datum_type_.AsFormat();
    format_.swap(fmt);
    return format_.get();
  }

  /// Sets the DatumType of the buffer to |type|.
  void SetDatumType(const DatumType& type) {
    datum_type_ = type;
    format_ = datum_type_.AsFormat();
  }
  /// Returns the DatumType describing the buffer data.
  const DatumType GetDatumType() const { return datum_type_; }

 private:
  Result CopyData(const std::vector<Value>& data);

  std::unique_ptr<Format> format_;
  DatumType datum_type_;
};

/// A buffer class where the data is described by a |format| object.
class FormatBuffer : public Buffer {
 public:
  FormatBuffer();
  explicit FormatBuffer(BufferType type);
  ~FormatBuffer() override;

  // Buffer
  bool IsFormatBuffer() const override { return true; }
  uint32_t GetSizeInBytes() const override {
    return GetSize() * format_->SizeInBytes();
  }
  Result SetData(std::vector<Value>&& data) override;

  /// Sets the Format of the buffer to |format|.
  void SetFormat(std::unique_ptr<Format> format) {
    format_ = std::move(format);
  }
  /// Returns the Format describing the buffer data.
  Format* GetFormat() override { return format_.get(); }

  uint32_t GetTexelStride() { return format_->SizeInBytes(); }

  // When copying the image to the host buffer, we specify a row length of 0
  // which results in tight packing of rows.  So the row stride is the product
  // of the texel stride and the number of texels in a row.
  uint32_t GetRowStride() { return GetTexelStride() * GetWidth(); }

 private:
  Result CopyData(const std::vector<Value>& data);

  std::unique_ptr<Format> format_;
};

}  // namespace amber

#endif  // SRC_BUFFER_H_
