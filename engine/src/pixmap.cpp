#include "pch.h"
#include "gfx_types.h"
#include "neko_exception.h"
#include "filesystem.h"
#include "locator.h"
#include "neko_platform.h"

#include "lodepng.h"
#include "tinytiffreader.hxx"
#include "tinyexr.h"

namespace neko {

  // PixelFormat mapping to <components, bytes per component>
  static const map<PixelFormat, pair<int, int>> g_fmtInfo = {
    { PixFmtColorRGB8, { 3, 1 } },
    { PixFmtColorRGBA8, { 4, 1 } },
    { PixFmtColorRGBA16f, { 4, 2 } },
    { PixFmtColorRGBA32f, { 4, 4 } },
    { PixFmtColorR8, { 1, 1 } },
    { PixFmtColorRG8, { 2, 1 } },
    { PixFmtColorR16f, { 1, 2 } }
  };

  Pixmap::Pixmap( PixelFormat fmt ): format_( fmt )
  {
    if ( !g_fmtInfo.contains( format_ ) )
      NEKO_EXCEPT( "Unsupported pixel format passed to Pixmap" );
  }

  Pixmap::Pixmap( int width, int height, PixelFormat fmt, const uint8_t* data ):
    width_( width ), height_( height ), format_( fmt )
  {
    assert( width_ > 0 && height_ > 0 );

    if ( !g_fmtInfo.contains( format_ ) )
      NEKO_EXCEPT( "Unsupported pixel format passed to Pixmap" );

    const auto& p = g_fmtInfo.at( format_ );
    auto datasize = ( width_ * height_ * p.first * p.second );
    data_.resize( static_cast<size_t>( datasize ) );

    if ( data )
      memcpy( data_.data(), data, datasize );
  }

  Pixmap::Pixmap( const Pixmap& from, int x, int y, int width, int height ):
    width_( width ), height_( height ), format_( from.format() )
  {
    assert( x >= 0 && y >= 0 && width_ > 0 && height_ > 0 );
    if ( x + width_ > from.width() || y + height_ > from.height() )
      NEKO_EXCEPT( "Crop constructor's parameters would overshoot source dimensions" );

    auto stride = g_fmtInfo.at( format_ ).first * g_fmtInfo.at( format_ ).second;
    data_.resize( width_ * height_ * stride );

    int dstrow = 0;
    for ( int srcrow = y; srcrow < ( y + height_ ); ++srcrow )
    {
      auto srcoffset = ( ( srcrow * from.width() * stride ) + ( x * stride ) );
      auto dstoffset = ( ( dstrow * width_ * stride ) );
      memcpy( data_.data() + dstoffset, from.data().data() + srcoffset, width_ * stride );
      dstrow++;
    }
  }

  Pixmap Pixmap::from( const Pixmap& rhs )
  {
    Pixmap out( rhs.width(), rhs.height(), rhs.format(), rhs.data().data() );
    return out;
  }

  Pixmap Pixmap::fromPNG( const vector<uint8_t>& input )
  {
    Pixmap out( PixFmtColorRGBA8 );

    vector<uint8_t> pngdata;
    unsigned int width, height;

    if ( lodepng::decode( pngdata, width, height, input.data(), input.size(), LCT_RGBA, 8 ) == 0 )
    {
      out.width_ = width;
      out.height_ = height;
      out.data_.resize( pngdata.size() );
      for ( size_t i = 0; i < height; ++i ) // flip rows
      {
        auto srcrow = pngdata.data() + ( i * (size_t)width * 4 );
        auto dstrow = out.data_.data() + ( ( (size_t)height - 1 - i ) * width * 4 );
        memcpy( dstrow, srcrow, (size_t)width * 4 );
      }
    }
    else
      NEKO_EXCEPT( "PNG decode failed" );

    return out;
  }

  Pixmap Pixmap::fromEXR( const vector<uint8_t>& input )
  {
    Pixmap out( PixFmtColorRGBA32f );

    int width, height;
    float* values = nullptr;
    const char* err = nullptr;

    auto ret = LoadEXRFromMemory( &values, &width, &height, input.data(), input.size(), &err );

    if ( ret != TINYEXR_SUCCESS )
      NEKO_EXCEPT( "LoadEXRFromMemory failed" );

    if ( !values )
      NEKO_EXCEPT( "No read from EXR" );

    out.width_ = width;
    out.height_ = height;
    out.data_.resize( static_cast<uint64_t>( width ) * height * 4 * sizeof( float ) );
    out.format_ = PixFmtColorRGBA32f;
    memcpy( out.data_.data(), values, out.data_.size() );
    free( values );

    return out;
  }

  struct TiffReaderInstance
  {
    FileReaderPtr reader;
  };

  uint64_t tiff_TinyTiffStatCallback( TinyTIFFReaderFile* tiff, const wchar_t* filename )
  {
    return Locator::fileSystem().fileStat( Dir_Textures, filename );
  }

  void tiff_TinyTiffOpenCallback( TinyTIFFReaderFile* tiff, const wchar_t* filename )
  {
    auto rdr = new TiffReaderInstance();
    rdr->reader = Locator::fileSystem().openFile( Dir_Textures, filename );
    tiff->hFile = reinterpret_cast<HANDLE>( rdr );
  }

  int tiff_TinyTiffSeekSetCallback( TinyTIFFReaderFile* tiff, unsigned long offset )
  {
    auto rdr = reinterpret_cast<TiffReaderInstance*>( tiff->hFile );
    return static_cast<int>( rdr->reader->seek( FileSeek_Beginning, offset ) );
  }

  int tiff_TinyTiffSeekCurCallback( TinyTIFFReaderFile* tiff, unsigned long offset )
  {
    auto rdr = reinterpret_cast<TiffReaderInstance*>( tiff->hFile );
    return static_cast<int>( rdr->reader->seek( FileSeek_Current, offset ) );
  }

  int tiff_TinyTiffCloseCallback( TinyTIFFReaderFile* tiff )
  {
    auto rdr = reinterpret_cast<TiffReaderInstance*>( tiff->hFile );
    rdr->reader.reset();
    delete rdr;
    return 0;
  }

  unsigned long tiff_TinyTiffReadCallback(
    void* ptr, unsigned long ptrsize, unsigned long size, unsigned long count, TinyTIFFReaderFile* tiff )
  {
    auto rdr = reinterpret_cast<TiffReaderInstance*>( tiff->hFile );
    auto toread = size * count;
    rdr->reader->read( ptr, toread );
    return toread;
  }

  long int tiff_TinyTiffTellCallback( TinyTIFFReaderFile* tiff )
  {
    auto rdr = reinterpret_cast<TiffReaderInstance*>( tiff->hFile );
    return rdr->reader->tell();
  }

  int tiff_TinyTiffGetPosCallback( TinyTIFFReaderFile* tiff, unsigned long* pos )
  {
    auto rdr = reinterpret_cast<TiffReaderInstance*>( tiff->hFile );
    *pos = rdr->reader->tell();
    return 0;
  }

  int tiff_TinyTiffSetPosCallback( TinyTIFFReaderFile* tiff, const unsigned long* pos )
  {
    auto rdr = reinterpret_cast<TiffReaderInstance*>( tiff->hFile );
    rdr->reader->seek( FileSeek_Beginning, *pos );
    return 0;
  }

  static TinyTiffCustomCallbacks g_tiffCallbacks = {
    tiff_TinyTiffStatCallback,
    tiff_TinyTiffOpenCallback,
    tiff_TinyTiffSeekSetCallback,
    tiff_TinyTiffSeekCurCallback,
    tiff_TinyTiffCloseCallback,
    tiff_TinyTiffReadCallback,
    tiff_TinyTiffTellCallback,
    tiff_TinyTiffGetPosCallback,
    tiff_TinyTiffSetPosCallback
  };

  Pixmap Pixmap::fromTIFF( const utf8String& filename )
  {
    Pixmap out( PixFmtColorRGBA8 );

    auto tiff = TinyTIFFReader_open( platform::utf8ToWide( filename ).c_str(), &g_tiffCallbacks );
    if ( tiff )
    {
      out.width_ = TinyTIFFReader_getWidth( tiff );
      out.height_ = TinyTIFFReader_getHeight( tiff );
      auto bps = TinyTIFFReader_getBitsPerSample( tiff, 0 );
      auto sf = TinyTIFFReader_getSampleFormat( tiff );
      auto spp = TinyTIFFReader_getSamplesPerPixel( tiff );

      if ( spp == 3 )
        out.format_ = PixFmtColorRGB8;
      else if ( spp == 4 )
        out.format_ = PixFmtColorRGBA8;
      else
        NEKO_EXCEPT( "Unsupported channel count in TIFF image" );

      vector<uint8_t> tiffSource;
      tiffSource.resize( out.width_ * out.height_ * ( bps / 8 ) );
      out.data_.resize( out.width_ * out.height_ * spp * sizeof( uint8_t ) );
      if ( bps == 16 && sf == TINYTIFF_SAMPLEFORMAT_UINT )
      {
        auto data = reinterpret_cast<uint8_t*>( out.data_.data() );
        auto source = reinterpret_cast<uint16_t*>( tiffSource.data() );
        for ( auto s = 0; s < spp; ++s )
        {
          TinyTIFFReader_getSampleData( tiff, tiffSource.data(), s );
          if ( TinyTIFFReader_wasError( tiff ) )
            NEKO_EXCEPT( "TIFF reader error" );
          for ( auto y = 0; y < out.height_; ++y )
          {
            for ( auto x = 0; x < out.width_; ++x )
            {
              size_t p = ( y * out.width_ ) + x;
              size_t d = ( ( ( y * out.width_ ) + x ) * spp ) + s;
              // I'm practically entirely sure that this is wrong - whatever,
              // fix when we actually need one of these textures
              data[d] = math::iround( static_cast<Real>( source[p] ) / 256.0f );
            }
          }
        }
      }
      else if ( bps == 8 && sf == TINYTIFF_SAMPLEFORMAT_UINT )
      {
        auto data = reinterpret_cast<uint8_t*>( out.data_.data() );
        auto source = reinterpret_cast<uint8_t*>( tiffSource.data() );
        for ( auto s = 0; s < spp; ++s )
        {
          TinyTIFFReader_getSampleData( tiff, tiffSource.data(), s );
          if ( TinyTIFFReader_wasError( tiff ) )
            NEKO_EXCEPT( "TIFF reader error" );
          for ( auto y = 0; y < out.height_; ++y )
          {
            for ( auto x = 0; x < out.width_; ++x )
            {
              size_t p = ( y * out.width_ ) + x;
              size_t d = ( ( ( y * out.width_ ) + x ) * spp ) + s;
              data[d] = static_cast<uint8_t>( source[p] );
            }
          }
        }
      }
      else
        NEKO_EXCEPT( "Unsupported bit/channel/sample format combination in TIFF image" );

      free( tiff );
    }
    else
      NEKO_EXCEPT( "TIFF reader creation failed" );

    return out;
  }

  void Pixmap::reset()
  {
    width_ = 0;
    height_ = 0;
    data_.clear();
  }

  void Pixmap::writePNG( const utf8String& filename ) const
  {
    platform::FileWriter writer( filename );
    vector<uint8_t> buffer;
    if ( format_ == PixFmtColorRGBA8 )
      lodepng::encode( buffer, data_.data(), width_, height_, LCT_RGBA, 8 );
    else if ( format_ == PixFmtColorRGB8 )
      lodepng::encode( buffer, data_.data(), width_, height_, LCT_RGB, 8 );
    else if ( format_ == PixFmtColorR8 )
      lodepng::encode( buffer, data_.data(), width_, height_, LCT_GREY, 8 );
    writer.writeBlob( buffer.data(), static_cast<uint32_t>( buffer.size() ) );
  }

  void Pixmap::flipVertical()
  {
    auto stride = g_fmtInfo.at( format_ ).first * g_fmtInfo.at( format_ ).second;
    auto line = width_ * stride;
    vector<uint8_t> tmp( width_ * stride );
    for ( auto i = 0; i < height_ / 2; ++i )
    {
      auto a = data_.data() + ( i * line );
      auto b = data_.data() + ( ( height_ - i - 1 ) * line );
      memcpy( tmp.data(), a, line );
      memcpy( a, b, line );
      memcpy( b, tmp.data(), line );
    }
  }

  void Pixmap::flipRectVertical( int x, int y, int width, int height )
  {
    auto stride = g_fmtInfo.at( format_ ).first * g_fmtInfo.at( format_ ).second;
    auto line = width * stride;
    vector<uint8_t> tmp( line );
    for ( auto i = 0; i < height / 2; ++i )
    {
      auto a = data_.data() + ( ( ( ( i + y ) * width_ ) + x ) * stride );
      auto b = data_.data() + ( ( ( ( ( height - i - 1 ) + y ) * width_ ) + x ) * stride );
      memcpy( tmp.data(), a, line );
      memcpy( a, b, line );
      memcpy( b, tmp.data(), line );
    }
  }

  void Pixmap::flipRectHorizontal( int x, int y, int width, int height )
  {
    auto stride = g_fmtInfo.at( format_ ).first * g_fmtInfo.at( format_ ).second;
    auto line = width_ * stride;
    if ( stride == 4 )
    {
      for ( auto i = y; i < ( y + height ); ++i )
        for ( auto j = 0; j < width / 2; ++j )
        {
          auto base = data_.data() + ( i * line );
          auto a = reinterpret_cast<uint32_t*>( base + ( ( x + j ) * stride ) );
          auto b = reinterpret_cast<uint32_t*>( base + ( ( x + ( width - j - 1 ) ) * stride ) );
          auto v = *a;
          *a = *b;
          *b = v;
        }
    }
    else
      NEKO_EXCEPT( "Unsupported stride" );
  }

  void Pixmap::blitRectFrom( const Pixmap& rhs, int dst_x, int dst_y, int src_x, int src_y, int width, int height )
  {
    if ( rhs.format() != format_ )
      NEKO_EXCEPT( "Pixel formats don't match" );

    auto stride = g_fmtInfo.at( format_ ).first * g_fmtInfo.at( format_ ).second;

    if ( stride == 4 )
    {
      for ( auto i = 0; i < height; ++i )
      {
        auto src = rhs.data().data() + ( ( ( ( src_y + i ) * rhs.width() ) + src_x ) * stride );
        auto dst = data_.data() + ( ( ( ( dst_y + i ) * width_ ) + dst_x ) * stride );
        memcpy( dst, src, width * stride );
      }
    }
    else
      NEKO_EXCEPT( "Unsupported stride" );
  }

}