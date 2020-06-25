" Vim syntax file
" Language: Amber Script

if exists("b:current_syntax")
  finish
endif

" Regular int like number with - + or nothing in front
syn match amberNumber '\d\+' contained display
syn match amberNumber '[-+]\d\+' contained display

" Floating point number with decimal no E or e (+,-)
syn match amberNumber '\d\+\.\d*' contained display
syn match amberNumber '[-+]\d\+\.\d*' contained display

" Floating point like number with E and no decimal point (+,-)
syn match amberNumber '[-+]\=\d[[:digit:]]*[eE][\-+]\=\d\+' contained display
syn match amberNumber '\d[[:digit:]]*[eE][\-+]\=\d\+' contained display

" Floating point like number with E and decimal point (+,-)
syn match amberNumber '[-+]\=\d[[:digit:]]*\.\d*[eE][\-+]\=\d\+' contained display
syn match amberNumber '\d[[:digit:]]*\.\d*[eE][\-+]\=\d\+' contained display

syn region amberString start='"' end='"' contained
syn region amberDesc start='"' end='"'

syn keyword amberTodo contained TODO FIXME
syn match amberBang "\v#!.*$"
syn match amberComment "#.*$" contains=amberTodo

syn keyword amberBlockCmd SHADER BUFFER PIPELINE ATTACH END RUN TYPE SIZE FILL
syn keyword amberBlockCmd DESCRIPTOR_SET BINDING IDX TO EXPECT PASSTHROUGH
syn keyword amberBlockCmd DATA_TYPE DATA SERIES_FROM DRAW_ARRAY IN START_IDX
syn keyword amberBlockCmd COUNT CLEAR CLEAR_COLOR AS POS DRAW_RECT INC_BY
syn keyword amberBlockCmd FRAMEBUFFER ENTRY_POINT SHADER_OPTIMIZATION
syn keyword amberBlockCmd FORMAT FRAMEBUFFER_SIZE LOCATION BIND SAMPLER
syn keyword amberBlockCmd VERTEX_DATA INDEX_DATA INDEXED IMAGE_ATTACHMENT
syn keyword amberBlockCmd DEPTH_STENCIL_ATTACHMENT DEVICE_FEATURE TOLERANCE
syn keyword amberBlockCmd REPEAT COPY DERIVE_PIPELINE FROM STD140 STD430
syn keyword amberBlockCmd DEVICE_EXTENSION IMAGE INSTANCE_EXTENSION SET
syn keyword amberBlockCmd STRUCT VIRTUAL_FILE CLEAR_DEPTH CLEAR_STENCIL
syn keyword amberBlockCmd DEBUG TARGET_ENV SHADER_OPTIMIZATION COMPILE_OPTIONS
syn keyword amberBlockCmd POLYGON_MODE DEPTH STENCIL SUBGROUP SPECIALIZE
syn keyword amberBlockCmd FULLY_POPULATED VARYING_SIZE REQUIRED_SIZE
syn keyword amberBlockCmd MIN MAX BUFFER_ARRAY SAMPLER_ARRAY KERNEL OFFSET
syn keyword amberBlockCmd BASE_MIP_LEVEL ARG_NUMBER RATE TEST CLAMP
syn keyword amberBlockCmd WRITE COMPARE_OP BOUNDS BIAS FAIL_OP PASS_OP
syn keyword amberBlockCmd DEPTH_FAIL_OP COMPARE_MASK WRITE_MASK REFERENCE
syn keyword amberBlockCmd STRIDE ARRAY_STRIDE MATRIX_STRIDE MIP_LEVELS
syn keyword amberBlockCmd FILE DIM_1D DIM_2D DIM_3D WIDTH HEIGHT SAMPLES
syn keyword amberBlockCmd TEXT BINARY PNG DRAW_GRID CELLS INSTANCE_COUNT
syn keyword amberBlockCmd START_INSTANCE THREAD GLOBAL_INVOCATION_ID
syn keyword amberBlockCmd VERTEX_INDEX FRAGMENT_WINDOW_SPACE_POSITION
syn keyword amberBlockCmd CALLSTACK STEP_IN STEP_OUT STEP_OVER CONTINUE TO
syn keyword amberBlockCmd ENGINE_DATA MAG_FILTER MIN_FILTER ADDRESS_MODE_U
syn keyword amberBlockCmd ADDRESS_MODE_V ADDRESS_MODE_W BORDER_COLOR
syn keyword amberBlockCmd MIN_LOD MAX_LOD NORMALIZED_COORDS UNNORMALIZED_COORDS
syn keyword amberBlockCmd ARG_NAME

syn keyword amberComparator EQ NE LT LE GT GE EQ_RGB EQ_RGBA EQ_BUFFER RMSE_BUFFER
syn keyword amberComparator EQ_HISTOGRAM_EMD_BUFFER

syn keyword amberKeyword compute vertex geometry fragment graphics
syn keyword amberKeyword tessellation_evaulation tessellation_control multi

syn keyword amberFormat GLSL HLSL SPIRV-ASM SPIRV-HEX OPENCL-C

syn keyword amberTopology point_list line_list line_list_with_adjacency
syn keyword amberTopology line_strip line_strip_with_adjacency triangle_list
syn keyword amberTopology triangle_list_with_adjacench triangle_strip
syn keyword amberTopology triangle_strip_with_adjacency triangle_fan patch_list

syn keyword amberBufferType uniform storage push_constant color depth_stencil
syn keyword amberBufferType uniform_dynamic storage_dynamic combined_image_sampler
syn keyword amberBufferType storage_image sampled_image uniform_texel_buffer
syn keyword amberBufferType storage_texel_buffer

syn keyword amberAddressMode repeat mirrored_repeat clamp_to_edge clamp_to_border
syn keyword amberAddressMode mirror_clamp_to_edge

syn keyword amberCompareOp never less equal less_or_equal greater not_equal
syn keyword amberCompareOp greater_or_equal always

syn keyword amberStencilOp keep zero replace increment_and_clamp decrement_and_clamp
syn keyword amberStencilOp invert increment_and_wrap decrement_and_wrap

syn keyword amberBorderColor float_transparent_black int_transparent_black
syn keyword amberBorderColor float_opaque_black int_opaque_black float_opaque_white
syn keyword amberBorderColor int_opaque_white

syn keyword amberFace front back front_and_back

syn keyword amberFilter nearest linear

syn keyword amberPolygonMode fill line point

let b:current_syntax = "amber"
hi def link amberTodo Todo
hi def link amberBang Comment
hi def link amberComment Comment
hi def link amberBlockCmd Keyword
hi def link amberString String
hi def link amberNumber Number
hi def link amberDesc PreProc
hi def link amberKeyword String
hi def link amberFormat Type
hi def link amberComparator Keyword
hi def link amberTopology Type
hi def link amberBufferType Type
hi def link amberAddressMode Type
hi def link amberCompareOp Type
hi def link amberStencilOp Type
hi def link amberBorderColor Type
hi def link amberFace Type
hi def link amberFilter Type
hi def link amberPolygonMode Type
