#include "stdafx.h"

#ifndef NEKO_NO_GUI

#include "MyGUI_NekoPlatform.h"

#include "MyGUI_NekoRTTexture.h"

namespace MyGUI {

  using namespace gl;

  NekoTexture::NekoTexture( const utf8String& name, NekoImageLoader* loader ):
  name_( name ), width_( 0 ), height_( 0 ), pixelFormat_( GLenum::GL_INVALID_ENUM ),
  internalPixelFormat_( GLenum::GL_INVALID_ENUM ), usage_( GL_INVALID_ENUM ),
  access_( GL_INVALID_ENUM ), numElemBytes_( 0 ), dataSize_( 0 ), id_( 0 ),
  programId_( 0 ), pbo_( 0 ), lock_( false ), buffer_( nullptr ), loader_( loader ),
  target_( nullptr )
  {
  }

  NekoTexture::~NekoTexture()
  {
    // FIXME This nets us an access violation, and I don't know why, so for now
    // I just don't give a shit. UI textures are hardly going to be reloaded.
    // destroy();
  }

  const utf8String& NekoTexture::getName() const
  {
    return name_;
  }

  void NekoTexture::setUsage( TextureUsage usage )
  {
    access_ = GLenum::GL_INVALID_ENUM;
    usage_ = GLenum::GL_INVALID_ENUM;

    if ( usage == TextureUsage::Default )
    {
      usage_ = GL_STATIC_READ;
      access_ = GL_READ_ONLY;
    }
    else if ( usage.isValue( TextureUsage::Static ) )
    {
      if ( usage.isValue( TextureUsage::Read ) )
      {
        if ( usage.isValue( TextureUsage::Write ) )
        {
          usage_ = GL_STATIC_COPY;
          access_ = GL_READ_WRITE;
        }
        else
        {
          usage_ = GL_STATIC_READ;
          access_ = GL_READ_ONLY;
        }
      }
      else if ( usage.isValue( TextureUsage::Write ) )
      {
        usage_ = GL_STATIC_DRAW;
        access_ = GL_WRITE_ONLY;
      }
    }
    else if ( usage.isValue( TextureUsage::Dynamic ) )
    {
      if ( usage.isValue( TextureUsage::Read ) )
      {
        if ( usage.isValue( TextureUsage::Write ) )
        {
          usage_ = GL_DYNAMIC_COPY;
          access_ = GL_READ_WRITE;
        }
        else
        {
          usage_ = GL_DYNAMIC_READ;
          access_ = GL_READ_ONLY;
        }
      }
      else if ( usage.isValue( TextureUsage::Write ) )
      {
        usage_ = GL_DYNAMIC_DRAW;
        access_ = GL_WRITE_ONLY;
      }
    }
    else if ( usage.isValue( TextureUsage::Stream ) )
    {
      if ( usage.isValue( TextureUsage::Read ) )
      {
        if ( usage.isValue( TextureUsage::Write ) )
        {
          usage_ = GL_STREAM_COPY;
          access_ = GL_READ_WRITE;
        }
        else
        {
          usage_ = GL_STREAM_READ;
          access_ = GL_READ_ONLY;
        }
      }
      else if ( usage.isValue( TextureUsage::Write ) )
      {
        usage_ = GL_STREAM_DRAW;
        access_ = GL_WRITE_ONLY;
      }
    }
    else if ( usage.isValue( TextureUsage::RenderTarget ) )
    {
      usage_ = GL_DYNAMIC_READ;
      access_ = GL_READ_ONLY;
    }
  }

  void NekoTexture::createManual( int width, int height, TextureUsage usage, PixelFormat format )
  {
    createManual( width, height, usage, format, nullptr );
  }

  void NekoTexture::createManual( int width, int height, TextureUsage usage, PixelFormat format, void* data )
  {
    assert( !id_ );

    //FIXME перенести в метод
    internalPixelFormat_ = GLenum::GL_INVALID_ENUM;
    pixelFormat_ = GLenum::GL_INVALID_ENUM;
    numElemBytes_ = 0;
    if ( format == PixelFormat::R8G8B8 )
    {
      internalPixelFormat_ = GL_RGB8;
      pixelFormat_ = GL_BGR;
      numElemBytes_ = 3;
    }
    else if ( format == PixelFormat::R8G8B8A8 )
    {
      internalPixelFormat_ = GL_RGBA8;
      pixelFormat_ = GL_BGRA;
      numElemBytes_ = 4;
    }
    else
    {
      NEKO_EXCEPT( "format not support" );
    }

    width_ = width;
    height_ = height;
    dataSize_ = width * height * numElemBytes_;
    setUsage( usage );
    //MYGUI_PLATFORM_ASSERT(mUsage, "usage format not support");

    originalFormat_ = format;
    originalUsage_ = usage;

    // Set unpack alignment to one byte
    int alignment = 0;
    glGetIntegerv( GL_UNPACK_ALIGNMENT, &alignment );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    // создаем тукстуру
    glGenTextures( 1, &id_ );
    glBindTexture( GL_TEXTURE_2D, id_ );
    // Set texture parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexImage2D( GL_TEXTURE_2D, 0, internalPixelFormat_, width_, height_, 0, pixelFormat_, GL_UNSIGNED_BYTE, (GLvoid*)data );
    glBindTexture( GL_TEXTURE_2D, 0 );

    // Restore old unpack alignment
    glPixelStorei( GL_UNPACK_ALIGNMENT, alignment );

    if ( !data )
    {
      //создаем текстурнный буфер
      glGenBuffers( 1, &pbo_ );
      glBindBuffer( GL_PIXEL_UNPACK_BUFFER, pbo_ );
      glBufferData( GL_PIXEL_UNPACK_BUFFER, dataSize_, nullptr, usage_ );
      glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
    }
  }

  void NekoTexture::destroy()
  {
    if ( target_ != nullptr )
    {
      delete target_;
      target_ = nullptr;
    }

    if ( id_ != 0 )
    {
      glDeleteTextures( 1, &id_ );
      id_ = 0;
    }
    if ( pbo_ != 0 )
    {
      glDeleteBuffers( 1, &pbo_ );
      pbo_ = 0;
    }

    width_ = 0;
    height_ = 0;
    lock_ = false;
    pixelFormat_ = GLenum::GL_INVALID_ENUM;
    dataSize_ = 0;
    usage_ = GLenum::GL_INVALID_ENUM;
    buffer_ = nullptr;
    internalPixelFormat_ = GLenum::GL_INVALID_ENUM;
    access_ = GLenum::GL_INVALID_ENUM;
    numElemBytes_ = 0;
    originalFormat_ = PixelFormat::Unknow;
    originalUsage_ = TextureUsage::Default;
  }

  void* NekoTexture::lock( TextureUsage access )
  {
    assert( id_ );

    if ( access == TextureUsage::Read )
    {
      glBindTexture( GL_TEXTURE_2D, id_ );

      buffer_ = new unsigned char[dataSize_];
      glGetTexImage( GL_TEXTURE_2D, 0, pixelFormat_, GL_UNSIGNED_BYTE, buffer_ );

      lock_ = false;

      return buffer_;
    }

    // bind the texture
    glBindTexture( GL_TEXTURE_2D, id_ );

    // bind the PBO
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, pbo_ );

    // Note that glMapBuffer() causes sync issue.
    // If GPU is working with this buffer, glMapBuffer() will wait(stall)
    // until GPU to finish its job. To avoid waiting (idle), you can call
    // first glBufferData() with nullptr pointer before glMapBuffer().
    // If you do that, the previous data in PBO will be discarded and
    // glMapBuffer() returns a new allocated pointer immediately
    // even if GPU is still working with the previous data.
    glBufferData( GL_PIXEL_UNPACK_BUFFER, dataSize_, nullptr, usage_ );

    // map the buffer object into client's memory
    buffer_ = (GLubyte*)glMapBuffer( GL_PIXEL_UNPACK_BUFFER, access_ );
    if ( !buffer_ )
    {
      glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
      glBindTexture( GL_TEXTURE_2D, 0 );
      NEKO_EXCEPT( "Error texture lock" );
    }

    lock_ = true;

    return buffer_;
  }

  void NekoTexture::unlock()
  {
    if ( !lock_ && buffer_ )
    {
      delete[]( char* ) buffer_;
      buffer_ = nullptr;

      glBindTexture( GL_TEXTURE_2D, 0 );

      return;
    }

    assert( lock_ );

    // release the mapped buffer
    glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER );

    // copy pixels from PBO to texture object
    // Use offset instead of ponter.
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, width_, height_, pixelFormat_, GL_UNSIGNED_BYTE, nullptr );

    // it is good idea to release PBOs with ID 0 after use.
    // Once bound with 0, all pixel operations are back to normal ways.
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

    glBindTexture( GL_TEXTURE_2D, 0 );
    buffer_ = nullptr;
    lock_ = false;
  }

  void NekoTexture::loadFromFile( const std::string& filename )
  {
    destroy();

    if ( loader_ )
    {
      int width = 0;
      int height = 0;
      PixelFormat format = PixelFormat::Unknow;

      void* data = loader_->loadImage( width, height, format, filename );
      if ( data )
      {
        createManual( width, height, TextureUsage::Static | TextureUsage::Write, format, data );
        delete[]( unsigned char* ) data;
      }
    }
  }

  void NekoTexture::saveToFile( const std::string& _filename )
  {
    if ( loader_ )
    {
      void* data = lock( TextureUsage::Read );
      loader_->saveImage( width_, height_, originalFormat_, data, _filename );
      unlock();
    }
  }

  void NekoTexture::setShader( const std::string& shaderName )
  {
    NEKO_EXCEPT( "Mygui called setShader" );
    programId_ = 0; // NekoRenderManager::getInstance().getShaderProgramId( _shaderName );
  }

  IRenderTarget* NekoTexture::getRenderTarget()
  {
    if ( target_ == nullptr )
      target_ = new NekoRTTexture( id_ );

    return target_;
  }

  unsigned int NekoTexture::getTextureId() const
  {
    return id_;
  }

  unsigned int NekoTexture::getShaderId() const
  {
    return programId_;
  }

  int NekoTexture::getWidth() const
  {
    return width_;
  }

  int NekoTexture::getHeight() const
  {
    return height_;
  }

  bool NekoTexture::isLocked() const
  {
    return lock_;
  }

  PixelFormat NekoTexture::getFormat() const
  {
    return originalFormat_;
  }

  TextureUsage NekoTexture::getUsage() const
  {
    return originalUsage_;
  }

  size_t NekoTexture::getNumElemBytes() const
  {
    return numElemBytes_;
  }

}

#endif