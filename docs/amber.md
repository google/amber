# Amber

Amber is a multi-API shader test framework.Graphics bugs can be captured and
communicated through a scripting interface. This removes the need to program
to the low level interface when reproducing bugs.

Amber is broken into multiple parts, the applications, the parsing components
and the script execution.

## Applications
There are currently two applications, the `samples/amber.cc` amber application
and the amber integration into the [CTS](https://github.com/KhronosGroup/VK-GL-CTS/tree/master/external/vulkancts/modules/vulkan/amber). These applications are responsible
for configuring the script execution environment (setting up Vulkan, Dawn or
another engine), calling into the parsing code to generate a test script and
then passing that script into the script execution component.

We require the application to configure the execution engine. This allows the
application to handle loading function pointers, doing configuration and other
setups as required. There are no hardcoded assumptions in amber as to how you're
setting up the API to be tested.

This engine configuration is done through the `VulkanEngineConfig` in
`amber/amber_vulkan.h` and the `DawnEngineConfig` in `amber/amber_dawn.h`.

The sample application does this configuration through the config_helper
classes. `samples/config_helper_vulkan.*` and `samples/config_helper_dawn.*`.
For the CTS, the Vulkan engine is already configured and we just set the
`VulkanEngineConfig` as needed.

Accessing amber itself is done through the `Amber` class in `amber/amber.h`. We
require the application to parse and execute a script as two separate steps.
This separation allows for the scripts to be retrieved after a parse command
and then pre-compiled and passed into the execution. Passing the compiled
scripts is optional.

### Delegate
A delegate object can be provided to Amber, `nullptr` can be provided for the
delegate. If the delegate is provided and the `LogGraphicsCalls` method returns
`true` then the vulkan API wrappers will call `Log` for each call into Vulkan.
If the `LogGraphicsCallsTime` also returns `true` then timings for those
Vulkan calls will also be recorded. The timestamps are retrieved from the
`GetTimestampNS` callback.

### Buffer Extractions
Amber can be instructed to retrieve the contents of buffers when execution is
complete. This is done through the `extractions` list in the amber `Options`
structure. You must set the `buffer_name` for each of the buffers you want to
extract. When amber completes it will fill out the `width`, `height` and set
of `Value` objects for that buffer.

### Execution
There are two methods to execute a parsed script `Execute` and
`ExecuteWithShaderData`. They both accept the `Recipe` and `Options`, the
`ExecuteWithShaderData` also accepts a map of shader name to data. The data
is the compiled SPIR-V binary for the shader. This allows you to compile and
cache the shader if needed.

## Parsing component
There are currently two parsing backends for Amber.
[AmberScript](amber_script.md) and [VkScript](vk_script.md). The `AmberScript`
parser will be used if the first 7 characters of the input buffer are
`#!amber`, otherwise the `VkScript` parser will be used. The parsers both
generate a `Script` which is our representation of the script file.

### VkScript
For VkScript we do a bit of work to make the script match AmberScript. A default
pipeline is generated and all content in the script is considered part of the
generated pipeline. We generate names for all of the buffers in the file. The
framebuffer is named `framebuffer`. The generated depth buffer is named
`depth_buffer`. For each buffer seen we generate a name of `AutoBuf-<num>` where
the number is the current number of buffers seen counting from 0.

The VkScript parser is broken into three major chunks. The `Parser`,
`SectionParser` and `CommandParser`. The `Parser` is the overall driver for the
parser. The `SectionParser` breaks the input file into the overall chunks (each
of the sections separated by the \[blocks]). The `CommandParser` parses the
`[test]` section specifically. For other sections they're parsed directly in
the `Parser` object.

### Parsed Representation
The `Script` object is the owner for all of our data. Other objects hold
pointers but the script holds the `unique_ptr` for parsed objects.

```
                                        +--------+                 +--------------+
                                        | Script |---------------->| Requirements |
                                        +--------+                 +--------------+
                                             |
             +------------------+------------+--------+----------------------+
             |                  |                     |                      |
             v                  v                     v                      v
      +---------+          +---------+        +---------+            +---------+
      |  +--------+        |  +---------+     |  +----------+        |  +---------+
      +--|  +--------+     +--|  +--------+   +--|  +----------+     +--|  +---------+
         +--| Shader |        +--| Buffer |      +--| Pipeline |        +--| Command |
            +--------+           +--------+         +----------+           +---------+
               ^                  ^  ^                 |  |  ^               |    |
               |                  |  |                 |  |  +---------------+    |
Entry point    |                  |  +-----------------+  |                       |
Optimizations  |                  |  Descriptor Set       |                       |
Type           |                  |  Binding              |                       |
Compiled Shader|                  |  Attachment Location  |                       |
               |                  |                       |                       |
               +------------------------------------------+                       |
                                  |                                               |
                                  +-----------------------------------------------+
                                                        BufferCommand
```

A `Script` is made up for shaders, pipelines, buffers and commands. Pipelines
contain shaders and buffers. The buffers hold both image and storage buffers.
It is assumed that the `Buffer` object has the current value for that buffer at
any given time. This means that a draw command will need to copy buffer data to
the device, execute the draw, and then copy the device data back into the
buffer. Amber does not do any extra calls to fill the buffers. Then engine must
keep that data in sync.

The `Pipeline` object holds the context for a given set of tests. This includes
shaders, colour attachments, depth/stencil attachment, data buffers, etc. The
colour attachments will have their `Attachment Location` while data buffers
will have a `Descriptor Set` and `Binding` provided.

## Execution
When the script is executed the pipeline shaders will be compiled, if not
provided through the shader map. This will fill in the `Compiled Shader` data
in the each pipeline shader object. The `CreatePipeline` call will then be
executed for a given pipeline.

With the pipeline created on the backed each of the `Command` objects will
be executed. Each `Command` knows the amber `Pipeline` associated with it, that
`Pipeline` pointer is provided in the command execution and can be used to
lookup the engine specific representation of a pipeline.

When the `Probe` and `ProbeSSBO` commands are encountered they are not passed
to the engine but sent to the `Verifier`. This will validate that the data
in the requested buffer matches the test data. As mentioned above, this assumes
that the amber `Buffer` is kept up to date with the current device memory as we
do not attempt to fill in the buffers before executing the `Probe*` calls.
