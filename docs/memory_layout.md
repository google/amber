# Memory Layout Examples

Note, this is how we'll layout the data in Amber. Some of these types don't
exist in standard GLSL (they exist in GL_EXT_shader_explicit_arithmetic_types).
We allow them in Amber to give flexibility in the data provided, but attempt to
follow the layout rules with all types for consistency.

For the purposes of this document, all scalars are stored in little-endian. The
lower number components of a vector appear earlier than higher numbered ones.
Similar for columns and rows of matrices.

All type names used in the file are the Amber names. They differ from GLSL but
are a bit more explicit, so are used here for clarity.

## Scalars

| Name | Bytes |
|------|-------|
| int8    | <kbd>[b]</kbd> |
| uint8   | <kbd>[b]</kbd> |
| int16   | <kbd>[bb]</kbd> |
| uint16  | <kbd>[bb]</kbd> |
| float16 | <kbd>[bb]</kbd> |
| int32   | <kbd>[bbbb]</kbd> |
| uint32  | <kbd>[bbbb]</kbd> |
| float   | <kbd>[bbbb]</kbd> |
| int64   | <kbd>[bbbb][bbbb]</kbd> |
| uint64  | <kbd>[bbbb][bbbb]</kbd> |
| double  | <kbd>[bbbb][bbbb]</kbd> |

## Vectors
### STD140 & STD430

| Name | Bytes |
|------|-------|
| vec2\<float> | <kbd>[bbbb][bbbb]</kbd> |
| vec3\<float> | <kbd>[bbbb][bbbb][bbbb][----]</kbd> |
| vec4\<float> | <kbd>[bbbb][bbbb][bbbb][bbbb]</kbd> |
| vec2\<int8>  | <kbd>[bb]</kbd> |
| vec3\<int8>  | <kbd>[bbb-]</kbd> |
| vec4\<int8>  | <kbd>[bbbb]</kbd> |
| vec2\<int16> | <kbd>[bbbb]</kbd> |
| vec3\<int16> | <kbd>[bbbb][bb--]</kbd> |
| vec4\<int16> | <kbd>[bbbb][bbbb]</kbd> |
| vec2\<int32> | <kbd>[bbbb][bbbb]</kbd> |
| vec3\<int32> | <kbd>[bbbb][bbbb][bbbb][----]</kbd> |
| vec4\<int32> | <kbd>[bbbb][bbbb][bbbb][bbbb]</kbd> |


## Scalar Arrays
### STD140
| Name | Bytes |
|------|-------|
| int8[]  | <kbd>[b---][----][----][----]</kbd> |
| int16[] | <kbd>[bb--][----][----][----]</kbd> |
| int32[] | <kbd>[bbbb][----][----][----]</kbd> |
| int64[] | <kbd>[bbbb][bbbb][----][----]</kbd> |
| float[] | <kbd>[bbbb][----][----][----]</kbd> |

### STD430
| Name | Bytes |
|------|-------|
| int8[]  | <kbd>[b]</kbd> |
| int16[] | <kbd>[bb]</kbd> |
| int32[] | <kbd>[bbbb]</kbd> |
| int64[] | <kbd>[bbbb][bbbb]</kbd> |
| float[] | <kbd>[bbbb]</kbd> |


## Vector Arrays
### STD140
| Name | Bytes |
|------|-------|
| vec2\<float>[] | <kbd>[bbbb][bbbb][----][----]</kbd> |
| vec3\<float>[] | <kbd>[bbbb][bbbb][bbbb][----]</kbd> |
| vec4\<float>[] | <kbd>[bbbb][bbbb][bbbb][bbbb]</kbd> |
| vec2\<int8>[]  | <kbd>[bb--][----][----][----]</kbd> |
| vec2\<int16>[] | <kbd>[bbbb][----][----][----]</kbd> |
| vec2\<int32>[] | <kbd>[bbbb][bbbb][----][----]</kbd> |
| vec3\<int8>[]  | <kbd>[bbb-][----][----][----]</kbd> |
| vec3\<int16>[] | <kbd>[bbbb][bb--][----][----]</kbd> |
| vec3\<int32>[] | <kbd>[bbbb][bbbb][bbbb][----]</kbd> |
| vec4\<int8>[]  | <kbd>[bbbb][----][----][----]</kbd> |
| vec4\<int16>[] | <kbd>[bbbb][bbbb][----][----]</kbd> |
| vec4\<int32>[] | <kbd>[bbbb][bbbb][bbbb][bbbb]</kbd> |

### STD430
| Name | Bytes |
|------|-------|
| vec2\<float>[] | <kbd>[bbbb][bbbb]</kbd> |
| vec3\<float>[] | <kbd>[bbbb][bbbb][bbbb][----]</kbd> |
| vec4\<float>[] | <kbd>[bbbb][bbbb][bbbb][bbbb]</kbd> |
| vec2\<int8>[]  | <kbd>[bb]</kbd> |
| vec2\<int16>[] | <kbd>[bbbb]</kbd> |
| vec2\<int32>[] | <kbd>[bbbb][bbbb]</kbd> |
| vec3\<int8>[]  | <kbd>[bbb-]</kbd> |
| vec3\<int16>[] | <kbd>[bbbb][bb--]</kbd> |
| vec3\<int32>[] | <kbd>[bbbb][bbbb][bbbb][----]</kbd> |
| vec4\<int8>[]  | <kbd>[bbbb]</kbd> |
| vec4\<int16>[] | <kbd>[bbbb][bbbb]</kbd> |
| vec4\<int32>[] | <kbd>[bbbb][bbbb][bbbb][bbbb]</kbd> |


## Matrices (All matrices are column-major matrices, format is matCxR)

Note, GLSL does not have matrices with integer components although they exist in
other shader languages.

### STD140
| Name | Bytes |
|------|-------|
| mat2x2\<int8>  | <kbd>[bb--][----][----][----]<br/>[bb--][----][----][----]</kbd> |
| mat2x2\<fp16>  | <kbd>[bbbb][----][----][----]<br/>[bbbb][----][----][----]</kbd> |
| mat2x2\<float> | <kbd>[bbbb][bbbb][----][----]<br/>[bbbb][bbbb][----][----]</kbd> |
| mat2x3\<float> | <kbd>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]</kbd> |
| mat2x4\<float> | <kbd>[bbbb][bbbb][bbbb][bbbb]<br/>[bbbb][bbbb][bbbb][bbbb]</kbd> |
| mat3x2\<float> | <kbd>[bbbb][bbbb][----][----]<br/>[bbbb][bbbb][----][----]<br/>[bbbb][bbbb][----][----]</kbd> |
| mat4x2\<float> | <kbd>[bbbb][bbbb][----][----]<br/>[bbbb][bbbb][----][----]<br/>[bbbb][bbbb][----][----]<br/>[bbbb][bbbb][----][----]</kbd> |
| mat4x3\<float> | <kbd>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]</kbd> |

### STD430
| Name | Bytes |
|------|-------|
| mat2x2\<fp16>  | <kbd>[bbbb]<br/>[bbbb]</kbd> |
| mat2x2\<float> | <kbd>[bbbb][bbbb]<br/>[bbbb][bbbb]</kbd> |
| mat2x3\<float> | <kbd>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]</kbd> |
| mat2x4\<float> | <kbd>[bbbb][bbbb][bbbb][bbbb]<br/>[bbbb][bbbb][bbbb][bbbb]</kbd> |
| mat2x2\<int8>  | <kbd>[bb]<br/>[bb]</kbd> |
| mat3x2\<float> | <kbd>[bbbb][bbbb]<br/>[bbbb][bbbb]<br/>[bbbb][bbbb]</kbd> |
| mat3x3\<float> | <kbd>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]</kbd> |
| mat4x2\<float> | <kbd>[bbbb][bbbb][bbbb][bbbb]<br/>[bbbb][bbbb][bbbb][bbbb]</kbd> |
| mat4x3\<float> | <kbd>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]</kbd> |


## Matrix Array (All matrices are column-major matrices).

In the examples shown the array stride is equal to the base alignment of the matrix
so no extra padding is added between elements.

### STD140
| Name | Bytes |
|------|-------|
| mat2x2\<int8>[]  | <kbd>[bb--][----][----][----]<br/>[bb--][----][----][----]</kbd> |
| mat2x2\<fp16>[]  | <kbd>[bbbb][----][----][----]<br/>[bbbb][----][----][----]</kbd> |
| mat2x2\<float>[] | <kbd>[bbbb][bbbb][----][----]<br/>[bbbb][bbbb][----][----]</kbd> |
| mat2x3\<float>[] | <kbd>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]</kbd> |
| mat2x4\<float>[] | <kbd>[bbbb][bbbb][bbbb][bbbb]<br/>[bbbb][bbbb][bbbb][bbbb]</kbd> |
| mat3x2\<float>[] | <kbd>[bbbb][bbbb][----][----]<br/>[bbbb][bbbb][----][----]<br/>[bbbb][bbbb][----][----]</kbd> |
| mat4x2\<float>[] | <kbd>[bbbb][bbbb][----][----]<br/>[bbbb][bbbb][----][----]<br/>[bbbb][bbbb][----][----]<br/>[bbbb][bbbb][----][----]</kbd> |
| mat4x3\<float>[] | <kbd>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]</kbd> |

### STD430
| Name | Bytes |
|------|-------|
| mat2x2\<fp16>[]  | <kbd>[bbbb]<br/>[bbbb]</kbd> |
| mat2x2\<float>[] | <kbd>[bbbb][bbbb]<br/>[bbbb][bbbb]</kbd> |
| mat2x3\<float>[] | <kbd>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]</kbd> |
| mat2x4\<float>[] | <kbd>[bbbb][bbbb][bbbb][bbbb]<br/>[bbbb][bbbb][bbbb][bbbb]</kbd> |
| mat2x2\<int8>[]  | <kbd>[bb]<br/>[bb]</kbd> |
| mat3x2\<float>[] | <kbd>[bbbb][bbbb][bbbb][bbbb]<br/>[bbbb][bbbb]</kbd> |
| mat3x3\<float>[] | <kbd>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]</kbd> |
| mat4x2\<float>[] | <kbd>[bbbb][bbbb][bbbb][bbbb]<br/>[bbbb][bbbb][bbbb][bbbb]</kbd> |
| mat4x3\<float>[] | <kbd>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]<br/>[bbbb][bbbb][bbbb][----]</kbd> |

## Structures
```
struct {
  int32 w;
  float x;
}
```

The STD140 pads 8 bytes at the end to become a multiple of 16 bytes.

| STD | Array Stride | Bytes |
|-----|:--------------------:|-------|
| 140 | 16 | <kbd>{w [bbbb]}<br/>{x [bbbb]}<br/> [----][----]</kbd> |
| 430 |  8 | <kbd>{w [bbbb]}<br/>{x [bbbb]}</kbd> |

<hr>

```
struct {
  struct {
    int32 a;
    float b;
  } x;
  float y;
}
```

The STD140 pads 8 bytes at the end to become a multiple of 16 bytes.

| STD | Array Stride | Bytes |
|-----|:--------------------:|-------|
| 140 | 32 | <kbd>{x {a [bbbb]}<br/> &nbsp;&nbsp; {b [bbbb]}<br/> &nbsp;&nbsp; [----][----]<br/>{y [bbbb][----][----][----]}</kbd> |
| 430 | 12 | <kbd>{x {a [bbbb]}<br/> &nbsp;&nbsp; {b [bbbb]}<br/>{y [bbbb]}</kbd> |


<hr>

```
struct {
  int32 w;
  vec2<float> x;
  float y;
}
```

For both cases, the `vec2` is the member with the largest base alignment
requirement, so giving a base alignment of 8 bytes. The STD140 cases pads 8
bytes at the end (in the array element case) to become a multiple of 16.

| STD | Array Stride  | Bytes |
|-----|:---------------------:|-------|
| 140 | 32 | <kbd>{w [bbbb][----]}<br/>{x [bbbb][bbbb]}<br/>{y [bbbb][----]}<br/> [----][----]</kbd> |
| 430 | 24 | <kbd>{w [bbbb][----]}<br/>{x [bbbb][bbbb]}<br/>{y [bbbb][----]}</kbd> |

<hr>


```
struct {
  int32 w;
  vec3<float> x;
  float y;
}
```

The `vec3` expands to a `vec4`. This gives a base alignment of 16 bytes. So
the `w` pads to 16 bytes. the `float y` is packed into the `vec3` as there is
space (effectively making it a vec4).


| STD | Array Stride | Bytes |
|-----|:--------------------:|-------|
| 140 | 32 | <kbd>{w [bbbb][----][----][----]}<br/>{x [bbbb][bbbb][bbbb]}<br/>{y [bbbb]}</kbd> |
| 430 | 32 | <kbd>{w [bbbb][----][----][----]}<br/>{x [bbbb][bbbb][bbbb]}<br/>{y [bbbb]}</kbd> |

<hr>


```
struct {
  int32 w;
  vec3<float> x;
  vec2<float> y;
}
```

The `vec3` expands to a `vec4`. This gives a base alignment of 16 bytes. So
the `w` pads to 16 bytes.


| STD | Array Stride | Bytes |
|-----|:--------------------:|-------|
| 140 | 48 | <kbd>{w [bbbb][----][----][----]}<br/>{x [bbbb][bbbb][bbbb][----]}<br/>{y [bbbb][bbbb]}<br/>[----][----]</kbd> |
| 430 | 48 | <kbd>{w [bbbb][----][----][----]}<br/>{x [bbbb][bbbb][bbbb][----]}<br/>{y [bbbb][bbbb]}<br/>[----][----]</kbd> |

<hr>


```
struct {
  int32 w;
  mat2x2<float> x;
  float y;
}
```

In STD140 the `mat2x2<float>` has a base alignment of 16 bytes (the mat2x2 is, effectively,
an array of vec2's The vec2's have a size of 8 bytes this size rounds up to a vec4, so 16 bytes).

In STD430, the round up doesn't happen, so the base alignment is 8 bytes.

| STD | Array Stride  | Bytes |
|-----|:---------------------:|-------|
| 140 | 64 | <kbd>{w [bbbb][----][----][----]}<br/>{x [bbbb][bbbb][----][----]<br/> &nbsp;&nbsp; [bbbb][bbbb][----][----]}<br/>{y [bbbb][----][----][----]}</kbd> |
| 430 | 32 | <kbd>{w [bbbb][----]}<br/>{x [bbbb][bbbb][bbbb][bbbb]}<br/>{y [bbbb][----]}</kbd> |

<hr>


```
struct {
  int32 w;
  struct {
    int32 a;
    int32 b;
    float c;
  } x;
  float y;
}
```

The base alignment of the largest item is 4 bytes. In STD140, this rounds up to
16 bytes because of the substructure.

| STD | Array Stride | Bytes |
|-----|:--------------------:|-------|
| 140 | 48 | <kbd>{w [bbbb][----][----][----]}<br/>{x a{[bbbb]}<br/> &nbsp;&nbsp; b{[bbbb]}<br/> &nbsp;&nbsp; c{[bbbb]}<br /> &nbsp;&nbsp; &nbsp;&nbsp;[----]<br/>{y [bbbb][----][----][----]}</kbd> |
| 430 | 20 | <kbd>{w [bbbb]}<br/>{x a{[bbbb]}<br/> &nbsp;&nbsp; b{[bbbb]}<br/> &nbsp;&nbsp; c{[bbbb]}<br/>{y [bbbb]}</kbd> |

<hr>

```
struct {
  int32 w;
  struct {
    int32 a;
    int32 b;
    float c[3];
  } x;
  float y;
}
```

The `int a` and `int b` end up packing together so 16 bytes of padding are added (instead of 24 bytes).
The `float c[3]` has an array stride of 16 bytes in STD140 and 4 bytes in STD430.

| STD | Array Stride | Bytes |
|-----|:--------------------:|-------|
| 140 | 96 | <kbd>{w [bbbb][----][----][----]}<br/>{x {a [bbbb]}<br/> &nbsp;&nbsp; {b [bbbb]}<br/> &nbsp;&nbsp; &nbsp;&nbsp; [----][----]<br/> &nbsp;&nbsp; {c [bbbb][----][----][----]<br/> &nbsp;&nbsp; &nbsp;&nbsp; [bbbb][----][----][----]<br/> &nbsp;&nbsp; &nbsp;&nbsp; [bbbb][----][----][----]}}<br/>{y [bbbb][----][----][----]}</kbd> |
| 430 | 28 | <kbd>{w [bbbb]}<br/>{x a{[bbbb]}<br/> &nbsp;&nbsp; {b [bbbb]}<br/> &nbsp;&nbsp; {c [bbbb][bbbb][bbbb] }}<br/>{y [bbbb]}</kbd> |
