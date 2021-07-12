#pragma once
#include "neko_types.h"

namespace neko {

  class Subsystem;
  using SubsystemPtr = shared_ptr<Subsystem>;

  struct EngineInfo;

  class Engine;
  using EnginePtr = shared_ptr<Engine>;

  class Gfx;
  using GfxPtr = shared_ptr<Gfx>;

  class Console;
  using ConsolePtr = shared_ptr<Console>;

  class Scripting;
  using ScriptingPtr = shared_ptr<Scripting>;

  class Input;
  using InputPtr = shared_ptr<Input>;

  class Memory;
  using MemoryPtr = shared_ptr<Memory>;

  namespace shaders {

    class Shaders;
    using ShadersPtr = shared_ptr<Shaders>;

  }

  class Renderer;
  using RendererPtr = shared_ptr<Renderer>;

  class MeshManager;
  using MeshManagerPtr = shared_ptr<MeshManager>;

  class Camera;
  using CameraPtr = shared_ptr<Camera>;

  class ThreadedLoader;
  using ThreadedLoaderPtr = shared_ptr<ThreadedLoader>;

  class FontManager;
  using FontManagerPtr = shared_ptr<FontManager>;

  class Messaging;
  using MessagingPtr = shared_ptr<Messaging>;

  class ThreadedRenderer;
  using ThreadedRendererPtr = shared_ptr<ThreadedRenderer>;

  class Director;
  using DirectorPtr = shared_ptr<Director>;

  class MeshGenerator;
  using MeshGeneratorPtr = shared_ptr<MeshGenerator>;

}