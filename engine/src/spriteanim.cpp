#include "pch.h"
#include "locator.h"
#include "spriteanim.h"
#include "neko_exception.h"
#include "console.h"
#include "engine.h"
#include "loader.h"
#include "renderer.h"
#include "json.h"
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
      auto def = make_shared<SpriteAnimationDefinition>();
      def->name_ = name;
      def->width_ = obj["width"].get<uint32_t>();
      def->height_ = obj["height"].get<uint32_t>();
      def->frameCount_ = obj["frames"].get<uint32_t>();
      animdefs_[def->name()] = def;
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
    auto input = Locator::fileSystem().openFile( Dir_Data, filename )->readFullString();
    loadAnimdefJSON( input );
  }

  SpriteAnimationSetDefinitionEntry::SpriteAnimationSetDefinitionEntry( const utf8String& name,
    const utf8String& sheetname, const utf8String& defname, const vec2i& sheetpos, const vector<int>& flipframesx ):
    name_( name ), sheetName_( sheetname ), defName_( defname ), sheetPos_( sheetpos ), flipFramesX_( flipframesx )
  {
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
        if ( !setvalue.is_object() )
          NEKO_EXCEPT( "Sprite animation set object entry is not an object" );
        auto set = make_shared<SpriteAnimationSetDefinition>();
        set->name_ = setkey;
        for ( const auto& [key, value] : setvalue.items() )
        {
          auto sheetname = value["sheet"].get<utf8String>();
          auto defname = value["animdef"].get<utf8String>();
          vec2i sheetpos;
          if ( value.contains( "sheetpos" ) )
            sheetpos = njson::readVec2i( value["sheetpos"] );
          vector<int> flipx;
          if ( value.contains( "flip-frames-x" ) )
            flipx = njson::readVector<int>( value["flip-frames-x"] );
          auto def = make_shared<SpriteAnimationSetDefinitionEntry>( key, sheetname, defname, sheetpos, flipx );
          def->definition_ = animdefs_.at( def->defName_ );
          auto matname = set->name_ + "_" + def->name_;
          def->material_ = renderer_->materials().createMaterial( matname );
          set->entries_[def->name_] = def;
        }
        renderer_->loader()->addLoadTask( { LoadTask( set ) } );
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
    auto input = Locator::fileSystem().openFile( Dir_Data, filename )->readFullString();
    loadAnimsetJSON( input );
  }

  SpriteManager::~SpriteManager() {}

}