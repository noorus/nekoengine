#include "pch.h"
#include "loader.h"
#include "utilities.h"
#include "gfx_types.h"
#include "renderer.h"
#include "console.h"
#include "filesystem.h"

#include "lodepng.h"
#include "tinytiffreader.hxx"
#include "tinyexr.h"

namespace neko {

  const string c_loaderThreadName = "nekoLoader";

  const static unicodeString g_prerenderGlyphs = utils::uniFrom( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" );

  ThreadedLoader::ThreadedLoader(): thread_( c_loaderThreadName, threadProc, this )
  {
  }

  bool ThreadedLoader::threadProc( platform::Event& running, platform::Event& wantStop, void* argument )
  {
    platform::performanceInitializeLoaderThread();

    auto loader = ( (ThreadedLoader*)argument )->shared_from_this();
    running.set();
    platform::EventVector events = { loader->newTasksEvent_.get(), wantStop.get() };
    while ( true )
    {
      const size_t timeoutValue = 258;
      auto waitRet = platform::waitForEvents( events, 0, false, timeoutValue );
      if ( waitRet == 0 ) // new tasks
      {
        loader->handleNewTasks();
      }
      else if ( waitRet == timeoutValue )
      {
        // Nothing to do lol
        continue;
      }
      else
        break;
    }

    platform::performanceTeardownCurrentThread();
    return true;
  }

  void ThreadedLoader::start()
  {
    thread_.start();
  }

  void ThreadedLoader::stop()
  {
    thread_.stop();
  }

  void ThreadedLoader::addLoadTask( const LoadTaskVector& resources )
  {
    ScopedRWLock lock( &addTaskLock_ );

    newTasks_.reserve( newTasks_.size() + resources.size() );
    newTasks_.insert( newTasks_.end(), resources.begin(), resources.end() );
    newTasksEvent_.set();
  }

  void ThreadedLoader::getFinishedMaterials( MaterialVector& materials )
  {
    if ( !finishedMaterialsEvent_.check() )
      return;

    finishedTasksLock_.lock();
    materials.swap( finishedMaterials_ );
    finishedTasksLock_.unlock();

    finishedMaterials_.clear();
    finishedMaterialsEvent_.reset();
  }

  void ThreadedLoader::getFinishedFonts( FontVector& fonts )
  {
    if ( !finishedFontsEvent_.check() )
      return;

    finishedTasksLock_.lock();
    fonts.swap( finishedFonts_ );
    finishedTasksLock_.unlock();

    finishedFonts_.clear();
    finishedFontsEvent_.reset();
  }

  void ThreadedLoader::getFinishedModels( vector<MeshNodePtr>& models )
  {
    if ( !finishedModelsEvent_.check() )
      return;

    finishedTasksLock_.lock();
    models.swap( finishedModels_ );
    finishedTasksLock_.unlock();

    finishedModels_.clear();
    finishedModelsEvent_.reset();
  }

  void ThreadedLoader::getFinishedAnimations( AnimationVector& animations )
  {
    if ( !finishedAnimationsEvent_.check() )
      return;

    finishedTasksLock_.lock();
    animations.swap( finishedAnimations_ );
    finishedTasksLock_.unlock();

    finishedAnimations_.clear();
    finishedAnimationsEvent_.reset();
  }

  using namespace gl;

  struct TiffReaderInstance {
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

  unsigned long tiff_TinyTiffReadCallback( void* ptr, unsigned long ptrsize, unsigned long size, unsigned long count, TinyTIFFReaderFile* tiff )
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

  void ThreadedLoader::loadFontFace( LoadTask::FontfaceLoad& task )
  {
    vector<uint8_t> input;
    Locator::fileSystem().openFile( Dir_Fonts, task.path_ )->readFullVector( input );
    auto face = task.font_->loadFace( input, 0 );
    for ( const auto& spec : task.specs_ )
    {
      face->loadStyle( spec.rendering, spec.size, spec.thickness, g_prerenderGlyphs );
    }

    Locator::console().printf( srcLoader, "Loaded font %s (%s) with %i sizes", task.font_->name().c_str(),
      task.path_.c_str(), task.specs_.size() );

    finishedTasksLock_.lock();
    finishedFonts_.push_back( task.font_ );
    finishedTasksLock_.unlock();
  }

  void ThreadedLoader::loadModel( LoadTask::ModelLoad& task )
  {
    auto path = task.path_;

    auto filename = utils::extractFilename( path );
    auto filepath = utils::extractFilepath( R"(assets\meshes\)" + path );
    auto ext = utils::extractExtension( path );

    if ( ext == L"gltf" )
    {
      vector<uint8_t> input;
      Locator::fileSystem().openFile( Dir_Meshes, path )->readFullVector( input );
      loaders::loadGLTFModel(
        input, platform::wideToUtf8( filename ), platform::wideToUtf8( filepath ), task.node_ );
    }
    else
      NEKO_EXCEPT( "Unsupported mesh format in load model task" );

    finishedTasksLock_.lock();
    finishedModels_.push_back( task.node_ );
    finishedTasksLock_.unlock();
  }

  void ThreadedLoader::loadTexture( LoadTask::TextureLoad& task )
  {
    for ( const auto& target : task.paths_ )
    {
      vector<uint8_t> input;
      unsigned int width, height;
      Locator::fileSystem().openFile( Dir_Textures, target )->readFullVector( input );
      MaterialLayer layer;
      vector<uint8_t> rawData;
      auto ext = utils::extractExtension( target );
      if ( ext == L"png" )
      {
        if ( lodepng::decode( rawData, width, height, input.data(), input.size(), LCT_RGBA, 8 ) == 0 )
        {
          Locator::console().printf(
            srcLoader, "Loaded texture %s, %ix%i, rgba8", target.c_str(), width, height );
          layer.image_.width_ = width;
          layer.image_.height_ = height;
          layer.image_.format_ = PixFmtColorRGBA8;
          layer.image_.data_.resize( rawData.size() );
          // flip rows
          for ( size_t i = 0; i < height; ++i )
          {
            auto srcRow = rawData.data() + ( i * (size_t)width * 4 );
            auto dstRow = layer.image_.data_.data() + ( ( (size_t)height - 1 - i ) * width * 4 );
            memcpy( dstRow, srcRow, (size_t)width * 4 );
          }
          task.material_->layers_.push_back( move( layer ) );
          task.material_->wantWrapping_ = Texture::Repeat;
        }
        else
          NEKO_EXCEPT( "PNG decode failed" );
      }
      else if ( ext == L"exr" )
      {
        float* exr_values = nullptr;
        int exr_width, exr_height;
        const char* exr_err = nullptr;
        auto ret = LoadEXRFromMemory( &exr_values, &exr_width, &exr_height, input.data(), input.size(), &exr_err );
        if ( ret != TINYEXR_SUCCESS )
          NEKO_EXCEPT( "LoadEXRFromMemory failed" );
        if ( exr_values )
        {
          Locator::console().printf(
            srcLoader, "Loaded texture %s, %ix%i, rgba32f", target.c_str(), exr_width, exr_height );
          layer.image_.width_ = exr_width;
          layer.image_.height_ = exr_height;
          layer.image_.data_.resize( static_cast<uint64_t>( exr_width ) * exr_height * 4 * sizeof( float ) );
          layer.image_.format_ = PixFmtColorRGBA32f;
          memcpy( layer.image_.data_.data(), exr_values, layer.image_.data_.size() );
          free( exr_values );
          task.material_->layers_.push_back( move( layer ) );
        }
      }
      else if ( ext == L"tif" )
      {
        auto tiff = TinyTIFFReader_open( platform::utf8ToWide( target ).c_str(), &g_tiffCallbacks );
        if ( tiff )
        {
          layer.image_.width_ = TinyTIFFReader_getWidth( tiff );
          layer.image_.height_ = TinyTIFFReader_getHeight( tiff );
          auto bps = TinyTIFFReader_getBitsPerSample( tiff, 0 );
          auto sf = TinyTIFFReader_getSampleFormat( tiff );
          auto spp = TinyTIFFReader_getSamplesPerPixel( tiff );
          Locator::console().printf( srcLoader, "Loaded texture %s, %ix%i %ibit %ix%s", target.c_str(),
            layer.image_.width_, layer.image_.height_, bps, spp,
            sf == TINYTIFF_SAMPLEFORMAT_FLOAT ? "float"
            : sf == TINYTIFF_SAMPLEFORMAT_INT ? "int"
                                              : "uint" );
          if ( spp == 3 )
            layer.image_.format_ = PixFmtColorRGB8;
          else if ( spp == 4 )
            layer.image_.format_ = PixFmtColorRGBA8;
          else
            NEKO_EXCEPT( "Unsupported channel count in TIFF image" );
          vector<uint8_t> tiffSource;
          tiffSource.resize( layer.image_.width_ * layer.image_.height_ * ( bps / 8 ) );
          layer.image_.data_.resize( layer.image_.width_ * layer.image_.height_ * spp * sizeof( uint8_t ) );
          if ( bps == 16 && sf == TINYTIFF_SAMPLEFORMAT_UINT )
          {
            auto data = reinterpret_cast<uint8_t*>( layer.image_.data_.data() );
            auto source = reinterpret_cast<uint16_t*>( tiffSource.data() );
            for ( auto s = 0; s < spp; ++s )
            {
              TinyTIFFReader_getSampleData( tiff, tiffSource.data(), s );
              if ( TinyTIFFReader_wasError( tiff ) )
                NEKO_EXCEPT( "It's a fucking error" );
              for ( unsigned int y = 0; y < layer.image_.height_; ++y )
              {
                for ( unsigned int x = 0; x < layer.image_.width_; ++x )
                {
                  size_t p = ( y * layer.image_.width_ ) + x;
                  size_t d = ( ( ( y * layer.image_.width_ ) + x ) * spp ) + s;
                  // I'm practically entirely sure that this is wrong - whatever,
                  // fix when we actually need one of these textures
                  data[d] = math::iround( static_cast<Real>( source[p] ) / 256.0f );
                }
              }
            }
          }
          else if ( bps == 8 && sf == TINYTIFF_SAMPLEFORMAT_UINT )
          {
            auto data = reinterpret_cast<uint8_t*>( layer.image_.data_.data() );
            auto source = reinterpret_cast<uint8_t*>( tiffSource.data() );
            for ( auto s = 0; s < spp; ++s )
            {
              TinyTIFFReader_getSampleData( tiff, tiffSource.data(), s );
              if ( TinyTIFFReader_wasError( tiff ) )
                NEKO_EXCEPT( "It's a fucking error" );
              for ( unsigned int y = 0; y < layer.image_.height_; ++y )
              {
                for ( unsigned int x = 0; x < layer.image_.width_; ++x )
                {
                  size_t p = ( y * layer.image_.width_ ) + x;
                  size_t d = ( ( ( y * layer.image_.width_ ) + x ) * spp ) + s;
                  data[d] = static_cast<uint8_t>( source[p] );
                }
              }
            }
          }
          else
            NEKO_EXCEPT( "Unsupported bit/channel/sample format combination in TIFF image" );
          task.material_->layers_.push_back( move( layer ) );
          free( tiff );
        }
      }
      else
        NEKO_EXCEPT( "Unsupport image format in load texture task" );
    }

    task.material_->loaded_ = true;

    finishedTasksLock_.lock();
    finishedMaterials_.push_back( task.material_ );
    finishedTasksLock_.unlock();
  }

  // Context: Worker thread
  void ThreadedLoader::handleNewTasks()
  {
    LoadTaskVector newTasks;

    addTaskLock_.lock();
    newTasks.swap( newTasks_ );
    newTasksEvent_.reset();
    addTaskLock_.unlock();

    for ( auto& task : newTasks )
    {
      if ( task.type_ == LoadTask::Load_Texture )
      {
        loadTexture( task.textureLoad );
      }
      else if ( task.type_ == LoadTask::Load_Fontface )
      {
        loadFontFace( task.fontfaceLoad );
      }
      else if ( task.type_ == LoadTask::Load_Model )
      {
        loadModel( task.modelLoad );
      }
    }

    if ( !finishedMaterials_.empty() )
      finishedMaterialsEvent_.set();

    if ( !finishedFonts_.empty() )
      finishedFontsEvent_.set();

    if ( !finishedModels_.empty() )
      finishedModelsEvent_.set();

    if ( !finishedAnimations_.empty() )
      finishedAnimationsEvent_.set();
  }

  ThreadedLoader::~ThreadedLoader()
  {
  }

}