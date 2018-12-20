This is copied from #193

### Explicit memory layout

We can provide `ssbo` or `ubo` commands with explicit memory
layout information. For example,
`ssbo std140 0:0 subdata float 0     1 2 3`.

However, as I gave an example [here](
https://github.com/Igalia/vkrunner/issues/46#issuecomment-449036491),
this design gives lots of suprises to users.

### How Amber script must be designed

**Providing no memory layout** is much more clear to users.

Buffer setup commands of Amber script **must** have semantics to
**simply copy given data** to the buffer and it must be independent
with GLSL shader semantics.

### Mismatch between VkRunner script and Amber script

As debated [here](
https://github.com/Igalia/vkrunner/issues/46#issuecomment-449039999),
VkRunner script will support explicit memory layout.

I think changing Vulkan engine for that is inefficient and requires
too much complication in engine implementation.
We should interpret it just before calling Vulkan engine i.e.,
executor layer.

For example,
```
layout std430
ssbo 0:0 subdata float 0 1 2 3
```
We can do what we do now for this VkRunner script.

However, for the following VkRunner script,
```
layout std140
ssbo 0:0 subdata float 0 1 2 3
```
we should call the following function of engine
```
DoBuffer (
  type: ssbo,
  descriptor set: 0,
  binding: 0,
  data type: float,
  offset: 0,
  data: [ 1, 0, 0, 0, 2, 0, 0, 0, 3 ]
)
```
