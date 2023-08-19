#include "pch.h"
#include "locator.h"
#include "spriteanim.h"
#include "neko_exception.h"
#include "console.h"
#include "engine.h"
#include "loader.h"
#include "renderer.h"
#include "materials.h"

namespace neko {

  SpriteManager::SpriteManager( Renderer* renderer ): renderer_( renderer )
  {
  }

  void SpriteManager::initialize()
  {

  }

  void SpriteManager::shutdown()
  {

  }

  void SpriteManager::loadAnimdefJSONRaw( const json& obj )
  {
    if ( obj.is_array() )
    {
      for ( const auto& entry : obj )
      {
        if ( !entry.is_object() )
          NEKO_EXCEPT( "Sprite animation definition array entry is not an object" );
        loadAnimdefJSONRaw( entry );
      }
    }
    else if ( obj.is_object() )
    {
      auto name = obj["name"].get<utf8String>();
      if ( animdefs_.find( name ) != animdefs_.end() )
        NEKO_EXCEPT( "Sprite animation definition already exists: " + name );
      SpriteAnimationDefinition def;
      def.name_ = name;
      def.width_ = obj["width"].get<uint32_t>();
      def.height_ = obj["height"].get<uint32_t>();
      def.frameCount_ = obj["frames"].get<uint32_t>();
      animdefs_[def.name()] = move( def );
    }
    else
      NEKO_EXCEPT( "Sprite animation definition JSON is not an array or an object" );
  }

  void SpriteManager::loadAnimdefJSON( const utf8String& input )
  {
    auto parsed = nlohmann::json::parse( input );
    loadAnimdefJSONRaw( parsed );
  }

  void SpriteManager::loadAnimdefFile( const utf8String& filename )
  {
    auto input = move( Locator::fileSystem().openFile( Dir_Data, filename )->readFullString() );
    loadAnimdefJSON( input );
  }

  void SpriteManager::draw( GameTime time )
  {
    auto& mat = renderer_->useMaterial( "demo_shroom" );
    auto ppl = &renderer_->shaders().usePipeline( "shroom" );
    auto lr = math::iround( (Real)time ) % 4;
    
    ppl->setUniform( "tex_layer", 0 );
  }

  inline vec2 readJSONVec2( const json& obj )
  {
    if ( !obj.is_object() )
      NEKO_EXCEPT( "Expected JSON vec2 is not an object" );
    vec2 ret = { 0.0f, 0.0f };
    if ( obj.contains( "x" ) )
      ret.x = obj["x"].get<float>();
    if ( obj.contains( "y" ) )
      ret.y = obj["y"].get<float>();
    return ret;
  }

  inline vec2i readJSONVec2i( const json& obj )
  {
    if ( !obj.is_object() )
      NEKO_EXCEPT( "Expected JSON vec2i is not an object" );
    vec2i ret = { 0, 0 };
    if ( obj.contains( "x" ) )
      ret.x = math::iround( obj["x"].get<float>() );
    if ( obj.contains( "y" ) )
      ret.y = math::iround( obj["y"].get<float>() );
    return ret;
  }

  template <typename T>
  inline vector<T> readJSONVector( const json& arr )
  {
    vector<T> ret;
    if ( !arr.is_array() )
      NEKO_EXCEPT( "Expected JSON array is not an array" );
    for ( const auto& entry : arr )
      ret.push_back( entry.get<T>() );
    return ret;
  }

  void SpriteManager::loadAnimsetJSONRaw( const json& obj )
  {
    if ( obj.is_array() )
    {
      for ( const auto& entry : obj )
      {
        if ( !entry.is_object() )
          NEKO_EXCEPT( "Sprite animation set array entry is not an object" );
        loadAnimsetJSONRaw( entry );
      }
    }
    else if ( obj.is_object() )
    {
      for ( const auto& [setkey, setvalue] : obj.items() )
      {
        map<utf8String, SpriteAnimationSetDefinitionEntry> entries;
        if ( !setvalue.is_object() )
          NEKO_EXCEPT( "Sprite animation set object entry is not an object" );
        for ( const auto& [key, value] : setvalue.items() )
        {
          SpriteAnimationSetDefinitionEntry def;
          def.name_ = key;
          auto sheetname = value["sheet"].get<utf8String>();
          def.defName_ = value["animdef"].get<utf8String>();
          if ( value.contains( "sheetpos" ) )
            def.sheetPos_ = readJSONVec2i( value["sheetpos"] );
          if ( value.contains( "flip-frames-x" ) )
            def.flipFramesX_ = readJSONVector<int>( value["flip-frames-x"] );
          auto mat = renderer_->materials().get( sheetname );
          if ( mat )
          {
            const auto& adef = animdefs_.at( def.defName_ );
            auto sheet = Pixmap::from( mat->layer( 0 ).image() );
            sheet.flipVertical();
            Pixmap cut( sheet, def.sheetPos_.x, def.sheetPos_.y, adef.frameCount() * adef.width(), adef.height() );
            auto matname = setkey + "_" + def.name_;
            cut.writePNG( matname + ".png" );
            def.material_ = renderer_->createTextureWithData( matname, adef.width(), adef.height(),
              adef.frameCount(), cut.format(), cut.data().data(), Texture::ClampBorder, Texture::Nearest );
          }
          entries[key] = move( def );
        }
        setdefs_[setkey] = move( entries );
      }
    }
    else
      NEKO_EXCEPT( "Sprite animation set JSON is not an array or an object" );
  }

  void SpriteManager::loadAnimsetJSON( const utf8String& input )
  {
    auto parsed = nlohmann::json::parse( input );
    loadAnimsetJSONRaw( parsed );
  }

  void SpriteManager::loadAnimsetFile( const utf8String& filename )
  {
    auto input = move( Locator::fileSystem().openFile( Dir_Data, filename )->readFullString() );
    loadAnimsetJSON( input );
  }

  SpriteManager::~SpriteManager() {}

}