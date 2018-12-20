This is copied from #197

### Multiple updates for a SSBO in VkRunner

As debated [here](https://github.com/Igalia/vkrunner/issues/46),
VkRunner applies multiple updates for a SSBO and uses the final
one for all computes or draws.

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
ssbo 0 subdata float 4000 1.0 2.0 3.0 \
                          ... ( more than 4000 numbers) ... 4096.0

compute 1 1 1
compute 1 1 1
compute 1 1 1
probe ssbo float 0 ~= 2.0
compute 1 1 1
probe ssbo float 4000 ~= 1.0
```

### Multiple updates for a SSBO in Amber script

Amber script will have different semantics for those multiple
updates for a SSBO.
It will apply updates immediately for each compute or draw.

For example,
```
ssbo 0 subdata float 0 1.0
compute 1 1 1
ssbo 0 subdata float 0 2.0
compute 1 1 1
ssbo 0 subdata float 36 1.0 2.0 3.0 ... ( more than 4000 numbers) ... 4096.0
compute 1 1 1
```

In this script,
* the first `compute` sees SSBO at `0:0` as `1.0`
* the second one sees `2.0` if the first compute does not change the
  SSBO
* the third one sees
  `2.0 0 0 ... 0 1.0 2.0 3.0 ... ( more than 4000 numbers) ... 4096.0`
  if the first and second computes does not change the SSBO

If each `compute` writes something to SSBO, it also remains in the SSBO.


### How to solve mismatch between those two scripts

It must be handled by executor layer and we will use the same Vulkan
engine for both scripts.
If it is VkRunner script, the executor must call all `DoBuffer`
first before doing anything.
It is one of the simplest solution for this mismatch.
