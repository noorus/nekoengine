#pragma once

#ifndef NEKO_NO_GUI

#include "MyGUI/MyGUI_Prerequest.h"
#include "MyGUI/MyGUI_LogManager.h"
#include "MyGUI/MyGUI_RenderFormat.h"
#include "MyGUI/MyGUI_IVertexBuffer.h"
#include "MyGUI/MyGUI_RenderManager.h"
#include "MyGUI/MyGUI_DataManager.h"

#include "forwards.h"
#include "mesh_primitives.h"
#include "neko_types.h"
#include "renderer.h"
#include "shaders.h"

namespace MyGUI {

  using neko::unique_ptr;
  using neko::utf8String;
  using neko::make_unique;

  class NekoTexture;
  class NekoRTTexture;

  class NekoImageLoader
  {
  public:
    virtual ~NekoImageLoader() {}
    virtual void* loadImage( int& _width, int& _height, PixelFormat& _format, const std::string& _filename ) = 0;
    virtual void saveImage( int _width, int _height, MyGUI::PixelFormat _format, void* _texture, const std::string& _filename ) = 0;
  };

  class NekoTexture : public ITexture
  {
  public:
    NekoTexture( const utf8String& name, NekoImageLoader* loader );
    ~NekoTexture() override;

    const std::string& getName() const override;

    void createManual( int width, int height, TextureUsage usage, PixelFormat format ) override;
    void loadFromFile( const utf8String& filename ) override;
    void saveToFile( const utf8String& filename ) override;
    void setShader( const utf8String& shaderName ) override;

    void destroy() override;

    int getWidth() const override;
    int getHeight() const override;

    void* lock( TextureUsage access ) override;
    void unlock() override;
    bool isLocked() const override;

    PixelFormat getFormat() const override;
    TextureUsage getUsage() const override;
    size_t getNumElemBytes() const override;

    IRenderTarget* getRenderTarget() override;

    unsigned int getTextureId() const;
    unsigned int getShaderId() const;
    void setUsage( TextureUsage usage );
    void createManual( int width, int height, TextureUsage usage, PixelFormat format, void* data );

  private:
    void _create();

  private:
    std::string name_;
    int width_;
    int height_;
    gl::GLenum pixelFormat_;
    gl::GLenum internalPixelFormat_;
    gl::GLenum usage_;
    gl::GLenum access_;
    size_t numElemBytes_;
    size_t dataSize_;
    unsigned int id_;
    unsigned int programId_;
    unsigned int pbo_;
    bool lock_;
    void* buffer_;
    PixelFormat originalFormat_;
    TextureUsage originalUsage_;
    NekoImageLoader* loader_;
    NekoRTTexture* target_;
  };

  class NekoVertexBuffer : public IVertexBuffer
  {
  public:
    NekoVertexBuffer( neko::Renderer* renderer );
    ~NekoVertexBuffer() override;

    void setVertexCount( size_t count ) override;
    size_t getVertexCount() const override;

    Vertex* lock() override;
    void unlock() override;
    void draw( int count );

  private:
    void resize();
    void create();
    void destroy();

  private:
    neko::Renderer* renderer_;
    unique_ptr<neko::MappedGLBuffer<MyGUI::Vertex>> buffer_;
    size_t needVertexCount_;
    size_t vertexCount_;
    gl::GLuint vao_;
  };

  class NekoRenderManager : public RenderManager, public IRenderTarget {
  public:
    NekoRenderManager();

    void initialise( neko::Renderer* renderer, NekoImageLoader* loader = nullptr );
    void shutdown();

    static NekoRenderManager& getInstance();
    static NekoRenderManager* getInstancePtr();

    const IntSize& getViewSize() const override;
    VertexColourType getVertexFormat() const override;
    bool isFormatSupported( PixelFormat format, TextureUsage usage ) override;
    IVertexBuffer* createVertexBuffer() override;
    void destroyVertexBuffer( IVertexBuffer* buffer ) override;
    ITexture* createTexture( const utf8String& name ) override;
    void destroyTexture( ITexture* texture ) override;
    ITexture* getTexture( const utf8String& name ) override;

    void begin() override;
    void end() override;
    void doRender( IVertexBuffer* buffer, ITexture* texture, size_t count ) override;
    const RenderTargetInfo& getInfo() const override;

    void setViewSize( int width, int height ) override;

    void registerShader( const utf8String& shaderName, const utf8String& vertexProgramFile, const utf8String& fragmentProgramFile ) override;

    void doRenderRtt( IVertexBuffer* buffer, ITexture* texture, size_t count );

    void drawOneFrame( neko::shaders::Shaders* shaders );

  private:
    void destroyAllResources();

  private:
    IntSize viewSize_;
    bool update_;
    VertexColourType vertexFormat_;
    neko::Renderer* renderer_;
    RenderTargetInfo info_;
    neko::shaders::Shaders* shaders_;
    unsigned int drawRefcount_;

    typedef std::map<utf8String, ITexture*> MapTexture;
    MapTexture textures_;
    NekoImageLoader* imageLoader_;

    bool initialized_;
  };

  class NekoDataManager : public DataManager
  {
  public:
    static NekoDataManager& getInstance()
    {
      return *getInstancePtr();
    }
    static NekoDataManager* getInstancePtr()
    {
      return static_cast<NekoDataManager*>( DataManager::getInstancePtr() );
    }

    IDataStream* getData( const utf8String& name ) const override;

    void freeData( IDataStream* data ) override;

    bool isDataExist( const utf8String& name ) const override;

    const VectorString& getDataListNames( const utf8String& pattern ) const override;

    const utf8String& getDataPath( const utf8String& name ) const override;

    void setDataPath( const utf8String& path );

  private:
    utf8String dataPath_;
  };

  class NekoPlatform {
  private:
    bool initialized_;
    unique_ptr<NekoRenderManager> renderManager_;
    unique_ptr<NekoDataManager> dataManager_;
    unique_ptr<LogManager> logManager_;
  public:
    NekoPlatform();
    ~NekoPlatform();
    void initialise( neko::Renderer* renderer, NekoImageLoader* imageloader, const utf8String& logname );
    void shutdown();
    inline NekoRenderManager* getRenderManagerPtr() const noexcept { return renderManager_.get(); }
    NekoDataManager* getDataManagerPtr() const noexcept { return dataManager_.get(); }
  };

}

#endif