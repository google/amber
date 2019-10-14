# AmberScript
 * DRAFT

This document defines the script input language for the Amber system. The format
is based on the Talvos format, VkRunner format, and VkScript proposed format.

## Specification
All amber scripts must start with `#!amber` as the first line. Comments are
specified by a # character and continue to the end of the line, except in
inlined shader source code, where AmberScript comments are not
possible. Keywords are case sensitive. All names are made up of ASCII
characters, and delimited by whitespace.

TODO(dneto): What characters are valid in a name?

### Number literals

Literal numbers are normally presented in decimal form.  They are interpreted
as integers or floating point depending on context: a command parameter is
predefined as either integral or floating point, or the data type is
user-specified (such as for buffer data).

Hex values: Whenever an integer is expected, you may use a hexadecimal number,
which is the characters `0x` followed by hexadecimal digits.

### Requesting features

If specific device features are required you can use the `DEVICE_FEATURE`
command to enable them.

```groovy
DEVICE_FEATURE vertexPipelineStoresAndAtomics
DEVICE_FEATURE VariablePointerFeatures.variablePointersStorageBuffer
```

Currently each of the items in `VkPhysicalDeviceFeatures` are recognized along
with `VariablePointerFeatures.variablePointers` and
`VariablePointerFeatures.variablePointersStorageBuffer`.

Extensions can be enabled with the `DEVICE_EXTENSION` and `INSTANCE_EXTENSION`
commands.

```groovy
DEVICE_EXTENSION VK_KHR_get_physical_device_properties2
INSTANCE_EXTENSION VK_KHR_storage_buffer_storage_class
```

### Setting Engine Configuration

In some instances there is extra data we want to provide to an engine for
configuration purposes. The `SET ENGINE_DATA` command allows that for the given
set of data types.

#### Engine Data Variables
  * `fence_timeout_ms`  - value must be a single uint32 in milliseconds.

```groovy
SET ENGINE_DATA {engine data variable} {value}*
```

### Shaders

#### Shader Type
 * `vertex`
 * `fragment`
 * `geometry`
 * `tessellation_evaluation`
 * `tessellation_control`
 * `compute`
 * `multi`

The compute pipeline can only contain compute shaders. The graphics pipeline
can not contain compute shaders, and must contain a vertex shader and a fragment
shader.

The provided `multi` shader can only be used with `SPIRV-ASM` and `SPIRV-HEX`
and allows for providing multiple shaders in a single module (so the `vertex`
and `fragment` shaders can be provided together.)

Note, `SPIRV-ASM` and `SPIRV-HEX` can also be used with each of the other shader
types, but in that case must only provide a single shader type in the module.

#### Shader Format
 * `GLSL`  (with glslang)
 * `HLSL`  (with dxc or glslang if dxc disabled)  -- future
 * `SPIRV-ASM` (with spirv-as)
 * `SPIRV-HEX` (decoded straight to SPIR-V)
 * `OPENCL-C` (with clspv)

```groovy
# Creates a passthrough vertex shader. The shader passes the vec4 at input
# location 0 through to the `gl_Position`.
SHADER vertex {shader_name} PASSTHROUGH

# Creates a shader of |shader_type| with the given |shader_name|. The shader
# will be of |shader_format|. The shader should then be inlined before the
# |END| tag.
SHADER {shader_type} {shader_name} {shader_format}
...
END
```

### Buffers

An AmberScript buffer represents a set of contiguous bits. This can be used for
either image buffers or, what the target API would refer to as a buffer.

#### Data Types
 * `int8`
 * `int16`
 * `int32`
 * `int64`
 * `uint8`
 * `uint16`
 * `uint32`
 * `uint64`
 * `float`
 * `double`
 * vec[2,3,4]{type}
 * mat[2,3,4]x[2,3,4]{type}  (mat<columns>x<rows>)
 * Any of the `Image Formats` listed below.

Sized arrays and structures are not currently representable.

```groovy
# Filling the buffer with a given set of data. The values must be
# of |type| data. The data can be provided as the type or as a hex value.
# Buffers are STD430 by default.
BUFFER {name} DATA_TYPE {type} {STD140 | STD430} DATA
_value_+
END

# Defines a buffer which is filled with data as specified by the `initializer`.
BUFFER {name} DATA_TYPE {type} {STD140 | STD430} SIZE _size_in_items_ \
    {initializer}

# Creates a buffer which will store the given `FORMAT` of data. These
# buffers are used as image and depth buffers in the `PIPELINE` commands.
# The buffer will be sized based on the `RENDER_SIZE` of the `PIPELINE`.
BUFFER {name} FORMAT {format_string}
```

#### Buffer Initializers

```groovy
# Fill the buffer with a single value.
FILL _value_

# Fill the buffer with an increasing value from |start| increasing by |inc|.
# Floating point data uses floating point addition to generate increasing
# values. Likewise, integer data uses integer addition to generate increasing
# values.
SERIES_FROM _start_ INC_BY _inc_
```

#### Buffer Copy

```groovy
# Copies all data, values and memory from |buffer_from| to |buffer_to|.
# Both buffers must be declared, and of the same data type.
# Buffers used as copy destination can be used only as copy destination, and as
# argument to an EXPECT command.
COPY {buffer_from} TO {buffer_to}
```

### Pipelines

#### Pipeline type
 * `compute`
 * `graphics`

```groovy
# The PIPELINE command creates a pipeline. This can be either compute or
# graphics. Shaders are attached to the pipeline at pipeline creation time.
PIPELINE {pipeline_type} {pipeline_name}
...
END

# Create a pipeline and inherit from a previously declared pipeline.
DERIVE_PIPELINE {pipeline_name} FROM {parent_pipeline}
...
END
```

### Pipeline Content

The following commands are all specified within the `PIPELINE` command.
```groovy
  # Attach the shader provided by |name_of_shader| to the pipeline and set
  # the entry point to be |name|. The provided shader for ATTACH must _not_ be
  # a 'multi' shader.
  ATTACH {name_of_shader} ENTRY_POINT {name}

  # Attach the shader provided by |name_of_shader| to the pipeline and set
  # the entry point to be 'main'. The provided shader for ATTACH must _not_ be
  # a 'multi' shader.
  ATTACH {name_of_shader}

  # Attach a 'multi' shader to the pipeline of |shader_type| and use the entry
  # point with |name|. The provided shader _must_ be a 'multi' shader.
  ATTACH {name_of_multi_shader} TYPE {shader_type} ENTRY_POINT {name}

  # Attach specialized shader. Specialization can be specified multiple times.
  # Specialization values must be a 32-bit type. Shader type and entry point
  # must be specified prior to specializing the shader.
  ATTACH {name_of_shader} SPECIALIZE 1 AS uint32 4
  ATTACH {name_of_shader} SPECIALIZE 1 AS uint32 4 SPECIALIZE 4 AS float 1.0
```

```groovy
  # Set the SPIRV-Tools optimization passes to use for a given shader. The
  # default is to run no optimization passes.
  SHADER_OPTIMIZATION {shader_name}
    {optimization_name}+
  END
```

```groovy
  # Set the compile options used to compile the given shader. Options are parsed
  # the same as on the command line. Currently, only supported for OPENCL-C shaders.
  COMPILE_OPTIONS {shader_name}
    {option}+
  END
```

```groovy
  # Set the size of the render buffers. |width| and |height| are integers and
  # default to 250x250.
  FRAMEBUFFER_SIZE _width_ _height_
```

### Pipeline Buffers

#### Buffer Types
 * `uniform`
 * `storage`

TODO(dsinclair): Sync the BufferTypes with the list of Vulkan Descriptor types.

A `pipeline` can have buffers bound. This includes buffers to contain image
attachment content, depth/stencil content, uniform buffers, etc.

```groovy
  # Attach |buffer_name| as an output color attachment at location |idx|.
  # The provided buffer must be a `FORMAT` buffer. If no color attachments are
  # provided a single attachment with format `B8G8R8A8_UNORM` will be created
  # for graphics pipelines.
  BIND BUFFER {buffer_name} AS color LOCATION _idx_

  # Attach |buffer_name| as the depth/stencil buffer. The provided buffer must
  # be a `FORMAT` buffer. If no depth/stencil buffer is specified a default
  # buffer of format `D32_SFLOAT_S8_UINT` will be created for graphics
  # pipelines.
  BIND BUFFER {buffer_name} AS depth_stencil

  # Attach |buffer_name| as the push_constant buffer. There can be only one
  # push constant buffer attached to a pipeline.
  BIND BUFFER <buffer_name> AS push_constant

  # Bind the buffer of the given |buffer_type| at the given descriptor set
  # and binding. The buffer will use a start index of 0.
  BIND BUFFER {buffer_name} AS {buffer_type} DESCRIPTOR_SET _id_ \
       BINDING _id_

  # Bind the sampler at the given descriptor set and binding.
  BIND SAMPLER {sampler_name} DESCRIPTOR_SET _id_ BINDING _id_

  # Bind OpenCL argument buffer by name. Specifying the buffer type is optional.
  # Amber will set the type as appropriate for the argument buffer. All uses
  # of the buffer must have a consistent |buffer_type| across all pipelines.
  BIND BUFFER {buffer_name} [AS {buffer_type}] KERNEL ARG_NAME _name_

  # Bind OpenCL argument buffer by argument ordinal. Arguments use 0-based
  # numbering. Specifying the buffer type is optional. Amber will set the
  # type as appropriate for the argument buffer. All uses of the buffer
  # must have a consistent |buffer_type| across all pipelines.
  BIND BUFFER {buffer_name} [AS {buffer_type}] KERNEL ARG_NUMBER _number_
```

```groovy
  # Set |buffer_name| as the vertex data at location |val|.
  VERTEX_DATA {buffer_name} LOCATION _val_

  # Set |buffer_name| as the index data to use for `INDEXED` draw commands.
  INDEX_DATA {buffer_name}
```

#### OpenCL Plain-Old-Data Arguments
OpenCL kernels can have plain-old-data (pod or pod_ubo in the desriptor map)
arguments set their data via this command. Amber will generate the appropriate
buffers for the pipeline populated with the specified data.

```groovy
  # Set argument |name| to |data_type| with value |val|.
  SET KERNEL ARG_NAME _name_ AS {data_type} _val_

  # Set argument |number| to |data_type| with value |val|.
  # Arguments use 0-based numbering.
  SET KERNEL ARG_NUMBER _number_ AS {data_type} _val_
```

#### Topologies
 * `point_list`
 * `line_list`
 * `line_list_with_adjacency`
 * `line_strip`
 * `line_strip_with_adjacency`
 * `triangle_list`
 * `triangle_list_with_adjacency`
 * `triangle_strip`
 * `triangle_strip_with_adjacency`
 * `triangle_fan`
 * `patch_list`

### Run a pipeline.

When running a `DRAW_ARRAY` command, you must attach the vertex data to the
`PIPELINE` with the `VERTEX_DATA` command.

To run an indexed draw, attach the index data to the `PIPELINE` with an
`INDEX_DATA` command.

For the commands which take a `START_IDX` and a `COUNT` they can be left off the
command (although, `START_IDX` is required if `COUNT` is provided). The default
value for `START_IDX` is 0. The default value for `COUNT` is the item count of
vertex buffer minus the `START_IDX`.

```groovy
# Run the given |pipeline_name| which must be a `compute` pipeline. The
# pipeline will be run with the given number of workgroups in the |x|, |y|, |z|
# dimensions. Each of the x, y and z values must be a uint32.
RUN {pipeline_name} _x_ _y_ _z_

# Run the given |pipeline_name| which must be a `graphics` pipeline. The
# rectangle at |x|, |y|, |width|x|height| will be rendered. Ignores VERTEX_DATA
# and INDEX_DATA on the given pipeline.
RUN {pipeline_name} \
  DRAW_RECT POS _x_in_pixels_ _y_in_pixels_ \
  SIZE _width_in_pixels_ _height_in_pixels_
```

```groovy
# Run the |pipeline_name| which must be a `graphics` pipeline. The vertex
# data must be attached to the pipeline. A start index of 0 will be used
# and a count of the number of elements in the vertex buffer.
RUN {pipeline_name} DRAW_ARRAY AS {topology}

# Run the |pipeline_name| which must be a `graphics` pipeline. The vertex
# data must be attached to the pipeline. A start index of |value| will be used
# and a count of the number of items from |value| to the end of the vertex
# buffer.
RUN {pipeline_name} DRAW_ARRAY AS {topology} START_IDX _value_

# Run the |pipeline_name| which must be a `graphics` pipeline. The vertex
# data must be attached to the pipeline. A start index of |value| will be used
# and a count |count_value| will be used.
RUN {pipeline_name} DRAW_ARRAY AS {topology} START_IDX _value_ \
  COUNT _count_value_
```

```groovy
# Run the |pipeline_name| which must be a `graphics` pipeline. The vertex
# data and  index data must be attached to the pipeline. The vertices will be
# drawn using the given |topology|. A start index of 0 will be used and the
# count will be determined by the size of the index data buffer.
RUN {pipeline_name} DRAW_ARRAY AS {topology} INDEXED

# Run the |pipeline_name| which must be a `graphics` pipeline. The vertex
# data and  index data must be attached to the pipeline. The vertices will be
# drawn using the given |topology|. A start index of |value| will be used and
# the count will be determined by the size of the index data buffer.
RUN {pipeline_name} DRAW_ARRAY AS {topology} INDEXED START_IDX _value_

# Run the |pipeline_name| which must be a `graphics` pipeline. The vertex
# data and  index data must be attached to the pipeline. The vertices will be
# drawn using the given |topology|. A start index of |value| will be used and
# the count of |count_value| items will be processed.
RUN {pipeline_name} DRAW_ARRAY AS {topology} INDEXED \
  START_IDX _value_ COUNT _count_value_
```

### Repeating commands

```groovy
# It is sometimes useful to run a given draw command multiple times. This can be
# to detect deterministic rendering or other features.
REPEAT {count}
{command}+
END
```

The commands which can be used inside a `REPEAT` block are:
  * `CLEAR`
  * `CLEAR_COLOR`
  * `COPY`
  * `EXPECT`
  * `RUN`

### Commands

```groovy
# Sets the clear color to use for |pipeline| which must be a graphics
# pipeline. The colors are integers from 0 - 255.
CLEAR_COLOR {pipeline} _r (0 - 255)_ _g (0 - 255)_ _b (0 - 255)_ _a (0 - 255)_

# Instructs the |pipeline| which must be a graphics pipeline to execute the
# clear command.
CLEAR {pipeline}
```

### Expectations

#### Comparators
 * `EQ`
 * `NE`
 * `LT`
 * `LE`
 * `GT`
 * `GE`
 * `EQ_RGB`
 * `EQ_RGBA`
 * `EQ_BUFFER`
 * `RMSE_BUFFER`
 * `EQ_HISTOGRAM_EMD_BUFFER`

```groovy
# Checks that |buffer_name| at |x| has the given |value|s when compared
# with the given |comparator|.
EXPECT {buffer_name} IDX _x_ {comparator} _value_+

# Checks that |buffer_name| at |x| has values within |tolerance| of |value|
# The |tolerance| can be specified as 1-4 float values separated by spaces.
# The tolerances may be given as a percentage by placing a '%' symbol after
# the value. If less tolerance values are provided then are needed for a given
# data component the default tolerance will be applied.
EXPECT {buffer_name} IDX _x_ TOLERANCE _tolerance_{1,4} EQ _value_+

# Checks that |buffer_name| at |x|, |y| for |width|x|height| pixels has the
# given |r|, |g|, |b| values. Each r, g, b value is an integer from 0-255.
EXPECT {buffer_name} IDX _x_in_pixels_ _y_in_pixels_ \
  SIZE _width_in_pixels_ _height_in_pixels_ \
  EQ_RGB _r (0 - 255)_ _g (0 - 255)_ _b (0 - 255)_

# Checks that |buffer_name| at |x|, |y| for |width|x|height| pixels has the
# given |r|, |g|, |b|, |a| values. Each r, g, b, a value is an integer
# from 0-255.
EXPECT {buffer_name} IDX _x_in_pixels_ _y_in_pixels_ \
  SIZE _width_in_pixels_ _height_in_pixels_ \
  EQ_RGBA _r (0 - 255)_ _g (0 - 255)_ _b (0 - 255)_ _a (0 - 255)_

# Checks that |buffer_1| contents are equal to those of |buffer_2|
EXPECT {buffer_1} EQ_BUFFER {buffer_2}

# Checks that the Root Mean Square Error when comparing |buffer_1| to
# |buffer_2| is less than or equal to |tolerance|. Note, |tolerance| is a
# unit-less number.
EXPECT {buffer_1} RMSE_BUFFER {buffer_2} TOLERANCE _value_

# Checks that the Earth Mover's Distance when comparing histograms of
# |buffer_1| to |buffer_2| is less than or equal to |tolerance|.
# Note, |tolerance| is a unit-less number.
EXPECT {buffer_1} EQ_HISTOGRAM_EMD_BUFFER {buffer_2} TOLERANCE _value_
```

## Examples

### Compute Shader

```groovy
#!amber
# Simple amber compute shader.

SHADER compute kComputeShader GLSL
#version 450

layout(binding = 3) buffer block {
  vec2 values[];
};

void main() {
  values[gl_WorkGroupID.x + gl_WorkGroupID.y * gl_NumWorkGroups.x] =
                gl_WorkGroupID.xy;
}
END  # shader

BUFFER kComputeBuffer DATA_TYPE vec2<int32> SIZE 524288 FILL 0

PIPELINE compute kComputePipeline
  ATTACH kComputeShader
  BIND BUFFER kComputeBuffer AS storage DESCRIPTOR_SET 0 BINDING 3
END  # pipeline

RUN kComputePipeline 256 256 1

# Four corners
EXPECT kComputeBuffer IDX 0 EQ 0 0
EXPECT kComputeBuffer IDX 2040 EQ 255 0
EXPECT kComputeBuffer IDX 522240 EQ 0 255
EXPECT kComputeBuffer IDX 524280 EQ 255 255

# Center
EXPECT kComputeBuffer IDX 263168 EQ 128 128
```

### Entry Points

```groovy
#!amber

SHADER vertex kVertexShader PASSTHROUGH

SHADER fragment kFragmentShader SPIRV-ASM
              OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450

; two entrypoints
               OpEntryPoint Fragment %red "red" %color
               OpEntryPoint Fragment %green "green" %color

               OpExecutionMode %red OriginUpperLeft
               OpExecutionMode %green OriginUpperLeft
               OpSource GLSL 430
               OpName %red "red"
               OpDecorate %color Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
      %color = OpVariable %_ptr_Output_v4float Output
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
  %red_color = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%green_color = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1

; this entrypoint outputs a red color
        %red = OpFunction %void None %3
          %5 = OpLabel
               OpStore %color %red_color
               OpReturn
               OpFunctionEnd

; this entrypoint outputs a green color
      %green = OpFunction %void None %3
          %6 = OpLabel
               OpStore %color %green_color
               OpReturn
               OpFunctionEnd
END  # shader

BUFFER kImgBuffer FORMAT R8G8B8A8_UINT

PIPELINE graphics kRedPipeline
  ATTACH kVertexShader ENTRY_POINT main
  SHADER_OPTIMIZATION kVertexShader
    --eliminate-dead-branches
    --merge-return
    --eliminate-dead-code-aggressive
  END
  ATTACH kFragmentShader ENTRY_POINT red

  FRAMEBUFFER_SIZE 256 256
  BIND BUFFER kImgBuffer AS color LOCATION 0
END  # pipeline

PIPELINE graphics kGreenPipeline
  ATTACH kVertexShader
  ATTACH kFragmentShader ENTRY_POINT green

  FRAMEBUFFER_SIZE 256 256
  BIND BUFFER kImgBuffer AS color LOCATION 0
END  # pipeline

RUN kRedPipeline DRAW_RECT POS 0 0 SIZE 256 256
RUN kGreenPipeline DRAW_RECT POS 128 128 SIZE 256 256

EXPECT kImgBuffer IDX 0 0 SIZE 127 127 EQ_RGB 255 0 0
EXPECT kImgBuffer IDX 128 128 SIZE 128 128 EQ_RGB 0 255 0
```

### Buffers

```groovy
#!amber

SHADER vertex kVertexShader GLSL
  #version 430

  layout(location = 0) in vec4 position;
  layout(location = 1) in vec4 color_in;
  layout(location = 0) out vec4 color_out;

  void main() {
    gl_Position = position;
    color_out = color_in;
  }
END  # shader

SHADER fragment kFragmentShader GLSL
  #version 430

  layout(location = 0) in vec4 color_in;
  layout(location = 0) out vec4 color_out;

  void main() {
    color_out = color_in;
  }
END  # shader

BUFFER kPosData DATA_TYPE vec2<int32> DATA
# Top-left
-1 -1  
 0 -1  
-1  0
 0  0
# Top-right
 0 -1  
 1 -1  
 0  0
 1  0
# Bottom-left
-1  0
 0  0
-1  1
 0  1
# Bottom-right
 0  0
 1  0
 0  1
 1  1
END

BUFFER kColorData DATA_TYPE uint32 DATA
# red
0xff0000ff
0xff0000ff
0xff0000ff
0xff0000ff

# green
0xff00ff00
0xff00ff00
0xff00ff00
0xff00ff00

# blue
0xffff0000
0xffff0000
0xffff0000
0xffff0000

# purple
0xff800080
0xff800080
0xff800080
0xff800080
END

BUFFER kIndices DATA_TYPE int32 DATA
0  1  2    2  1  3
4  5  6    6  5  7
8  9  10   10 9  11
12 13 14   14 13 15
END

PIPELINE graphics kGraphicsPipeline
  ATTACH kVertexShader
  ATTACH kFragmentShader

  VERTEX_DATA kPosData LOCATION 0
  VERTEX_DATA kColorData LOCATION 1
  INDEX_DATA kIndices
END  # pipeline

CLEAR_COLOR kGraphicsPipeline 255 0 0 255
CLEAR kGraphicsPipeline

RUN kGraphicsPipeline DRAW_ARRAY AS triangle_list START_IDX 0 COUNT 24
```

### OpenCL-C Shaders

```groovy
SHADER compute my_shader OPENCL-C
kernel void line(const int* in, global int* out, int m, int b) {
  *out = *in * m + b;
}
END

BUFFER in_buf DATA_TYPE int32 DATA 4 END
BUFFER out_buf DATA_TYPE int32 DATA 0 END

PIPELINE compute my_pipeline
  ATTACH my_shader ENTRY_POINT line
  COMPILE_OPTIONS
    -cluster-pod-kernel-args
    -pod-ubo
    -constant-args-ubo
    -max-ubo-size=128
  END
  BIND BUFFER in_buf KERNEL ARG_NAME in
  BIND BUFFER out_buf KERNEL ARG_NAME out
  SET KERNEL ARG_NAME m AS int32 3
  SET KERNEL ARG_NAME b AS int32 1
END

RUN my_pipeline 1 1 1

EXPECT out_buf EQ IDX 0 EQ 13
```

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
