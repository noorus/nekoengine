#include "stdafx.h"

#ifndef NEKO_NO_GUI

#include "MyGUI_NekoPlatform.h"

#include "MyGUI/MyGUI_VertexData.h"
#include "MyGUI/MyGUI_Gui.h"
#include "MyGUI/MyGUI_Timer.h"
#include "MyGUI/MyGUI_DataManager.h"

#include "neko_exception.h"

namespace MyGUI {

  using namespace gl;

  NekoRenderManager& NekoRenderManager::getInstance()
  {
    return *getInstancePtr();
  }

  NekoRenderManager* NekoRenderManager::getInstancePtr()
  {
    return static_cast<NekoRenderManager*>( RenderManager::getInstancePtr() );
  }

  NekoRenderManager::NekoRenderManager():
  update_( false ), imageLoader_( nullptr ), initialized_( false ),
  shaders_( nullptr ), drawRefcount_( 0 ), renderer_( nullptr )
  {
  }

  void NekoRenderManager::initialise( neko::Renderer* renderer, NekoImageLoader* imageloader )
  {
    vertexFormat_ = VertexColourType::ColourARGB;

    update_ = false;

    renderer_ = renderer;
    imageLoader_ = imageloader;

    drawRefcount_ = 0;

    initialized_ = true;
  }

  void NekoRenderManager::shutdown()
  {
    destroyAllResources();
    initialized_ = false;
  }

  IVertexBuffer* NekoRenderManager::createVertexBuffer()
  {
    return new NekoVertexBuffer( renderer_ );
  }

  void NekoRenderManager::destroyVertexBuffer( IVertexBuffer* buffer )
  {
    delete buffer;
  }

  void NekoRenderManager::doRenderRtt( IVertexBuffer* buffer, ITexture* texture, size_t count )
  {
    auto& pipeline = shaders_->usePipeline( "gui" );
    pipeline.setUniform( "yscale", -1.0f );
    doRender( buffer, texture, count );
    pipeline.setUniform( "yscale", 1.0f );
  }

  void NekoRenderManager::doRender( IVertexBuffer* buffer, ITexture* texture, size_t count )
  {
    auto vbuf = static_cast<NekoVertexBuffer*>( buffer );

    unsigned int texture_id = 0;
    if ( texture )
    {
      auto ntex = static_cast<NekoTexture*>( texture );
      texture_id = ntex->getTextureId();
      /*if ( texture->getShaderId() )
      {
        glUseProgram( texture->getShaderId() );
      }*/
    }

    glBindTextureUnit( 0, texture_id );
    vbuf->draw( (int)count );

    /*if ( _texture && static_cast<NekoTexture*>( _texture )->getShaderId() )
    {
      glUseProgram( mDefaultProgramId );
    }*/
  }

  void NekoRenderManager::begin()
  {
    ++drawRefcount_;

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }

  void NekoRenderManager::end()
  {
    if ( --drawRefcount_ == 0 )
    {
      glDisable( GL_BLEND );
    }
  }

  const RenderTargetInfo& NekoRenderManager::getInfo() const
  {
    return info_;
  }

  const IntSize& NekoRenderManager::getViewSize() const
  {
    return viewSize_;
  }

  VertexColourType NekoRenderManager::getVertexFormat() const
  {
    return vertexFormat_;
  }

  bool NekoRenderManager::isFormatSupported( PixelFormat format, TextureUsage usage )
  {
    return ( format == PixelFormat::R8G8B8 || format == PixelFormat::R8G8B8A8 );
  }

  void NekoRenderManager::drawOneFrame( neko::shaders::Shaders* shaders )
  {
    Gui* gui = Gui::getInstancePtr();
    if ( gui == nullptr )
      return;

    shaders_ = shaders;

    static Timer timer;
    static unsigned long last_time = timer.getMilliseconds();
    unsigned long now_time = timer.getMilliseconds();
    unsigned long time = now_time - last_time;

    onFrameEvent( time / 1000.0f );

    last_time = now_time;

    begin();
    onRenderToTarget( this, update_ );
    end();

    update_ = false;
  }

  void NekoRenderManager::setViewSize( int width, int height )
  {
    viewSize_.set( width, height );

    info_.maximumDepth = 1;
    info_.hOffset = 0;
    info_.vOffset = 0;
    info_.aspectCoef = float( viewSize_.height ) / float( viewSize_.width );
    info_.pixScaleX = 1.0f / float( viewSize_.width );
    info_.pixScaleY = 1.0f / float( viewSize_.height );

    onResizeView( viewSize_ );
    update_ = true;
  }

  void NekoRenderManager::registerShader( const std::string& shaderName, const std::string& vertexProgramFile, const std::string& fragmentProgramFile )
  {
    NEKO_EXCEPT( "registerShader called by something" );
  }

  ITexture* NekoRenderManager::createTexture( const utf8String& name )
  {
    MapTexture::const_iterator item = textures_.find( name );
    assert( item == textures_.end() );

    auto texture = new NekoTexture( name, imageLoader_ );
    textures_[name] = texture;
    return texture;
  }

  void NekoRenderManager::destroyTexture( ITexture* texture )
  {
    if ( texture == nullptr )
      return;

    MapTexture::iterator item = textures_.find( texture->getName() );
    assert( item != textures_.end() );

    textures_.erase( item );
    delete texture;
  }

  ITexture* NekoRenderManager::getTexture( const utf8String& name )
  {
    MapTexture::const_iterator item = textures_.find( name );
    if ( item == textures_.end() )
      return nullptr;
    return item->second;
  }

  void NekoRenderManager::destroyAllResources()
  {
    for ( MapTexture::const_iterator item = textures_.begin(); item != textures_.end(); ++item )
    {
      delete item->second;
    }
    textures_.clear();
    shaders_ = nullptr;
  }

}

#endif