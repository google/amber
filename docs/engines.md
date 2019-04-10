# Engines

Amber is designed to supported multiple graphics APIs. This is done through the
engine layer. The parsing/executing side of Amber (which we'll call the
frontend in this document) doesn't know anything about how the code is executed
and is API agnostic. Mostly.


## Engine Lifecycle
The engine will go through several states as the script executes. First the
engine will be created. The creation should not configure the backing graphics
API. The creation just sets up the basic engine structure.

```
                                 Engine
                                +------------------------------------+
                                |              +----------+          |
                       +---------------------->|  Create  |          |
                       |        |              +----------+          |
                       |        |                                    |                                                          +------------+
                       |        |                                    |                                                  +------>|Entry Point |
                       |        |             +------------+         |                                   +---------+    |       +------------+
                       +--------------------->| Initialize |         |                       +---------->| Shaders +----+
                       |        |             +------------+         |                       |           +---------+    |
                       |        |                                    |                       |                          |       +--------------+
    +----------+       |        |                                    |                       |                          +------>|Shader binary |
    | Executor +-------+        |                                    |        +----------+   |        +---------------+         +--------------+
    +----------+       |        |         *  +----------------+      |        |          |   +------->| Vertex Buffers|-----+
                       +-------------------->|Create Pipeline +-------------->| Pipeline |---+        +---------------+     |
                       |        |            +----------------+      |        |          |   |                              +--------+
Executor execution     |        |                                    |        +----------+   |                                       |
flows downwards        |        |                                    |                       |       +--------------------+          |
                       |        |         * +---------------------+  |                       +------>| Colour Attachments |----------+
                       +------------------->| Execute Do* methods |  |                       |       +--------------------+          |
                       |        |           +---------------------+  |                       |                                       |
                       |        |                                    |                       |                                       |      +--------------+
                       |        |                                    |                       |      +-------------------------+      +----->| Backing data |
                       |        |               +----------+         |                       +----->|Depth/Stencil Attachment |------+      +--------------+
                       +----------------------->| Destroy  |         |                       |      +-------------------------+      |
                                |               +----------+         |                       |                                       |
                                |                                    |                       |           +-------------+             |
                                |                                    |                       +---------->| Index Buffer|-------------+
                                +------------------------------------+                       |           +-------------+             |
                                                                                             |                                       |
                                                                                             |          +---------------+            |
                                                                                             +--------->| Other Buffers |------------+
                                                                                                        +---------------+
```

Once created, the engine will be initialized. This initialization will receive
the configured graphics API from the embedder. The engine can then setup any
extra needed queues, verify features/extensions are available or do any extra
configuration.

With the engine initialized all of the pipelines will be created through a
`CreatePipeline` method. The provided `amber::Pipeline` is fully specified
at this point and provides:
  * if this is a graphics or compute pipeline
  * all shader information (including SPIR-V binary)
  * all vertex/index buffer information.
    * initial buffer data if provided
    * descriptor set/binding information
  * all colour attachments
    * initial buffer data if provided
    * location information
  * all depth/stencil attachments
  * all storage buffers
    * initial buffer data if provided
    * descriptor set/binding information
  * framebuffer width/height

The engine should go through and create all the needed pipeline resources.

The shaders can be retrieved with `GetShaders`. The shader information provides
the entry point to be used. The shader is pre-compiled and any optimizations
will have been done already.

Buffer data is stored depending on how it's used. For colour attachments use
`GetcolorAttachments`. The depth/stencil is enabled if the `BufferInfo::buffer`
pointer is not `nullptr` from `GetDepthBuffer`. The vertex buffers are retrieved
from `GetVertexBuffers` and the index buffer is provided if `GetIndexBuffer`
returns non-`nullptr`. For all other storage buffers the `GetBuffers` method
will provide information on each buffer.

Each of the buffers should be allocated and have the data from the buffers
`ValuePtr` copied into the device side buffer.

At this point, all information needed to run a pipeline should have been
provided. The executor will then start running the commands provided in the
script. This can request the engine to run compute pipelines, graphics pipelines
or do various other things.

There is an assumption that the data in the `amber::Buffer` for each of the
buffers in the pipeline is always complete and up to date. This means when,
for instance, a draw command executes on a pipeline each of the buffers needs
to be read off the device and written back into the `amber::Buffer`.

When the script is finished the engine destructor will be executed and must
cleanup any resources created by the engine. It is the job of the embedder
to shut down the graphics API.


## API Layer
The engine API is described in `src/engine.h`. The `Engine` base class must be
implemented by the backend engine.


### API Methods
#### `Engine::Create`
The engines are all created in `src/engine.cc`. The `Engine::Create` method
will attempt to create the requested engine and return it if possible. When
adding a new engine a new block needs to be added to this method. When
`Engine::Create` is complete, the engine will be created but is _not_
initialized.


#### `Initialize`
The engine is initialized through the `Initialize` method. The initialize will
accept engine specific configuration as an `EngineConfig` parameter. This allows
for clients to set configuration data needed for the engine. The assumption is
that the engine itself does not initialize the graphics API. The API should
have been provided and all needed information provided in the `EngineConfig`
object.

A `Delegate` object is also provided to the engine. The delegate object is used
when the engine needs to talk back to the embedder. For instance, the delegates
can tell the engine to `LogGraphicsCalls`. The engine then, should, call the
`Log` method on the delegate for each internal graphics API method called.

If the executing script specified specific features, instance or device
extensions they will also be provided. The engine can use these as needed.


#### `CreatePipeline`
The `CreatePipeline` method is responsible for creating an engine specific
version of the given `amber::Pipeline`. Each command which needs a pipeline
will have the pipeline provided, so the engine must provide a way to go from
the amber pipeline to the engine pipeline. There can also be multiple pipeline
lines of a given type.

All information on the pipeline will be provided including shader and buffer
information.


#### `DoClearColor`
The `DoClearColor` command provides the colour that is to be used for a given
pipeline when executing the `DoClear` command.


#### `DoClearStencil`
The `DoClearStencil` command provides the value that is to be used for clearing
the stencil buffer for a given pipeline when `DoClear` is executed.


#### `DoClearDepth`
The `DoClearDepth` command provides the value that is to be used for clearing
the depth buffer for a given pipeline when `DoClear` is executed.


#### `DoClear`
The `DoClear` command instructs the engine to clear the various colour and
depth attachments for the given pipeline.


#### `DoDrawRect`
The `DoDrawRect` instructs the engine to draw the given pipeline in the box
at (x,y) of size (width,height). The buffers must be read back into the backing
`amber::Buffer` at the end of this method.


#### `DoDrawArrays`
The `DoDrawArrays` instructs the engine to draw the given pipeline using
the information in the attached vertex and index buffers.  The buffers must be
read back into the backing `amber::Buffer` at the end of this method.


#### `DoCompute`
The `DoCompute` instructs the engine to execute the given compute pipeline with
the provided (x,y,z) parameters.


#### `DoEntryPoint`
The `DoEntryPoint` command instructs the engine to change the entry point in
the given pipeline for the give shader.


#### `DoPatchParameterVertices`
The `DoPatchParameterVertices` tells the engine to do a thing with some
values....I don't really know what it means, heh.


#### `DoBuffer`
The `DoBuffer` command tells the engine that for a given pipeline the given
data should be updated into the given buffer.


#### `SetEngineData`
There are cases where there is extra data for a given engine created by the
front end system. That data is stored in a `EngineData` structure and passed
into the engine through the `SetEngineData` method. The engine should make
use if this data if it makes sense.


## APIisms in the Frontend
Most of the things which show up in the front end are the names for drawing
topologies, image formats and other descriptions use the Vulkan names. So,
`kR8G8B8A8_SINT` is directly from the Vulkan `VK_FORMAT_R8G8B8A8_SINT` type.
In the case of image formats the engine will either need to convert to their own
internal version, or ignore the type and use the `Format` components directly.
