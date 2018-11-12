# AmberScript
 * DRAFT

This document defines the script input language for the Amber system. The format
is based on the Talvos format, VkRunner format, and VkScript proposed format.

## Specification
All amber scripts must start with `#!amber` as the first line. Comments are
specified by a # character and continue to the end of the line. Keywords are
case sensitive. All names are made up of ASCII characters, and delimited by
whitespace.

TODO(dneto): What characters are valid in a name?

### Number literals

Literal numbers are normally presented in decimal form.  They are interpreted
as integers or floating point depending on context: a command parameter is predefined
as either integral or floating point, or the data type is user-specified (such
as for buffer data).

Hex values: Whenever an integer is expected, you may use a hexadecimal number, which
is the characters `0x` followed by hexadecimal digits.

### Shaders

#### Shader Type
 * vertex
 * fragment
 * geometry
 * tessellation\_evaluation
 * tessellation\_control
 * compute

The compute pipeline can only contain compute shaders. The graphics pipeline
can not contain compute shaders, and must contain a vertex shader and a fragment
shader.

#### Shader Format
 * GLSL  (with glslang)
 * HLSL  (with dxc or glslang if dxc disabled)  -- future
 * SPIRV-ASM (with spirv-as)
 * SPIRV-HEX (decoded straight to spv)
 * OPENCL-C (with clspv)  --- potentially?  -- future

```
SHADER vertex <shader_name> PASSTHROUGH

SHADER <shader_type> <shader_name> <shader_format>
...
END
```

### Buffers

#### Data Types
 * int8
 * int16
 * int32
 * int64
 * uint8
 * uint16
 * uint32
 * uint64
 * float
 * double
 * vec[2,3,4]\<type>
 * mat[2,3,4]\<type>    -- useful?

TODO(dneto): Support half-precision floating point.

Sized arrays and structures are not currently representable.

#### Buffer Types
 * uniform
 * storage
 * vertex
 * index
 * sampled
 * storage
 * color
 * depth

```
// Filling the buffer with a given set of data. The values must provide
// <size_in_bytes> of <type> data. The data can be provided as the type or
// as a hex value.

BUFFER <buffer_type> <name> DATA_TYPE <type> DATA
<value>+
END

BUFFER <buffer_type> <name> DATA_TYPE <type> SIZE <size_in_bytes> <initializer>

BUFFER framebuffer <name> DIMS <width_in_pixels> <height_in_pixels>
```

#### Buffer Initializers
Fill the buffer with a single value.

```
FILL <value>
```

Fill the buffer with an increasing value from \<start> increasing by \<inc>.
Floating point data uses floating point addition to generate increasting values.
Likewise, integer data uses integer addition to generate increasing values.

```
SERIES_FROM <start> INC_BY <inc>
```

### Pipelines

#### Pipeline type
 * compute
 * graphics

The PIPELINE command creates a pipeline. This can be either compute or graphics.
Shaders are attached to the pipeline at pipeline creation time.

```
PIPELINE <pipeline_type> <pipeline_name>
...
END
```

### Pipeline Content

The following commands are all specified within the `PIPELINE` command. If you
have multiple entry points for a given shader, you'd create multiple pipelines
each with a different `ENTRY_POINT`.

Bind the entry point to use for a given shader. The default entry point is main.

```
  ENTRY_POINT <shader_name> <entry_point_name>
```

Shaders can be added into pipelines with the `ATTACH` call. Shaders may be
attached to multiple pipelines at the same time.

```
  ATTACH <name_of_vertex_shader>
  ATTACH <name_of_fragment_shader>
```

Set the SPIRV-Tools optimization passes to use for a given shader. The default
is to run no optimization passes.
```
  SHADER_OPTIMIZATION <shader_name>
    <optimization_name>+
  END
```

#### Bindings

##### Topologies
 * point\_list
 * line\_list
 * line\_list\_with\_adjacency
 * line\_strip
 * line\_strip\_with\_adjacency
 * triangle\_list
 * triangle\_list\_with\_adjacency
 * triangle\_strip
 * triangle\_strip\_with\_adjacency
 * triangle\_fan
 * patch\_list

Bind a provided framebuffer.

```
  FRAMEBUFFER <buffer_of_type_framebuffer_name>
```

Descriptor sets can be bound as:

```
  DESCRIPTOR_SET <id> BINDING <id> IDX <0> TO <buffer_name>
```

### Run a pipeline.

```
RUN <pipeline_name> <x> <y> <z>

RUN <pipeline_name> \
  DRAW_RECT POS <x_in_pixels> <y_in_pixels> \
  SIZE <width_in_pixels> <height_in_pixels>

RUN <pipeline_name> \
  DRAW_ARRAY <indices_buffer> IN <data_buffer> \
  AS <topology> START_IDX <value> COUNT <value>
```

### Commands
```
CLEAR_COLOR <pipeline> <r (0 - 255)> <g (0 - 255)> <b (0 - 255)>

CLEAR <pipeline>
```

### Expectations

#### Comparators
 * EQ
 * NE
 * LT
 * LE
 * GT
 * GE
 * EQ\_RGB
 * EQ\_RGBA

```
EXPECT <buffer_name> IDX <x> <y> <comparator> <value>+

EXPECT <framebuffer_name> IDX <x_in_pixels> <y_in_pixels> \
  SIZE <width_in_pixels> <height_in_pixels> \
  EQ_RGB <r (0 - 255)> <g (0 - 255)> <b (0 - 255)>
```

## Examples

### Compute Shader
```
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

BUFFER storage kComputeBuffer TYPE vec2<int32> SIZE 524288 FILL 0

PIPELINE compute kComputePipeline
  ATTACH kComputeShader
  DESCRIPTOR_SET 0 BINDING 3 IDX 0 TO kComputeBuffer
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
```
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

BUFFER framebuffer kFrameBuffer DIMS 256 256

PIPELINE graphics kRedPipeline
  ATTACH kVertexShader
  SHADER_OPTIMIZATION kVertexShader
    eliminate-dead-branches
    merge-return
    eliminate-dead-code-aggressive
  END

  ATTACH kFragmentShader

  FRAMEBUFFER kFrameBuffer
  ENTRY_POINT kFragmentShader red
END  # pipeline

PIPELINE graphics kGreenPipeline
  ATTACH kVertexShader
  ATTACH kFragmentShader

  FRAMEBUFFER kFrameBuffer
  ENTRY_POINT kFragmentShader green
END  # pipeline

RUN kRedPipeline DRAW_RECT POS 0 0  SIZE 256 256
RUN kGreenPipeline DRAW_RECT POS 128 128 SIZE 256 256

EXPECT kFrameBuffer IDX 0 0 SIZE 127 127 EQ_RGB 255 0 0
EXPECT kFrameBuffer IDX 128 128 SIZE 128 128 EQ_RGB 0 255 0
```

### Buffers
```
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

PIPELINE graphics kGraphicsPipeline
  ATTACH kVertexShader
  ATTACH kFragmentShader
END  # pipeline

BUFFER vertex kData TYPE vec3<int32> DATA
# Top-left red
-1 -1  0xff0000ff
 0 -1  0xff0000ff
-1  0  0xff0000ff
 0  0  0xff0000ff
# Top-right green
 0 -1  0xff00ff00
 1 -1  0xff00ff00
 0  0  0xff00ff00
 1  0  0xff00ff00
# Bottom-left blue
-1  0  0xffff0000
 0  0  0xffff0000
-1  1  0xffff0000
 0  1  0xffff0000
# Bottom-right purple
 0  0  0xff800080
 1  0  0xff800080
 0  1  0xff800080
 1  1  0xff800080
END

BUFFER index kIndices TYPE int32 DATA
0  1  2    2  1  3
4  5  6    6  5  7
8  9  10   10 9  11
12 13 14   14 13 15
END

CLEAR_COLOR kGraphicsPipeline 255 0 0 255
CLEAR kGraphicsPipeline

RUN kGraphicsPipeline \
  DRAW_ARRAY kIndices IN kBuffer AS triangle_list \
  START_IDX 0 COUNT 24
 ```
