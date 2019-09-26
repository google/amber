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

syn keyword amberComparator EQ NE LT LE GT GE EQ_RGB EQ_RGBA EQ_BUFFER RMSE_BUFFER

syn keyword amberKeyword compute vertex geometry fragment graphics
syn keyword amberKeyword tessellation_evaulation tessellation_control multi

syn keyword amberFormat GLSL HLSL SPIRV-ASM SPIRV-HEX OPENCL-C

syn keyword amberTopology point_list line_list line_list_with_adjacency
syn keyword amberTopology line_strip line_strip_with_adjacency triangle_list
syn keyword amberTopology triangle_list_with_adjacench triangle_strip
syn keyword amberTopology triangle_strip_with_adjacency triangle_fan patch_list

syn keyword amberBufferType uniform storage push_constant color depth_stencil

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
