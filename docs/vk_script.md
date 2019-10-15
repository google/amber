# VkScript

The VkScript format is a clone of the format used by VkRunner as described in
[1].

# General
## Comments
The # symbol can be used to start a comment which extends to the end of the
line.

## Continuations
The \ can be used at the end of a line to signal a continuation, the
new line will be skipped and parsing will treat the following line as a
continuation of the current line.

## Descriptor Sets and Bindings
Any command below which accepts a binding will accept either a single integer
value which will have a descriptor set of 0 and a binding of the value give or
a string can be provided of the form _set integer_:_binding integer_ in which
case the descriptor set value will be `set` and the binding value will be
`binding`.

# Sections

The format is broken down into five main sections:
 * `require`
 * `shaders`
 * `indices`
 * `vertex data`
 * `test`

## Require
The `require` section lists all of the requirements for the testing environment.
There are four types of information that can be encoded in the require section.

The _feature_ list contains a list of features that are required in
order for the test to execute. If a feature is missing an error will be reported
and the test will fail. The _features_ are listed below in the
*Available Require Features* section.

The _framebuffer_ and _depthstencil_ commands allow setting the format for the
given buffer. The valid values are listed below in the *Image Formats*
section.

The _fbsize_ command allows setting the width and height of the framebuffer.

The _fence\_timeout_ option allows setting an integer number of milliseconds
for any fence timeouts.

The last option is _extensions_. Any string which isn't listed above is assumed
to be an _extension_. The extensions must be of the format [a-zA-Z0-9_]+. If the
device extension is not available we will report it is not available and the
test will continue.


#### Require Examples

```
[require]
independentBlend
VK_KHR_storage_buffer_storage_class
```

## Shaders
The shader section allows you to specify the content of the shaders under test.
This can be done as GLSL, SPIRV-ASM or SPIRV-Hex depending on how the shader is
formatted. There is also a special *passthrough* vertex shader which can be
used which just passes the vec4 input location 0 through to the `gl_Position`.
The shader format is specified in the header after the word `shader`. The
default is `GLSL`, SPIRV-ASM is specified as `spirv` and SPIRV-Hex as
`spirv hex`.

The shaders accepted are:
 * `compute`
 * `fragment`
 * `geometry`
 * `tessellation control`
 * `tessellation evaulation`
 * `vertex`

#### Shader examples

```
[fragment shader]
#version 430

layout(location = 0) out vec4 color_out;

void main() {
  color_out = vec4(1, 2, 3, 4);
}

```

Other example shader header lines are:
 * `[fragment shader spirv hex]` -- a hex encoded SPIRV binary fragment shader
 * `[tessellation evaluation shader spirv]` -- a spirv-asm tessellation evaluation shader
 * `[vertex shader passthrough]`

## Vertex Data
The `vertex data` section provides vertex attributes and data for `draw array`
commands. The data is formated with a header row followed by data rows.

The headers can be provided in one of two forms. The first,
`attribute_location/format` where `attribute_location` is the location of the
attribute to be bound. The format is one of the *Image Formats* listed below.
The second, `attribute_location/gl_type/glsl_type`. The `gl_type` is one of
the types listed in the *GL Types* section below. The `glsl_type` is one listed
in the *GLSL Types* section below.

#### Vertex Data example

```
[vertex data]
0/R32G32B32_SFLOAT  1/R8G8B8_UNORM
-1    -1 0.25       255 0 0  # ending comment
# Another Row
0.25  -1 0.25       255 0 255
```

## Indices
The `indices` section contains the list of indices to use along with the
provided `vertex data`. The `indices` are used if the `indexed` option is
provided to the `draw arrays` command. The indices themselves are a list of
integer indexes to use.

#### Indices Example

```
[indices]
# comment line
1 2 3   4 5 6
# another comment
7 8 9  10 11 12
```

## Test
The test section contains a list of commands which can be executed to perform
the actual testing. The commands range from setting up pipeline parameters,
executing compute shaders and probing buffers to verify results.


### Draw Rect
 * `draw rect [ortho] [patch] _x_ _y_ _width_ _height_`

The `draw rect` command draws a rectangle at the given coordinates. The vertices
are uploaded at location 0 as a `vec3`. The `ortho` modifier scales the
coordinates to be in the range of [-1,1] instead of [0,window size]. The `patch`
modifier sets the draw call to use a given topology. Accepted possible
topology value are listed in *Topologies*. The patch size will be set to 4.


### Draw Arrays
 * `draw arrays [indexed] [instanced] _topology_ _first_vertex_ _vertex_count_ [instance_count]`

The `draw arrays` command uses data from the `vertex data` section when executing
the draw command. The `topology` is from the *Topologies* list. If the `indexed`
modifier is provided then the `indices` section data will be used as well.


### Compute
 * `compute _x_ _y_ _z_`

Executes the compute shader with the given `x`, `y`, and `z` parameters.


### Shader Entry Point
 * `_stage_ entrypoint _name_`

Sets the `stage` shader to use the entry point of `name` for subsequent
executions.


### Probe all
 * `probe all (rgb|rgba) _r_ _g_ _b_ [_a_]`

Probes the entire window to verify all pixels are of color r,g,b and optionally
a. If `rgba` is specified then the `a` parameter is required. If `rgb` is
specified then the `a` parameter is dis-allowed.


### Probe
 * `[relative] probe [rect] (rgb|rgba) (_x_, _y_[, _width_, _height_]) (_r_, _g_, _b_[, _a_])`

Probes a portion of the window to verify the pixes are of color r,g,b and
optionally a. If `rgba` is specifed then the `a` parameter is required. If
`rgb` is specified then the `a` parameter is dis-allowed. If `rect` is specified
then `width` and `height` are required. If `rect` is not specified then `width`
and `height` are dis-allowed and a value of 1 will be used for each parameter.
If the `relative` parameter is provided the coordinates are normalized to
be in the range [0.0, 1.0].


### Probe SSBO
* `probe ssbo _type_ _binding_ _offset_ _comparison_ _values_+`

Probes the value in the storage buffer at `binding` and `offset` within that
binding. The `type` is the data type to be probed as seen in the *Data Types*
section below.

The comparison operators are:
 * `==`   (equal)
 * `!=`   (not equal)
 * `<`    (less than)
 * '>'    (greater than)
 * `<=`   (less or equal)
 * `>=`   (greater or equal)
 * `~=`   (fuzzy equal, for floating point comparisons using `tolerances`)

The `values` provided must be a non-zero multiple of the `type`.


### Uniform
 * `uniform _type_ _offset _values_+`

Sets the push constants at `offset`. The `type` is from the *Data Types*
section below. The `values` must be a non-zero multiple of the requested
`type`.

When setting push constant data each call to `uniform` must use the same
`type` or the script will be rejected as invalid.


### Unifom UBO
 * `uniform ubo _binding_ _type_ _offset_ _values_+`

Sets the values in the uniform buffer at `binding` and `offset`. The `type`
is from the *Data Types* section below. The `values` must be a non-zero
multiple of the requested `type`.

When setting data into a single `binding`, each call to `uniform ubo` must
use the same `type` or the script will be rejected as invalid.


### SSBO size
 * `ssbo _binding_ _size_`

Sets the number of elements in the SSBO at `binding` to `size`. The buffer will
be created with a default format of `char`. This default format will be
overridden by a call to `ssbo subdata` or `probe ssbo`.


### SSBO subdata
 * `ssbo _binding_ subdata _type_ _offset_ _values_+`

Sets the value of the buffer at `binding` and `offset`. The `type` is from the
*Data Types* section below. The `values` must be a non-zero multiple of the
requested `type`. The offset must be a multiple of the _type_ size in bytes.

When setting data into a single `binding`, each call to `ssbo subdata` must
use the same `type` or the script will be rejected as invalid.


### Patch Parameters
 * `patch parameter vertices _count_`

Sets the number of control points for tessellation patches to `count`. Defaults
to 3.


### Tolerance
 * `tolerance tolerance0 [tolerance1 tolerance2 tolerance3]`

The `tolerance` command sets the amount of fuzzyness used when using the `~=`
comparator. If a single tolerance value is set it is used for every comparison.
If all four values are set then each `vecN` command will use the first `N`
tolerance values. Each column of a `matMxN` will also use the first `N`
tolerances. A tolerance maybe either a number or a percentage `0.01%`.


### Clear Color
 * `clear color _r_ _g_ _b_ _a_`

Sets the clear color. Defaults to (0, 0, 0, 0).


### Clear Depth
 * `clear depth _value_`

Sets the depth clear value. The _value_ is a float and defaults to 1.0.


### Clear Stencil
 * `clear stencil _value_`

Sets the stencil clear value. The _value_ is an integer and defaults to 0.


### Clear
 * `clear`

Clears the framebuffer.

### Pipeline Configuration
There are a number of pipeline flags which can be set to alter execution. Each
draw call uses the pipeline configuration that was specified prior to the draw
call.

The pipeline commands with their accepted data are:
 * `primitiveRestartEnable <bool>`
 * `depthClampEnable <bool>`
 * `rasterizerDiscardEnable <bool>`
 * `depthBiasEnable <bool>`
 * `logicOpEnable <bool>`
 * `blendEnable <bool>`
 * `depthTestEnable <bool>`
 * `depthWriteEnable <bool>`
 * `depthBoundsTestEnable <bool>`
 * `stencilTestEnable <bool>`
 * `topology <VkPrimitiveTopology>`
 * `polygonMode <VkPolygonMode>`
 * `logicOp <VkLogicOp>`
 * `frontFace <VkFrontFace>`
 * `cullMode <VkCullMode>`
 * `depthBiasConstantFactor <float>`
 * `depthBiasClamp <float>`
 * `depthBiasSlopeFactor <float>`
 * `lineWidth <float>`
 * `minDepthBounds <float>`
 * `maxDepthBounds <float>`
 * `srcColorBlendFactor <VkBlendFactor>`
 * `dstColorBlendFactor <VkBlendFactor>`
 * `srcAlphaBlendFactor <VkBlendFactor>`
 * `dstAlphaBlendFactor <VkBlendFactor>`
 * `colorBlendOp <VkBlendOp>`
 * `alphaBlendOp <VkBlendOp>`
 * `depthCompareOp <VkCompareOp>`
 * `front.compareOp <VkCompareOp>`
 * `back.compareOp <VkCompareOp>`
 * `front.failOp <VkStencilOp>`
 * `front.passOp <VkStencilOp>`
 * `front.depthFailOp <VkStencilOp>`
 * `back.failOp <VkStencilOp>`
 * `back.passOp <VkStencilOp>`
 * `back.depthFailOp <VkStencilOp>`
 * `front.reference <uint32_t>`
 * `back.reference <uint32_t>`
 * `colorWriteMask <VkColorComponent bitmask>`

#### Test Example

```
[test]
clear color 1 0.4 0.5 0.2
clear
relative probe rect rgba (0.0, 0.0, 1.0, 1.0) (1.0, 0.4, 0.5, 0.2)
```

### Data Types
 * `int`
 * `uint`
 * `int8_t`
 * `uint8_t`
 * `int16_t`
 * `uint16_t`
 * `int64_t`
 * `uint64_t`
 * `float`
 * `double`
 * `vec`
 * `vec[234]`
 * `dvec`
 * `dvec[234]`
 * `ivec`
 * `ivec[234]`
 * `uvec`
 * `uvec[234]`
 * `i8vec`
 * `i8vec[234]`
 * `u8vec`
 * `u8vec[234]`
 * `i16vec`
 * `i16vec[234]`
 * `u16vec`
 * `u16vec[234]`
 * `i64vec`
 * `i64vec[234]`
 * `u64vec`
 * `u64vec[234]`
 * `mat`
 * `mat[234]x[234]`
 * `dmat`
 * `dmat[234]x[234]`

### Topologies
 * `PATCH_LIST`
 * `POINT_LIST`
 * `GL_LINE_STRIP_ADJACENCY`
 * `GL_LINE_STRIP`
 * `GL_LINES`
 * `GL_LINES_ADJACENCY`
 * `GL_PATCHES`
 * `GL_POINTS`
 * `GL_TRIANGLE_STRIP`
 * `GL_TRIANGLE_FAN`
 * `GL_TRIANGLES`
 * `GL_TRIANGLES_ADJACENCY`
 * `GL_TRIANGLE_STRIP_ADJACENCY`
 * `LINE_LIST`
 * `LINE_LIST_WITH_ADJACENCY`
 * `LINE_STRIP`
 * `LINE_STRIP_WITH_ADJACENCY`
 * `TRIANGLE_FAN`
 * `TRIANGLE_LIST`
 * `TRIANGLE_LIST_WITH_ADJACENCY`
 * `TRIANGLE_STRIP`
 * `TRIANGLE_STRIP_WITH_ADJACENCY`


### GL Types
 * `byte`
 * `ubyte`
 * `short`
 * `ushort`
 * `int`
 * `uint`
 * `half`
 * `float`
 * `double`

### GLSL Types
 * `int`
 * `uint`
 * `float`
 * `double`
 * `vec`
 * `vec2`
 * `vec3`
 * `vec4`
 * `dvec`
 * `dvec2`
 * `dvec3`
 * `dvec4`
 * `uvec`
 * `uvec2`
 * `uvec3`
 * `uvec4`
 * `ivec`
 * `ivec2`
 * `ivec3`
 * `ivec4`

### Available Require Features
  * `robustBufferAccess`
  * `fullDrawIndexUint32`
  * `imageCubeArray`
  * `independentBlend`
  * `geometryShader`
  * `tessellationShader`
  * `sampleRateShading`
  * `dualSrcBlend`
  * `logicOp`
  * `multiDrawIndirect`
  * `drawIndirectFirstInstance`
  * `depthClamp`
  * `depthBiasClamp`
  * `fillModeNonSolid`
  * `depthBounds`
  * `wideLines`
  * `largePoints`
  * `alphaToOne`
  * `multiViewport`
  * `samplerAnisotropy`
  * `textureCompressionETC2`
  * `textureCompressionASTC_LDR`
  * `textureCompressionBC`
  * `occlusionQueryPrecise`
  * `pipelineStatisticsQuery`
  * `vertexPipelineStoresAndAtomics`
  * `fragmentStoresAndAtomics`
  * `shaderTessellationAndGeometryPointSize`
  * `shaderImageGatherExtended`
  * `shaderStorageImageExtendedFormats`
  * `shaderStorageImageMultisample`
  * `shaderStorageImageReadWithoutFormat`
  * `shaderStorageImageWriteWithoutFormat`
  * `shaderUniformBufferArrayDynamicIndexing`
  * `shaderSampledImageArrayDynamicIndexing`
  * `shaderStorageBufferArrayDynamicIndexing`
  * `shaderStorageImageArrayDynamicIndexing`
  * `shaderClipDistance`
  * `shaderCullDistance`
  * `shaderFloat64`
  * `shaderInt64`
  * `shaderInt16`
  * `shaderResourceResidency`
  * `shaderResourceMinLod`
  * `sparseBinding`
  * `sparseResidencyBuffer`
  * `sparseResidencyImage2D`
  * `sparseResidencyImage3D`
  * `sparseResidency2Samples`
  * `sparseResidency4Samples`
  * `sparseResidency8Samples`
  * `sparseResidency16Samples`
  * `sparseResidencyAliased`
  * `variableMultisampleRate`
  * `inheritedQueries`

### Image Formats
  * `A1R5G5B5_UNORM_PACK16`
  * `A2B10G10R10_SINT_PACK32`
  * `A2B10G10R10_SNORM_PACK32`
  * `A2B10G10R10_SSCALED_PACK32`
  * `A2B10G10R10_UINT_PACK32`
  * `A2B10G10R10_UNORM_PACK32`
  * `A2B10G10R10_USCALED_PACK32`
  * `A2R10G10B10_SINT_PACK32`
  * `A2R10G10B10_SNORM_PACK32`
  * `A2R10G10B10_SSCALED_PACK32`
  * `A2R10G10B10_UINT_PACK32`
  * `A2R10G10B10_UNORM_PACK32`
  * `A2R10G10B10_USCALED_PACK32`
  * `A8B8G8R8_SINT_PACK32`
  * `A8B8G8R8_SNORM_PACK32`
  * `A8B8G8R8_SRGB_PACK32`
  * `A8B8G8R8_SSCALED_PACK32`
  * `A8B8G8R8_UINT_PACK32`
  * `A8B8G8R8_UNORM_PACK32`
  * `A8B8G8R8_USCALED_PACK32`
  * `B10G11R11_UFLOAT_PACK32`
  * `B4G4R4A4_UNORM_PACK16`
  * `B5G5R5A1_UNORM_PACK16`
  * `B5G6R5_UNORM_PACK16`
  * `B8G8R8A8_SINT`
  * `B8G8R8A8_SNORM`
  * `B8G8R8A8_SRGB`
  * `B8G8R8A8_SSCALED`
  * `B8G8R8A8_UINT`
  * `B8G8R8A8_UNORM`
  * `B8G8R8A8_USCALED`
  * `B8G8R8_SINT`
  * `B8G8R8_SNORM`
  * `B8G8R8_SRGB`
  * `B8G8R8_SSCALED`
  * `B8G8R8_UINT`
  * `B8G8R8_UNORM`
  * `B8G8R8_USCALED`
  * `D16_UNORM`
  * `D16_UNORM_S8_UINT`
  * `D24_UNORM_S8_UINT`
  * `D32_SFLOAT`
  * `D32_SFLOAT_S8_UINT`
  * `R16G16B16A16_SFLOAT`
  * `R16G16B16A16_SINT`
  * `R16G16B16A16_SNORM`
  * `R16G16B16A16_SSCALED`
  * `R16G16B16A16_UINT`
  * `R16G16B16A16_UNORM`
  * `R16G16B16A16_USCALED`
  * `R16G16B16_SFLOAT`
  * `R16G16B16_SINT`
  * `R16G16B16_SNORM`
  * `R16G16B16_SSCALED`
  * `R16G16B16_UINT`
  * `R16G16B16_UNORM`
  * `R16G16B16_USCALED`
  * `R16G16_SFLOAT`
  * `R16G16_SINT`
  * `R16G16_SNORM`
  * `R16G16_SSCALED`
  * `R16G16_UINT`
  * `R16G16_UNORM`
  * `R16G16_USCALED`
  * `R16_SFLOAT`
  * `R16_SINT`
  * `R16_SNORM`
  * `R16_SSCALED`
  * `R16_UINT`
  * `R16_UNORM`
  * `R16_USCALED`
  * `R32G32B32A32_SFLOAT`
  * `R32G32B32A32_SINT`
  * `R32G32B32A32_UINT`
  * `R32G32B32_SFLOAT`
  * `R32G32B32_SINT`
  * `R32G32B32_UINT`
  * `R32G32_SFLOAT`
  * `R32G32_SINT`
  * `R32G32_UINT`
  * `R32_SFLOAT`
  * `R32_SINT`
  * `R32_UINT`
  * `R4G4B4A4_UNORM_PACK16`
  * `R4G4_UNORM_PACK8`
  * `R5G5B5A1_UNORM_PACK16`
  * `R5G6B5_UNORM_PACK16`
  * `R64G64B64A64_SFLOAT`
  * `R64G64B64A64_SINT`
  * `R64G64B64A64_UINT`
  * `R64G64B64_SFLOAT`
  * `R64G64B64_SINT`
  * `R64G64B64_UINT`
  * `R64G64_SFLOAT`
  * `R64G64_SINT`
  * `R64G64_UINT`
  * `R64_SFLOAT`
  * `R64_SINT`
  * `R64_UINT`
  * `R8G8B8A8_SINT`
  * `R8G8B8A8_SNORM`
  * `R8G8B8A8_SRGB`
  * `R8G8B8A8_SSCALED`
  * `R8G8B8A8_UINT`
  * `R8G8B8A8_UNORM`
  * `R8G8B8A8_USCALED`
  * `R8G8B8_SINT`
  * `R8G8B8_SNORM`
  * `R8G8B8_SRGB`
  * `R8G8B8_SSCALED`
  * `R8G8B8_UINT`
  * `R8G8B8_UNORM`
  * `R8G8B8_USCALED`
  * `R8G8_SINT`
  * `R8G8_SNORM`
  * `R8G8_SRGB`
  * `R8G8_SSCALED`
  * `R8G8_UINT`
  * `R8G8_UNORM`
  * `R8G8_USCALED`
  * `R8_SINT`
  * `R8_SNORM`
  * `R8_SRGB`
  * `R8_SSCALED`
  * `R8_UINT`
  * `R8_UNORM`
  * `R8_USCALED`
  * `S8_UINT`
  * `X8_D24_UNORM_PACK32`

1- https://github.com/Igalia/vkrunner/blob/d817f8b186cccebed89471580a685dc80a330946/README.md
