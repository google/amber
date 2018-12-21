This is copied from #197

### Multiple updates for a SSBO in VkRunnerScript

As debated [here](https://github.com/Igalia/vkrunner/issues/46),
VkRunnerScript applies multiple updates for a SSBO before a `probe`
and uses the accumulated one for all computes or draws before the
`probe`. VkRunnerScript repeats this for every `probe` commands.

For example,
```
ssbo 0 subdata float 0 1.0
compute 1 1 1
ssbo 0 subdata float 0 2.0
compute 1 1 1
ssbo 0 subdata float 36 1.0 2.0 3.0 \
                        ... ( more than 4000 numbers) ... 4096.0
compute 1 1 1
probe ssbo float 0 ~= 2.0
ssbo 0 subdata float 4000 1.0 2.0 3.0 \
                          ... ( more than 4000 numbers) ... 4096.0
compute 1 1 1
probe ssbo float 4000 ~= 1.0
```

This script has the same behavior as the following script does

```
ssbo 0 subdata float 0 1.0
ssbo 0 subdata float 0 2.0
ssbo 0 subdata float 36 1.0 2.0 3.0 \
                        ... ( more than 4000 numbers) ... 4096.0
compute 1 1 1
compute 1 1 1
compute 1 1 1
probe ssbo float 0 ~= 2.0

ssbo 0 subdata float 4000 1.0 2.0 3.0 \
                          ... ( more than 4000 numbers) ... 4096.0
compute 1 1 1
probe ssbo float 4000 ~= 1.0
```

### Multiple updates for a SSBO in AmberScript

AmberScript will have different semantics for those multiple
updates for a SSBO.
It will apply updates immediately for each compute or draw.

For example,
```
#!amber

BUFFER kComputeBuffer DATA_TYPE float DATA
1.0
END  # buffer

PIPELINE compute kComputePipeline
  ATTACH kComputeShader
  BIND BUFFER kComputeBuffer AS storage DESCRIPTOR_SET 0 BINDING 0
END  # pipeline

RUN kComputePipeline 1 1 1

BUFFER kComputeBuffer DATA_TYPE float DATA
2.0
END  # buffer

PIPELINE compute kComputePipeline
  ATTACH kComputeShader
  BIND BUFFER kComputeBuffer AS storage DESCRIPTOR_SET 0 BINDING 0
END  # pipeline

RUN kComputePipeline 1 1 1

BUFFER kComputeBuffer DATA_TYPE float DATA
2.0 0 0 0 0 0 0 0 0 \
1.0 2.0 3.0 ... ( more than 4000 numbers) ... 4096.0
END  # buffer

PIPELINE compute kComputePipeline
  ATTACH kComputeShader
  BIND BUFFER kComputeBuffer AS storage DESCRIPTOR_SET 0 BINDING 0
END  # pipeline

RUN kComputePipeline 1 1 1
```

In this script,
* the first `RUN kComputePipeline` sees `kComputeBuffer` as `1.0`
* the second one sees `2.0` if the first compute shader does not change
  `kComputeBuffer`
* the third one sees
  `2.0 0 0 ... 0 1.0 2.0 3.0 ... ( more than 4000 numbers) ... 4096.0`
  if the first and second compute shaders does not change `kComputeBuffer`

If each compute shader writes something to `kComputeBuffer`, it remains
and later `RUN` should see it.


### Implementation to solve mismatch between those two scripts

It must be handled by executor layer and we will use the same Vulkan
engine for both scripts.
If it is VkRunnerScript, the executor must call all `DoBuffer`
first before `probe`. After `probe`, the executor must call all
`DoBuffer` before the next `probe` and so on.
It is one of the simplest solution for this mismatch.
