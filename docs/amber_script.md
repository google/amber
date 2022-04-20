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
with:
 * `VariablePointerFeatures.variablePointers`
 * `VariablePointerFeatures.variablePointersStorageBuffer`
 * `Float16Int8Features.shaderFloat16`
 * `Float16Int8Features.shaderInt8`
 * `Storage8BitFeatures.storageBuffer8BitAccess`
 * `Storage8BitFeatures.uniformAndStorageBuffer8BitAccess`
 * `Storage8BitFeatures.storagePushConstant8`
 * `Storage16BitFeatures.storageBuffer16BitAccess`
 * `Storage16BitFeatures.uniformAndStorageBuffer16BitAccess`
 * `Storage16BitFeatures.storagePushConstant16`
 * `Storage16BitFeatures.storageInputOutput16`
 * `SubgroupSizeControl.subgroupSizeControl`
 * `SubgroupSizeControl.computeFullSubgroups`
 * `SubgroupSupportedOperations.basic`
 * `SubgroupSupportedOperations.vote`
 * `SubgroupSupportedOperations.arithmetic`
 * `SubgroupSupportedOperations.ballot`
 * `SubgroupSupportedOperations.shuffle`
 * `SubgroupSupportedOperations.shuffleRelative`
 * `SubgroupSupportedOperations.clustered`
 * `SubgroupSupportedOperations.quad`
 * `SubgroupSupportedStages.vertex`
 * `SubgroupSupportedStages.tessellationControl`
 * `SubgroupSupportedStages.tessellationEvaluation`
 * `SubgroupSupportedStages.geometry`
 * `SubgroupSupportedStages.fragment`
 * `SubgroupSupportedStages.compute`


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

### Virtual File Store

Each amber script contains a virtual file system that can store files of textual
data. This lets you bundle multiple source files into a single, hermetic amber
script file.

Virtual files are declared using the `VIRTUAL_FILE` command:

```groovy
VIRTUAL_FILE {path}
 {file-content}
END
```

Paths must be unique.

Shaders can directly reference these virtual files for their source. \
HLSL shaders that `#include` other `.hlsl` files will first check the virtual
file system, before falling back to the standard file system.

### Shaders

Shader programs are declared using the `SHADER` command. \
Shaders can be declared as `PASSTHROUGH`, with inlined source or using source
from a `VIRTUAL_FILE`.

Pass-through shader:

```groovy
# Creates a passthrough vertex shader. The shader passes the vec4 at input
# location 0 through to the `gl_Position`.
SHADER vertex {shader_name} PASSTHROUGH
```

Shader using inlined source:

```groovy
# Creates a shader of |shader_type| with the given |shader_name|. The shader
# will be of |shader_format|. The shader source then follows and is terminated
# with the |END| tag.
SHADER {shader_type} {shader_name} {shader_format} [ TARGET_ENV {target_env} ]
{shader_source}
END
```

Shader using source from `VIRTUAL_FILE`:

```groovy
# Creates a shader of |shader_type| with the given |shader_name|. The shader
# will be of |shader_format|. The shader will use the virtual file with |path|.
SHADER {shader_type} {shader_name} {shader_format} [ TARGET_ENV {target_env} ] VIRTUAL_FILE {path}
```

`{shader_name}` is used to identify the shader to attach to `PIPELINE`s,

`{shader_type}` and `{shader_format}` are described below:

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
 * `HLSL`  (with dxc or glslang if dxc disabled)
 * `SPIRV-ASM` (with spirv-as; specifying `TARGET_ENV` is _highly recommended_
    in this case, as explained below)
 * `SPIRV-HEX` (decoded straight to SPIR-V)
 * `OPENCL-C` (with clspv)

### Target environment

Specifying `TARGET_ENV` is optional and can be used to select a target
SPIR-V environment. For example:

 * `spv1.0`
 * `spv1.5`
 * `vulkan1.0`
 * `vulkan1.2`

Check the help text of the corresponding tool (e.g. spirv-as, glslangValidator)
for the full list. The `SPIRV-HEX` shader format is not affected by the target
environment.

The specified target environment for the shader overrides the default (`spv1.0`)
or the one specified on the command line.

Specifying the target environment when using the `SPIRV-ASM` shader format
is _highly recommended_, otherwise the SPIR-V version of the final SPIR-V binary
shader passed to the graphics device might not be what you expect.
Typically, SPIR-V assembly text will contain a comment near the beginning similar
to `; Version: 1.0` but this is _ignored_ by the spirv-as assembler.
Thus, you should specify the equivalent target environment (e.g. `spv1.0`)
in the `SHADER` command.

Specifying the target environment for other shader formats depends on whether
you want to vary the final SPIR-V shader binary based on the target environment
specified on the command line. For example, you could write one AmberScript file
that contains a GLSL shader without specifying a target environment.
You could then run the AmberScript file several times with different
target environments specified on the command line
(`spv1.0`, `spv1.1`, `spv1.2`, etc.) to test the different SPIR-V shader variants.

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
 * `float16`
 * `float`
 * `double`
 * vec[2,3,4]{type}
 * mat[2,3,4]x[2,3,4]{type}  (mat<columns>x<rows>)
 * Any of the `Image Formats` listed below.
 * For any of the non-Image Formats types above appending '[]' will treat the
    data as an array. e.g. int8[], vec2<float>[]

Sized arrays and structures are not currently representable.

```groovy
# Filling the buffer with a given initializer. Initializer data must be
# of |type|. Buffers are STD430 by default.
BUFFER {name} DATA_TYPE {type} {STD140 | STD430} {initializer}

# Defines a buffer which is filled with data as specified by the `initializer`.
BUFFER {name} DATA_TYPE {type} {STD140 | STD430} SIZE _size_in_items_ \
    {initializer}

# Deprecated
# Defines a buffer with width and height and filled by data as specified by the
# `initializer`.
BUFFER {name} DATA_TYPE {type} {STD140 | STD430} WIDTH {w} HEIGHT {h} \
  {initializer}

# Defines a buffer which is filled with binary data from a file specified
# by `FILE`.
BUFFER {name} DATA_TYPE {type} {STD140 | STD430} SIZE _size_in_items_ \
    FILE BINARY {file_name}

# Defines a buffer which is filled with text data parsed from a file specified
# by `FILE`.
BUFFER {name} DATA_TYPE {type} {STD140 | STD430} SIZE _size_in_items_ \
    FILE TEXT {file_name}

# Creates a buffer which will store the given `FORMAT` of data. These
# buffers are used as image and depth buffers in the `PIPELINE` commands.
# The buffer will be sized based on the `RENDER_SIZE` of the `PIPELINE`.
# For multisampled images use value greater than one for `SAMPLES`. Allowed
# sample counts are 1, 2, 4, 8, 16, 32, and 64. Note that Amber doesn't
# preserve multisampled images across pipelines.
BUFFER {name} FORMAT {format_string} \
    [ MIP_LEVELS _mip_levels_ (default 1) ] \
    [ SAMPLES _samples_ (default 1) ]

# Load buffer data from a PNG image with file name specified by `FILE`.
# The file path is relative to the script file being run. Format specified
# by `FORMAT` must match the image format.
BUFFER {name} FORMAT {format_string} FILE PNG {file_name.png}
```

#### Images

An AmberScript image is a specialized buffer that specifies image-specific
attributes.

##### Dimensionality
 * `DIM_1D` -- A 1-dimensional image
 * `DIM_2D` -- A 2-dimensional image
 * `DIM_3D` -- A 3-dimensional image

```groovy
# Specify an image buffer with a format. HEIGHT is necessary for DIM_2D and
# DIM_3D. DEPTH is necessary for DIM_3D.
IMAGE {name} FORMAT {format_string} [ MIP_LEVELS _mip_levels_ (default 1) ] \
    [ SAMPLES _samples_ (default 1) ] \
    {dimensionality} \
    WIDTH {w} [ HEIGHT {h} [ DEPTH {d} ] ] \
    {initializer}

# Specify an image buffer with a data type. HEIGHT is necessary for DIM_2D and
# DIM_3D. DEPTH is necessary for DIM_3D.
IMAGE {name} DATA_TYPE {type} {dimensionality} \
    WIDTH {w} [ HEIGHT {h} [ DEPTH {d} ] ] \
    {intializer}
```

#### Buffer Initializers

```groovy
# Filling the buffer with a given set of data. The values must be
# of the correct type. The data can be provided as the type or as a hex
# value.
DATA
_value_+
END

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

### Samplers

Samplers are used for sampling buffers that are bound to a pipeline as
sampled image or combined image sampler.

#### Filter types
 * `nearest`
 * `linear`

#### Address modes
 * `repeat`
 * `mirrored_repeat`
 * `clamp_to_edge`
 * `clamp_to_border`
 * `mirrored_clamp_to_edge`

#### Border colors
 * `float_transparent_black`
 * `int_transparent_black`
 * `float_opaque_black`
 * `int_opaque_black`
 * `float_opaque_white`
 * `int_opaque_white`

#### Compare operations
* `never`
* `less`
* `equal`
* `less_or_equal`
* `greater`
* `not_equal`
* `greater_or_equal`
* `always`

```groovy

# Creates a sampler with |name|. |compare_enable| is either on or off.
SAMPLER {name} \
    [ MAG_FILTER {filter_type} (default nearest) ] \
    [ MIN_FILTER {filter_type} (default nearest) ] \
    [ ADDRESS_MODE_U {address_mode} (default repeat) ] \
    [ ADDRESS_MODE_V {address_mode} (default repeat) ] \
    [ ADDRESS_MODE_W {address_mode} (default repeat) ] \
    [ BORDER_COLOR {border_color} (default float_transparent_black) ] \
    [ MIN_LOD _val_ (default 0.0) ] \
    [ MAX_LOD _val_ (default 1.0) ] \
    [ NORMALIZED_COORDS | UNNORMALIZED_COORDS (default NORMALIZED_COORDS) ] \
    [ COMPARE _compare_enable_ (default off) ] \
    [ COMPARE_OP _compare_op_ (default never) ]
```

Note: unnormalized coordinates will override MIN\_LOD and MAX\_LOD to 0.0.

#### OpenCL Literal Samplers

Literal constant samplers defined in the OpenCL program are automatically
generated and bound to the pipeline in Amber.

Note: currently the border color is always transparent black.

Note: the addressing mode is used for all coordinates currently. Arrayed images
should use `clamp_to_edge` for the array index.

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
  # Attach the shader provided by |name_of_shader| to the pipeline with an
  # entry point name of |name|. The provided shader for ATTACH must _not_ be
  # a 'multi' shader.
  ATTACH {name_of_shader} \
      [ ENTRY_POINT {name} (default "main") ]

  # Attach a 'multi' shader to the pipeline of |shader_type| and use the entry
  # point with |name|. The provided shader _must_ be a 'multi' shader.
  ATTACH {name_of_multi_shader} TYPE {shader_type} ENTRY_POINT {name}

  # Attach specialized shader. Specialization can be specified multiple times.
  # Specialization values must be a 32-bit type. Shader type and entry point
  # must be specified prior to specializing the shader.
  ATTACH {name_of_shader} SPECIALIZE _id_ AS uint32 _value_
  ATTACH {name_of_shader} \
      SPECIALIZE _id_ AS uint32 _value_ \
      SPECIALIZE _id_ AS float _value_
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
  # Set the polygon mode used for all drawing with the pipeline.
  # |mode| is fill, line, or point and it defaults to fill.
  POLYGON_MODE {mode}
```

```groovy
  # Set the number of patch control points used by tessellation. The default value is 3.
  PATCH_CONTROL_POINTS {control_points}
```

#### Compare operations
 * `never`
 * `less`
 * `equal`
 * `less_or_equal`
 * `greater`
 * `not_equal`
 * `greater_or_equal`
 * `always`

```groovy
  # Set depth test settings. All enable options are specified with keywords on and off.
  # BOUNDS and BIAS values are specified with decimal numbers. |compare_op| is selected
  # from the list of compare operations above.
  DEPTH
    TEST {test_enable}
    WRITE {write_enable}
    COMPARE_OP {compare_op}
    CLAMP {clamp_enable}
    BOUNDS min {bound_min} max {bounds_max}
    BIAS constant {bias_constant} clamp {bias_clamp} slope {bias_slope}
  END
```

#### Stencil operations
 * `keep`
 * `replace`
 * `increment_and_clamp`
 * `decrement_and_clamp`
 * `invert`
 * `increment_and_wrap`
 * `decrement_and_wrap`

```groovy
  # Set stencil test settings. |face| can be front, back, or front_and_back.
  # |test_enable| is either on or off and affects both faces. |fail_op|, |pass_op|,
  # and |depth_fail_op| are selected from the stencil operations table above,
  # and |compare_op| from the compare operations table. |compare_mask|, |write_mask|,
  # and |reference| are 8bit unsigned integer values (range 0..255).
  STENCIL {face}
    TEST {test_enable}
    FAIL_OP {fail_op}
    PASS_OP {pass_op}
    DEPTH_FAIL_OP {depth_fail_op}
    COMPARE_OP {compare_op}
    COMPARE_MASK {compare_mask}
    WRITE_MASK {write_mask}
    REFERENCE {reference}
  END
```

#### Blend factors
* `zero`
* `one`
* `src_color`
* `one_minus_src_color`
* `dst_color`
* `one_minus_dst_color`
* `src_alpha`
* `one_minus_src_alpha`
* `dst_alpha`
* `one_minus_dst_alpha`
* `constant_color`
* `one_minus_constant_color`
* `constant_alpha`
* `one_minus_constant_alpha`
* `src_alpha_saturate`
* `src1_color`
* `one_minus_src1_color`
* `src1_alpha`
* `one_minus_src1_alpha`

#### Blend operations
* `add`
* `substract`
* `reverse_substract`
* `min`
* `max`

The following operations also require VK_EXT_blend_operation_advanced
when using a Vulkan backend.
* `zero`
* `src`
* `dst`
* `src_over`
* `dst_over`
* `src_in`
* `dst_in`
* `src_out`
* `dst_out`
* `src_atop`
* `dst_atop`
* `xor`
* `multiply`
* `screen`
* `overlay`
* `darken`
* `lighten`
* `color_dodge`
* `color_burn`
* `hard_light`
* `soft_light`
* `difference`
* `exclusion`
* `invert`
* `invert_rgb`
* `linear_dodge`
* `linear_burn`
* `vivid_light`
* `linear_light`
* `pin_light`
* `hard_mix`
* `hsl_hue`
* `hsl_saturation`
* `hsl_color`
* `hsl_luminosity`
* `plus`
* `plus_clamped`
* `plus_clamped_alpha`
* `plus_darker`
* `minus`
* `minus_clamped`
* `contrast`
* `invert_org`
* `red`
* `green`
* `blue`

```groovy
  # Enable alpha blending and set blend factors and operations. Available
  # blend factors and operations are listed above.
  BLEND
    SRC_COLOR_FACTOR {src_color_factor}
    DST_COLOR_FACTOR {dst_color_factor}
    COLOR_OP {color_op}
    SRC_ALPHA_FACTOR {src_alpha_factor}
    DST_ALPHA_FACTOR {dst_alpha_factor}
    ALPHA_OP {alpha_op}
  END
```

```groovy
  # Set the size of the render buffers. |width| and |height| are integers and
  # default to 250x250.
  FRAMEBUFFER_SIZE _width_ _height_
```

```groovy
  # Set the viewport size. If no viewport is provided then it defaults to the
  # whole framebuffer size. Depth range defaults to 0 to 1.
  VIEWPORT {x} {y} SIZE {width} {height} [MIN_DEPTH {mind}] [MAX_DEPTH {maxd}]
```

```groovy
  # Set subgroup size control setting. Require that subgroups must be launched
  # with all invocations active for given shader. Allow SubgroupSize to vary
  # for given shader. Require a specific SubgroupSize the for given shader.
  # |fully_populated_enable| and |varying_size_enable| can be on or off.
  # |subgroup_size| can be set one of the values below:
  #  - a power-of-two integer that _must_ be greater or equal to minSubgroupSize
  #    and be less than or equal to maxSubgroupSize
  # - MIN to set the required subgroup size to the minSubgroupSize
  # - MAX to set the required subgroup size to the maxSubgroupSize
  SUBGROUP {name_of_shader}
    FULLY_POPULATED {fully_populated_enable}
    VARYING_SIZE {varying_size_enable}
    REQUIRED_SIZE {subgroup_size}
  END
```

### Pipeline Buffers

#### Buffer Types
 * `uniform`
 * `storage`
 * `uniform_dynamic`
 * `storage_dynamic`
 * `uniform_texel_buffer`
 * `storage_texel_buffer`

TODO(dsinclair): Sync the BufferTypes with the list of Vulkan Descriptor types.

A `pipeline` can have buffers or samplers bound. This includes buffers to
contain image attachment content, depth/stencil content, uniform buffers, etc.

```groovy
  # Attach |buffer_name| as an output color attachment at location |idx|.
  # The provided buffer must be a `FORMAT` buffer. If no color attachments are
  # provided a single attachment with format `B8G8R8A8_UNORM` will be created
  # for graphics pipelines. The MIP level will have a base of |level|.
  BIND BUFFER {buffer_name} AS color LOCATION _idx_ \
      [ BASE_MIP_LEVEL _level_ (default 0) ]

  # Attach |buffer_name| as the depth/stencil buffer. The provided buffer must
  # be a `FORMAT` buffer. If no depth/stencil buffer is specified a default
  # buffer of format `D32_SFLOAT_S8_UINT` will be created for graphics
  # pipelines.
  BIND BUFFER {buffer_name} AS depth_stencil

  # Attach |buffer_name| as a multisample resolve target. The order of resolve
  # target images match with the order of color attachments that have more than
  # one sample.
  BIND BUFFER {buffer_name} AS resolve

  # Attach |buffer_name| as the push_constant buffer. There can be only one
  # push constant buffer attached to a pipeline.
  BIND BUFFER {buffer_name} AS push_constant

  # Bind OpenCL argument buffer by name. Specifying the buffer type is optional.
  # Amber will set the type as appropriate for the argument buffer. All uses
  # of the buffer must have a consistent |buffer_type| across all pipelines.
  BIND BUFFER {buffer_name} [ AS {buffer_type} (default computed)] \
      KERNEL ARG_NAME _name_

  # Bind OpenCL argument buffer by argument ordinal. Arguments use 0-based
  # numbering. Specifying the buffer type is optional. Amber will set the
  # type as appropriate for the argument buffer. All uses of the buffer
  # must have a consistent |buffer_type| across all pipelines.
  BIND BUFFER {buffer_name} [ AS {buffer_type} (default computed)] \
      KERNEL ARG_NUMBER _number_

  # Bind OpenCL argument sampler by argument name.
  BIND SAMPLER {sampler_name} KERNEL ARG_NAME _name_

  # Bind OpenCL argument sampler by argument ordinal. Arguments use 0-based
  # numbering.
  BIND SAMPLER {sampler_name} KERNEL ARG_NUMBER _number_
```

All BIND BUFFER and BIND SAMPLER commands below define a descriptor set and binding ID.
These commands can be replaced with BIND BUFFER_ARRAY and BIND SAMPLER_ARRAY commands.
In these cases multiple buffer or sampler names need to be provided, separated by spaces.
This creates a descriptor array of buffers or samplers bound to the same descriptor set
and binding ID. An array of dynamic offsets should be provided via `OFFSET offset1 offset2 ...`
when using dynamic buffers with BUFFER_ARRAY. Optional descriptor binding offset(s) and range(s)
can be defined via `DESCRIPTOR_OFFSET offset1 offset2 ...` and 
`DESCRIPTOR_RANGE range1 range2 ...` when using uniform or storage buffers. Offsets and 
ranges can be used also with dynamic buffers.
```groovy
  # Bind the buffer of the given |buffer_type| at the given descriptor set
  # and binding. The buffer will use a byte offset |descriptor_offset| 
  # with range |range|.
  BIND {BUFFER | BUFFER_ARRAY} {buffer_name} AS {buffer_type} DESCRIPTOR_SET _id_ \
       BINDING _id_ [ DESCRIPTOR_OFFSET _descriptor_offset_ (default 0) ] \ 
       [ DESCRIPTOR_RANGE _range_ (default -1 == VK_WHOLE_SIZE) ]

  # Attach |buffer_name| as a storage image. The MIP level will have a base
  # value of |level|.
  BIND {BUFFER | BUFFER_ARRAY} {buffer_name} AS storage_image \
      DESCRIPTOR_SET _id_ BINDING _id_ [ BASE_MIP_LEVEL _level_ (default 0) ]

  # Attach |buffer_name| as a sampled image.  The MIP level will have a base
  # value of |level|.
  BIND {BUFFER | BUFFER_ARRAY} {buffer_name} AS sampled_image \
      DESCRIPTOR_SET _id_ BINDING _id_ [ BASE_MIP_LEVEL _level_ (default 0) ]

  # Attach |buffer_name| as a combined image sampler. A sampler |sampler_name|
  # must also be specified. The MIP level will have a base value of 0.
  BIND {BUFFER | BUFFER_ARRAY} {buffer_name} AS combined_image_sampler SAMPLER {sampler_name} \
      DESCRIPTOR_SET _id_ BINDING _id_ [ BASE_MIP_LEVEL _level_ (default 0) ]

  # Bind the sampler at the given descriptor set and binding.
  BIND {SAMPLER | SAMPLER_ARRAY} {sampler_name} DESCRIPTOR_SET _id_ BINDING _id_

  # Bind |buffer_name| as dynamic uniform/storage buffer at the given descriptor set
  # and binding. The buffer will use a byte offset |offset| + |descriptor_offset|
  # with range |range|.
  BIND {BUFFER | BUFFER_ARRAY} {buffer_name} AS {uniform_dynamic | storage_dynamic} \
       DESCRIPTOR_SET _id_ BINDING _id_ OFFSET _offset_ \
       [ DESCRIPTOR_OFFSET _descriptor_offset_ (default 0) ] \ 
       [ DESCRIPTOR_RANGE _range_ (default -1 == VK_WHOLE_SIZE) ]
```

```groovy
  # Set |buffer_name| as the vertex data at location |val|. RATE defines the
  # input rate for vertex attribute reading. OFFSET sets the byte offset for the
  # vertex data within the buffer |buffer_name|, which by default is 0. FORMAT
  # sets the vertex buffer format, which by default is the format of the buffer
  # |buffer_name|. STRIDE sets the byte stride, which by default is the stride
  # of the format (set explicitly via FORMAT or from the format of the buffer
  # |buffer_name|).
  VERTEX_DATA {buffer_name} LOCATION _val_ [ RATE { vertex | instance } (default vertex) ] \
        [ FORMAT {format} ] [ OFFSET {offset} ] [ STRIDE {stride} ]

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
 * `POINT_LIST`
 * `LINE_LIST`
 * `LINE_LIST_WITH_ADJACENCY`
 * `LINE_STRIP`
 * `LINE_STRIP_WITH_ADJACENCY`
 * `TRIANGLE_LIST`
 * `TRIANGLE_LIST_WITH_ADJACENCY`
 * `TRIANGLE_STRIP`
 * `TRIANGLE_STRIP_WITH_ADJACENCY`
 * `TRIANGLE_fan`
 * `PATCH_LIST`

### Run a pipeline.

When running a `DRAW_ARRAY` command, you must attach the vertex data to the
`PIPELINE` with the `VERTEX_DATA` command.

To run an indexed draw, attach the index data to the `PIPELINE` with an
`INDEX_DATA` command.

For the commands which take a `START_IDX` and a `COUNT` they can be left off the
command (although, `START_IDX` is required if `COUNT` is provided). The default
value for `START_IDX` is 0. The default value for `COUNT` is the item count of
vertex buffer minus the `START_IDX`. The same applies to `START_INSTANCE`
(default 0) and `INSTANCE_COUNT` (default 1).

```groovy
# Run the given |pipeline_name| which must be a `compute` pipeline. The
# pipeline will be run with the given number of workgroups in the |x|, |y|, |z|
# dimensions. Each of the x, y and z values must be a uint32.
RUN {pipeline_name} _x_ _y_ _z_
```

```groovy
# Run the given |pipeline_name| which must be a `graphics` pipeline. The
# rectangle at |x|, |y|, |width|x|height| will be rendered. Ignores VERTEX_DATA
# and INDEX_DATA on the given pipeline.
RUN {pipeline_name} \
  DRAW_RECT POS _x_in_pixels_ _y_in_pixels_ \
  SIZE _width_in_pixels_ _height_in_pixels_
```

```groovy
# Run the given |pipeline_name| which must be a `graphics` pipeline. The
# grid at |x|, |y|, |width|x|height|, |columns|x|rows| will be rendered.
# Ignores VERTEX_DATA and INDEX_DATA on the given pipeline.
# For columns, rows of (5, 4) a total of 5*4=20 rectangles will be drawn.
RUN {pipeline_name} \
  DRAW_GRID POS _x_in_pixels_ _y_in_pixels_ \
  SIZE _width_in_pixels_ _height_in_pixels_ \
  CELLS _columns_of_cells_ _rows_of_cells_
```

```groovy
# Run the |pipeline_name| which must be a `graphics` pipeline. The vertex
# data must be attached to the pipeline.

# A start index of |value| will be used and the count of |count_value| items
# will be processed. The draw is instanced if |inst_count_value| is greater
# than one. In case of instanced draw |inst_value| controls the starting
# instance ID.
RUN {pipeline_name} DRAW_ARRAY AS {topology} \
    [ START_IDX _value_ (default 0) ] \
    [ COUNT _count_value_ (default vertex_buffer size - start_idx) ] \
    [ START_INSTANCE _inst_value_ (default 0) ] \
    [ INSTANCE_COUNT _inst_count_value_ (default 1) ]
```

```groovy
# Run the |pipeline_name| which must be a `graphics` pipeline. The vertex
# data and  index data must be attached to the pipeline. The vertices will be
# drawn using the given |topology|.
#
# A start index of |value| will be used and the count of |count_value| items
# will be processed. The draw is instanced if |inst_count_value| is greater
# than one. In case of instanced draw |inst_value| controls the starting
# instance ID.
RUN {pipeline_name} DRAW_ARRAY AS {topology} INDEXED \
    [ START_IDX _value_ (default 0) ] \
    [ COUNT _count_value_ (default index_buffer size - start_idx) ] \
    [ START_INSTANCE _inst_value_ (default 0) ] \
    [ INSTANCE_COUNT _inst_count_value_ (default 1) ]
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
  * `CLEAR_DEPTH`
  * `CLEAR_STENCIL`
  * `COPY`
  * `EXPECT`
  * `RUN`

### Commands

```groovy
# Sets the clear color to use for |pipeline| which must be a graphics
# pipeline. The colors are integers from 0 - 255.  Defaults to (0, 0, 0, 0)
CLEAR_COLOR {pipeline} _r (0 - 255)_ _g (0 - 255)_ _b (0 - 255)_ _a (0 - 255)_

# Sets the depth clear value to use for |pipeline| which must be a graphics
# pipeline. |value| must be a decimal number.
CLEAR_DEPTH {pipeline} _value_

# Sets the stencil clear value to use for |pipeline| which must be a graphics
# pipeline. |value| must be an integer from 0 - 255.
CLEAR_STENCIL {pipeline} _value_

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

RUN kGraphicsPipeline DRAW_ARRAY AS TRIANGLE_LIST START_IDX 0 COUNT 24
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
