#include "stdafx.h"
#include "loader.h"
#include "utilities.h"
#include "lodepng.h"
#include "gfx_types.h"
#include "renderer.h"
#include "console.h"

#include "tinytiffreader.hxx"
#include <ktx.h>
#pragma comment( lib, "ktx.lib" )

#include "tinyexr.h"

namespace neko {

  const string c_loaderThreadName = "nekoLoader";

  const char c_fontsBaseDirectory[] = R"(assets\fonts\)";
  const char c_texturesBaseDirectory[] = R"(assets\textures\)";
  const char c_meshesBaseDirectory[] = R"(assets\meshes\)";
  const char c_animationsBaseDirectory[] = R"(assets\animations\)";

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

  void ThreadedLoader::getFinishedModels( vector<SceneNode*>& models )
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
        for ( const auto& target : task.textureLoad.paths_ )
        {
          auto path = c_texturesBaseDirectory + target;

          vector<uint8_t> input;
          unsigned int width, height;
          platform::FileReader( path ).readFullVector( input );
          MaterialLayer layer;
          vector<uint8_t> rawData;
          auto ext = utils::extractExtension( path );
          /*if ( ext == L"ktx2" )
          {
            ktxTexture2* ktx;
            auto ret = ktxTexture2_CreateFromMemory( input.data(), input.size(), KTX_TEXTURE_CREATE_NO_FLAGS, &ktx );
            if ( ktx && ret == KTX_SUCCESS )
            {
              Locator::console().printf( Console::srcLoader, "ktx: libktx loaded %s, %ix%i depth %i %id %i faces",
                target.c_str(), ktx->baseWidth, ktx->baseHeight, ktx->baseDepth, ktx->numDimensions, ktx->numFaces );
              GLuint gl_id = 0;
              auto gl_tgt = gl::GLenum::GL_NONE;
              auto gl_err = gl::GLenum::GL_NONE;
              ret = ktxTexture_GLUpload( (ktxTexture*)ktx, &gl_id, (unsigned int*)&gl_tgt, (unsigned int*)&gl_err );
              if ( ret != KTX_SUCCESS )
                NEKO_OPENGL_EXCEPT( "ktxTexture_GLUpload failed", gl_err );
              //layer.preuploaded_ = gl_id;
              layer.image_.width_ = ktx->baseWidth;
              layer.image_.height_ = ktx->baseHeight;
              layer.image_.format_ = PixFmtColorRGBA16f;
              layer.image_.data_.resize( ktxTexture_GetDataSize( (ktxTexture*)ktx ) );
              auto source = ktxTexture_GetData( ktx );
              memcpy( layer.image_.data_.data(), source, layer.image_.data_.size() );
              //layer.image_.uploadedFormat_ = gl_tgt;
              task.textureLoad.material_->layers_.push_back( move( layer ) );
              ktxTexture_Destroy( (ktxTexture*)ktx );
            }
            else
              NEKO_EXCEPT( "KTX creation failed" );
          }
          else*/ if ( ext == L"png" )
          {
            if ( lodepng::decode( rawData, width, height, input.data(), input.size(), LCT_RGBA, 8 ) == 0 )
            {
              Locator::console().printf( Console::srcLoader, "png: lodepng loaded %s, %ix%i, rgba8", target.c_str(), width, height );
              layer.image_.width_ = width;
              layer.image_.height_ = height;
              layer.image_.format_ = PixFmtColorRGBA8;
              layer.image_.data_.reserve( rawData.size() );
              // flip rows
              for ( size_t i = 0; i < height; ++i )
              {
                auto srcRow = rawData.data() + ( i * (size_t)width * 4 );
                auto dstRow = layer.image_.data_.data() + ( ( (size_t)height - 1 - i ) * width * 4 );
                memcpy( dstRow, srcRow, (size_t)width * 4 );
              }
              task.textureLoad.material_->layers_.push_back( move( layer ) );
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
              Locator::console().printf( Console::srcLoader, "exr: tinyexr loaded %s, %ix%i, rgba32f", target.c_str(), exr_width, exr_height );
              layer.image_.width_ = exr_width;
              layer.image_.height_ = exr_height;
              layer.image_.data_.resize( exr_width * exr_height * 4 * sizeof( float ) );
              layer.image_.format_ = PixFmtColorRGBA32f;
              memcpy( layer.image_.data_.data(), exr_values, layer.image_.data_.size() );
              free( exr_values );
              task.textureLoad.material_->layers_.push_back( move( layer ) );
            }
            /*const char* miniexr_err = nullptr;
            size_t data_size;
            EXRVersion version;
            auto ret = ParseEXRVersionFromMemory( &version, input.data(), input.size() );
            if ( ret != TINYEXR_SUCCESS )
              NEKO_EXCEPT( "ParseEXRVersionFromMemory failed" );
            if ( version.multipart || version.non_image )
              NEKO_EXCEPT( "EXR is multipart or non-image" );
            EXRHeader header;
            InitEXRHeader( &header );
            ret = ParseEXRHeaderFromMemory( &header, &version, input.data(), input.size(), &miniexr_err );
            if ( ret != TINYEXR_SUCCESS )
              NEKO_EXCEPT( "ParseEXRHeaderFromMemory failed" );
            EXRImage image;
            InitEXRImage( &image );
            ret = LoadEXRImageFromMemory( &image, &header, input.data(), input.size(), &miniexr_err );
            if ( ret != TINYEXR_SUCCESS )
              NEKO_EXCEPT( "LoadEXRImageFromMemory failed" );
            image.
            FreeEXRImage( &image );
            FreeEXRHeader( &header );*/
          }
          else if ( ext == L"tif" )
          {
            auto tiff = TinyTIFFReader_open( platform::utf8ToWide( path ).c_str() );
            if ( tiff )
            {
              layer.image_.width_ = TinyTIFFReader_getWidth( tiff );
              layer.image_.height_ = TinyTIFFReader_getHeight( tiff );
              auto bps = TinyTIFFReader_getBitsPerSample( tiff, 0 );
              auto sf = TinyTIFFReader_getSampleFormat( tiff );
              auto spp = TinyTIFFReader_getSamplesPerPixel( tiff );
              Locator::console().printf( Console::srcLoader, "tif: tinytiff loaded %s, %ix%i %ibit %ix%s",
                target.c_str(), layer.image_.width_, layer.image_.height_, bps, spp,
                sf == TINYTIFF_SAMPLEFORMAT_FLOAT ? "float" : sf == TINYTIFF_SAMPLEFORMAT_INT ? "int" : "uint", spp );
              vector<uint8_t> tiffSource;
              tiffSource.resize( layer.image_.width_ * layer.image_.height_ * ( bps / 8 ) );
              if ( bps == 16 && sf == TINYTIFF_SAMPLEFORMAT_UINT )
              {
                layer.image_.format_ = PixFmtColorRGBA8;
                layer.image_.data_.resize( layer.image_.width_ * layer.image_.height_ * spp * sizeof( uint8_t ) );
                auto data = reinterpret_cast<uint8_t*>( layer.image_.data_.data() );
                auto source = reinterpret_cast<uint16_t*>( tiffSource.data() );
                for ( auto s = 0; s < spp; ++s )
                {
                  TinyTIFFReader_getSampleData( tiff, tiffSource.data(), s );
                  if ( TinyTIFFReader_wasError( tiff ) )
                  {
                    NEKO_EXCEPT( "It's a fucking error" );
                  }
                  uint16_t maxval = 0;
                  for ( unsigned int y = 0; y < layer.image_.height_; ++y )
                  {
                    for ( unsigned int x = 0; x < layer.image_.width_; ++x )
                    {
                      size_t p = ( y * layer.image_.width_ ) + x;
                      size_t d = ( ( ( y * layer.image_.width_ ) + x ) * spp ) + s;
                      if ( source[p] > maxval )
                        maxval = source[p];
                      data[d] = static_cast<uint8_t>( source[p] );
                    }
                  }
                  Locator::console().printf( Console::srcLoader, "tif: maximum value was %i", maxval );
                }
              }
              task.textureLoad.material_->layers_.push_back( move( layer ) );
              free( tiff );
            }
          }
          else
          {
            NEKO_EXCEPT( "Unsupport image format in load texture task" );
          }
        }
        task.textureLoad.material_->loaded_ = true;
        finishedTasksLock_.lock();
        finishedMaterials_.push_back( task.textureLoad.material_ );
        finishedTasksLock_.unlock();
      }
      else if ( task.type_ == LoadTask::Load_Fontface )
      {
        auto path = c_fontsBaseDirectory + task.fontfaceLoad.path_;

        vector<uint8_t> input;
        platform::FileReader( path ).readFullVector( input );
        task.fontfaceLoad.font_->manager_->loadFont( task.fontfaceLoad.font_, task.fontfaceLoad.specs_, input );
        finishedTasksLock_.lock();
        finishedFonts_.push_back( task.fontfaceLoad.font_ );
        finishedTasksLock_.unlock();
      }
      else if ( task.type_ == LoadTask::Load_Model )
      {
        auto path = c_meshesBaseDirectory + task.modelLoad.path_;
        auto filename = utils::extractFilename( path );
        auto filepath = utils::extractFilepath( path );
        auto ext = utils::extractExtension( path );

        if ( ext == L"gltf" )
        {
          vector<uint8_t> input;
          platform::FileReader( path ).readFullVector( input );
          loaders::loadGLTFModel( input, platform::wideToUtf8( filename ), platform::wideToUtf8( filepath ), task.modelLoad.node_ );
        }
        else
          NEKO_EXCEPT( "Unsupported mesh format in load model task" );
        //platform::FileReader( task.modelLoad.path_ ).readFullVector( input );
        /*fbxLoader_.loadFBXScene( input, path, task.modelLoad.node_ );*/
        finishedTasksLock_.lock();
        finishedModels_.push_back( task.modelLoad.node_ );
        finishedTasksLock_.unlock();
      }
      else if ( task.type_ == LoadTask::Load_Animation )
      {
        auto path = c_animationsBaseDirectory + task.animationLoad.path_;

        vector<uint8_t> input;
        platform::FileReader( path ).readFullVector( input );
        loaders::loadUnityAnimation( input, task.animationLoad.animation_ );
        finishedTasksLock_.lock();
        finishedAnimations_.push_back( task.animationLoad.animation_ );
        finishedTasksLock_.unlock();
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