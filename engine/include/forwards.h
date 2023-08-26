#pragma once
#include "neko_types.h"

namespace neko {

  template <class T>
  class ShareableBase: public enable_shared_from_this<T> {
  public:
    inline shared_ptr<T> ptr() noexcept { return this->shared_from_this(); }
  };

  struct EngineInfo;

  class Engine;
  using EnginePtr = shared_ptr<Engine>;

  template <class T>
  class SystemBase: public nocopy, public ShareableBase<T> {
  public:
    //
  };

  template <class T, LogSource VLogSource>
  class Subsystem: public SystemBase<T> {
  private:
    Subsystem<T, VLogSource>() = delete;
  protected:
    EnginePtr engine_;
  public:
    Subsystem<T, VLogSource>( EnginePtr engine ): engine_( engine ) { assert( engine_ ); }
    template <typename... Params>
    void log( Params&&... params )
    {
      engine_->console()->printf( VLogSource, std::forward<Params>( params )... );
    }
    virtual void preUpdate( GameTime time ) {}
    virtual void tick( GameTime tick, GameTime time ) {}
    virtual void postUpdate( GameTime delta, GameTime tick ) {}
    virtual ~Subsystem() {}
  };

  class Steam;
  using SteamPtr = shared_ptr<Steam>;

  class Gfx;
  using GfxPtr = shared_ptr<Gfx>;

  class Console;
  using ConsolePtr = shared_ptr<Console>;

  class Scripting;
  using ScriptingPtr = shared_ptr<Scripting>;

  class GfxInput;
  using GfxInputPtr = shared_ptr<GfxInput>;

  class Memory;
  using MemoryPtr = shared_ptr<Memory>;

  class Shaders;
  using ShadersPtr = shared_ptr<Shaders>;

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

  class FileSystem;
  using FileSystemPtr = shared_ptr<FileSystem>;

  class TextManager;
  using TextManagerPtr = shared_ptr<TextManager>;

  class Text;
  using TextPtr = shared_ptr<Text>;

  class ParticleSystemManager;
  using ParticleSystemManagerPtr = shared_ptr<ParticleSystemManager>;

  class SpriteManager;
  using SpriteManagerPtr = shared_ptr<SpriteManager>;

}