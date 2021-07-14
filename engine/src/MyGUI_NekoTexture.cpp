#include "stdafx.h"

#include "MyGUI_NekoPlatform.h"

#include "MyGUI_NekoRTTexture.h"

namespace MyGUI {

  using namespace gl;

  NekoTexture::NekoTexture( const std::string& _name, NekoImageLoader* _loader )
      : mName( _name ),
        mWidth( 0 ),
        mHeight( 0 ),
        mPixelFormat( GLenum::GL_INVALID_ENUM ),
        mInternalPixelFormat( GLenum::GL_INVALID_ENUM ),
        mUsage( GL_INVALID_ENUM ),
        mAccess( GL_INVALID_ENUM ),
        mNumElemBytes( 0 ),
        mDataSize( 0 ),
        mTextureId( 0 ),
        mProgramId( 0 ),
        mPboID( 0 ),
        mLock( false ),
        mBuffer( nullptr ),
        mImageLoader( _loader ),
        mRenderTarget( nullptr )
  {
  }

  NekoTexture::~NekoTexture()
  {
    destroy();
  }

  const std::string& NekoTexture::getName() const
  {
    return mName;
  }

  void NekoTexture::setUsage( TextureUsage _usage )
  {
    mAccess = GLenum::GL_INVALID_ENUM;
    mUsage = GLenum::GL_INVALID_ENUM;

    if ( _usage == TextureUsage::Default )
    {
      mUsage = GL_STATIC_READ;
      mAccess = GL_READ_ONLY;
    }
    else if ( _usage.isValue( TextureUsage::Static ) )
    {
      if ( _usage.isValue( TextureUsage::Read ) )
      {
        if ( _usage.isValue( TextureUsage::Write ) )
        {
          mUsage = GL_STATIC_COPY;
          mAccess = GL_READ_WRITE;
        }
        else
        {
          mUsage = GL_STATIC_READ;
          mAccess = GL_READ_ONLY;
        }
      }
      else if ( _usage.isValue( TextureUsage::Write ) )
      {
        mUsage = GL_STATIC_DRAW;
        mAccess = GL_WRITE_ONLY;
      }
    }
    else if ( _usage.isValue( TextureUsage::Dynamic ) )
    {
      if ( _usage.isValue( TextureUsage::Read ) )
      {
        if ( _usage.isValue( TextureUsage::Write ) )
        {
          mUsage = GL_DYNAMIC_COPY;
          mAccess = GL_READ_WRITE;
        }
        else
        {
          mUsage = GL_DYNAMIC_READ;
          mAccess = GL_READ_ONLY;
        }
      }
      else if ( _usage.isValue( TextureUsage::Write ) )
      {
        mUsage = GL_DYNAMIC_DRAW;
        mAccess = GL_WRITE_ONLY;
      }
    }
    else if ( _usage.isValue( TextureUsage::Stream ) )
    {
      if ( _usage.isValue( TextureUsage::Read ) )
      {
        if ( _usage.isValue( TextureUsage::Write ) )
        {
          mUsage = GL_STREAM_COPY;
          mAccess = GL_READ_WRITE;
        }
        else
        {
          mUsage = GL_STREAM_READ;
          mAccess = GL_READ_ONLY;
        }
      }
      else if ( _usage.isValue( TextureUsage::Write ) )
      {
        mUsage = GL_STREAM_DRAW;
        mAccess = GL_WRITE_ONLY;
      }
    }
    else if ( _usage.isValue( TextureUsage::RenderTarget ) )
    {
      mUsage = GL_DYNAMIC_READ;
      mAccess = GL_READ_ONLY;
    }
  }

  void NekoTexture::createManual( int _width, int _height, TextureUsage _usage, PixelFormat _format )
  {
    createManual( _width, _height, _usage, _format, nullptr );
  }

  void NekoTexture::createManual( int _width, int _height, TextureUsage _usage, PixelFormat _format, void* _data )
  {
    assert( !mTextureId );

    //FIXME перенести в метод
    mInternalPixelFormat = GLenum::GL_INVALID_ENUM;
    mPixelFormat = GLenum::GL_INVALID_ENUM;
    mNumElemBytes = 0;
    if ( _format == PixelFormat::R8G8B8 )
    {
      mInternalPixelFormat = GL_RGB8;
      mPixelFormat = GL_BGR;
      mNumElemBytes = 3;
    }
    else if ( _format == PixelFormat::R8G8B8A8 )
    {
      mInternalPixelFormat = GL_RGBA8;
      mPixelFormat = GL_BGRA;
      mNumElemBytes = 4;
    }
    else
    {
      NEKO_EXCEPT( "format not support" );
    }

    mWidth = _width;
    mHeight = _height;
    mDataSize = _width * _height * mNumElemBytes;
    setUsage( _usage );
    //MYGUI_PLATFORM_ASSERT(mUsage, "usage format not support");

    mOriginalFormat = _format;
    mOriginalUsage = _usage;

    // Set unpack alignment to one byte
    int alignment = 0;
    glGetIntegerv( GL_UNPACK_ALIGNMENT, &alignment );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

    // создаем тукстуру
    glGenTextures( 1, &mTextureId );
    glBindTexture( GL_TEXTURE_2D, mTextureId );
    // Set texture parameters
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      mInternalPixelFormat,
      mWidth,
      mHeight,
      0,
      mPixelFormat,
      GL_UNSIGNED_BYTE,
      (GLvoid*)_data );
    glBindTexture( GL_TEXTURE_2D, 0 );

    // Restore old unpack alignment
    glPixelStorei( GL_UNPACK_ALIGNMENT, alignment );

    if ( !_data )
    {
      //создаем текстурнный буфер
      glGenBuffers( 1, &mPboID );
      glBindBuffer( GL_PIXEL_UNPACK_BUFFER, mPboID );
      glBufferData( GL_PIXEL_UNPACK_BUFFER, mDataSize, nullptr, mUsage );
      glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
    }
  }

  void NekoTexture::destroy()
  {
    if ( mRenderTarget != nullptr )
    {
      delete mRenderTarget;
      mRenderTarget = nullptr;
    }

    if ( mTextureId != 0 )
    {
      glDeleteTextures( 1, &mTextureId );
      mTextureId = 0;
    }
    if ( mPboID != 0 )
    {
      glDeleteBuffers( 1, &mPboID );
      mPboID = 0;
    }

    mWidth = 0;
    mHeight = 0;
    mLock = false;
    mPixelFormat = GLenum::GL_INVALID_ENUM;
    mDataSize = 0;
    mUsage = GLenum::GL_INVALID_ENUM;
    mBuffer = nullptr;
    mInternalPixelFormat = GLenum::GL_INVALID_ENUM;
    mAccess = GLenum::GL_INVALID_ENUM;
    mNumElemBytes = 0;
    mOriginalFormat = PixelFormat::Unknow;
    mOriginalUsage = TextureUsage::Default;
  }

  void* NekoTexture::lock( TextureUsage _access )
  {
    assert( mTextureId );

    if ( _access == TextureUsage::Read )
    {
      glBindTexture( GL_TEXTURE_2D, mTextureId );

      mBuffer = new unsigned char[mDataSize];
      glGetTexImage( GL_TEXTURE_2D, 0, mPixelFormat, GL_UNSIGNED_BYTE, mBuffer );

      mLock = false;

      return mBuffer;
    }

    // bind the texture
    glBindTexture( GL_TEXTURE_2D, mTextureId );

    // bind the PBO
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, mPboID );

    // Note that glMapBuffer() causes sync issue.
    // If GPU is working with this buffer, glMapBuffer() will wait(stall)
    // until GPU to finish its job. To avoid waiting (idle), you can call
    // first glBufferData() with nullptr pointer before glMapBuffer().
    // If you do that, the previous data in PBO will be discarded and
    // glMapBuffer() returns a new allocated pointer immediately
    // even if GPU is still working with the previous data.
    glBufferData( GL_PIXEL_UNPACK_BUFFER, mDataSize, nullptr, mUsage );

    // map the buffer object into client's memory
    mBuffer = (GLubyte*)glMapBuffer( GL_PIXEL_UNPACK_BUFFER, mAccess );
    if ( !mBuffer )
    {
      glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
      glBindTexture( GL_TEXTURE_2D, 0 );
      NEKO_EXCEPT( "Error texture lock" );
    }

    mLock = true;

    return mBuffer;
  }

  void NekoTexture::unlock()
  {
    if ( !mLock && mBuffer )
    {
      delete[]( char* ) mBuffer;
      mBuffer = nullptr;

      glBindTexture( GL_TEXTURE_2D, 0 );

      return;
    }

    assert( mLock );

    // release the mapped buffer
    glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER );

    // copy pixels from PBO to texture object
    // Use offset instead of ponter.
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight, mPixelFormat, GL_UNSIGNED_BYTE, nullptr );

    // it is good idea to release PBOs with ID 0 after use.
    // Once bound with 0, all pixel operations are back to normal ways.
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

    glBindTexture( GL_TEXTURE_2D, 0 );
    mBuffer = nullptr;
    mLock = false;
  }

  void NekoTexture::loadFromFile( const std::string& _filename )
  {
    destroy();

    if ( mImageLoader )
    {
      int width = 0;
      int height = 0;
      PixelFormat format = PixelFormat::Unknow;

      void* data = mImageLoader->loadImage( width, height, format, _filename );
      if ( data )
      {
        createManual( width, height, TextureUsage::Static | TextureUsage::Write, format, data );
        delete[]( unsigned char* ) data;
      }
    }
  }

  void NekoTexture::saveToFile( const std::string& _filename )
  {
    if ( mImageLoader )
    {
      void* data = lock( TextureUsage::Read );
      mImageLoader->saveImage( mWidth, mHeight, mOriginalFormat, data, _filename );
      unlock();
    }
  }

  void NekoTexture::setShader( const std::string& _shaderName )
  {
    NEKO_EXCEPT( "Mygui called setShader" );
    mProgramId = 0; // NekoRenderManager::getInstance().getShaderProgramId( _shaderName );
  }

  IRenderTarget* NekoTexture::getRenderTarget()
  {
    if ( mRenderTarget == nullptr )
      mRenderTarget = new NekoRTTexture( mTextureId );

    return mRenderTarget;
  }

  unsigned int NekoTexture::getTextureId() const
  {
    return mTextureId;
  }

  unsigned int NekoTexture::getShaderId() const
  {
    return mProgramId;
  }

  int NekoTexture::getWidth() const
  {
    return mWidth;
  }

  int NekoTexture::getHeight() const
  {
    return mHeight;
  }

  bool NekoTexture::isLocked() const
  {
    return mLock;
  }

  PixelFormat NekoTexture::getFormat() const
  {
    return mOriginalFormat;
  }

  TextureUsage NekoTexture::getUsage() const
  {
    return mOriginalUsage;
  }

  size_t NekoTexture::getNumElemBytes() const
  {
    return mNumElemBytes;
  }

}