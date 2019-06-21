// Structure with one constructor per-type of bindings, so that the
// initializer_list accepts bindings with the right type and no extra
// information.
struct BindingInitializationHelper {
  BindingInitializationHelper(uint32_t binding,
                              const ::dawn::Buffer& buffer,
                              uint64_t offset,
                              uint64_t size)
      : binding(binding), buffer(buffer), offset(offset), size(size) {}

  ::dawn::BindGroupBinding GetAsBinding() const {
    ::dawn::BindGroupBinding result;
    result.binding = binding;
    result.sampler = sampler;
    result.textureView = textureView;
    result.buffer = buffer;
    result.offset = offset;
    result.size = size;
    return result;
  }

  uint32_t binding;
  ::dawn::Sampler sampler;
  ::dawn::TextureView textureView;
  ::dawn::Buffer buffer;
  uint64_t offset = 0;
  uint64_t size = 0;
};
