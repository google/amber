# Classes for Vulkan resources and descriptors
 * DRAFT

Vulkan has many resource and descriptor types.
Since it is complicated to manage them e.g.,
create/allocate/map/read/write/destory, we create several classes to
provide an abstraction. This document briefly explains those classes.


### Resource class
Represents a main resource i.e., VkBuffer or VkImage (See
[Resources in Vulkan spec](
https://www.khronos.org/registry/vulkan/specs/1.1/html/vkspec.html#resources))
in GPU device and an additional VkBuffer to allow read/write to the
main resource from CPU.

If the main resource is accessible from CPU, the additional
VkBuffer is not needed and it will be `VK_NULL_HANDLE`.
Otherwise, the additional VkBuffer has the same size with the main
resource and we must copy the main resource to the VkBuffer or
copy the VkBuffer to the main resource when reading from/write to
the main resource.

The Resource class has Buffer and Image sub-classes.

#### Buffer class
Abstracts VkBuffer and creates/allocates/maps/destorys
VkBuffer and VkBufferView resources.

#### Image class
Abstracts VkImage and creates/allocates/maps/destorys
VkImage and VkImageView resources.


### Sampler class
 * TODO: Not implementated yet
 * Represent [VkSampler](
   https://www.khronos.org/registry/vulkan/specs/1.1/html/vkspec.html#samplers)


### Descriptor class
Represents [Descriptor Types](
https://www.khronos.org/registry/vulkan/specs/1.1/html/vkspec.html#descriptorsets-types).
There are 11 Descriptor Types and they need different resources.
For example, a Combined Image Sampler needs both Image and
Sampler objects while Storage Buffer needs only a Buffer object.

* TODO: Describe 11 sub-classes of Descriptor for those Descriptor Types.


### FrameBuffer class
Abstracts [VkFrameBuffer](
https://www.khronos.org/registry/vulkan/specs/1.1/html/vkspec.html#_framebuffers)
for attachments and an Image for the FrameBuffer.
The usage of the Image is `VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT`.


### VertexBuffer class
Manages vertices data and a Buffer whose usage is
`VK_BUFFER_USAGE_VERTEX_BUFFER_BIT`.


### IndexBuffer class
 * TODO: Not implementated yet
 * Manages indices data and a Buffer whose usage is
   `VK_BUFFER_USAGE_INDEX_BUFFER_BIT`.
