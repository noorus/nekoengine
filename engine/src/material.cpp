#include "pch.h"

#include "gfx_types.h"
#include "materials.h"
#include "renderer.h"
#include "neko_exception.h"
#include "console.h"
#include "loader.h"
#include "filesystem.h"

namespace neko {

  MaterialManager::MaterialManager( Renderer* renderer, ThreadedLoaderPtr loader ):
  LoadedResourceManagerBase<Material>( loader ), renderer_( renderer )
  {
  }

  const map<utf8String, Material::Type> c_materialTypes = {
    { "unlit", Material::Type::UnlitSimple },
    { "worldparticle", Material::Type::WorldParticle }
  };

  MaterialPtr MaterialManager::createTextureWithData( const utf8String& name, int width, int height, PixelFormat format,
    const void* data, const Texture::Wrapping wrapping, const Texture::Filtering filtering )
  {
    MaterialPtr mat = make_shared<Material>( name );
    mat->type_ = Material::UnlitSimple;
    MaterialLayer layer;
    layer.image_.format_ = format;
    layer.image_.width_ = (unsigned int)width;
    layer.image_.height_ = (unsigned int)height;
    layer.texture_ = make_shared<Texture>( renderer_, layer.image_.width_, layer.image_.height_, layer.image_.format_, data, wrapping, filtering );
    mat->layers_.push_back( move( layer ) );
    mat->loaded_ = true;
    map_[name] = mat;
    return mat;
  }

  void MaterialManager::loadJSONRaw( const nlohmann::json& obj )
  {
    if ( obj.is_array() )
    {
      for ( const auto& entry : obj )
      {
        if ( !entry.is_object() )
          NEKO_EXCEPT( "Material array entry is not an object" );
        loadJSONRaw( entry );
      }
    }
    else if ( obj.is_object() )
    {
      auto name = obj["name"].get<utf8String>();
      auto material = make_shared<Material>( name );
      const auto& type = obj["type"].get<utf8String>();
      if ( c_materialTypes.find( type ) == c_materialTypes.end() )
        NEKO_EXCEPT( "Unknown material type " + type );
      material->type_ = c_materialTypes.at( type );
      const auto& layers = obj["layers"];
      if ( !layers.is_array() )
        NEKO_EXCEPT( "Material layers is not an array" );
      vector<utf8String> textures;
      for ( auto& layer : layers )
        textures.push_back( layer.get<utf8String>() );
      map_[material->name()] = material;
      loader_->addLoadTask( { LoadTask( material, textures ) } );
    }
    else
      NEKO_EXCEPT( "Material JSON is not an array or an object" );
  }

  void MaterialManager::loadJSON( const utf8String& input )
  {
    auto parsed = nlohmann::json::parse( input );
    loadJSONRaw( parsed );
  }

  void MaterialManager::loadFile( const utf8String& filename )
  {
    auto input = move( Locator::fileSystem().openFile( Dir_Data, filename )->readFullString() );
    loadJSON( input );
  }

  MaterialManager::~MaterialManager()
  {
  }

}