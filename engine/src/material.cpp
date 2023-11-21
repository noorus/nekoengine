#include "pch.h"

#include "gfx_types.h"
#include "materials.h"
#include "renderer.h"
#include "neko_exception.h"
#include "console.h"
#include "loader.h"
#include "filesystem.h"

namespace neko {

  static const map<utf8String, Texture::Wrapping> c_wrappingMap = {
    { "clamp_edge", Texture::Wrapping::ClampEdge },
    { "clamp_border", Texture::Wrapping::ClampBorder },
    { "mirrored_repeat", Texture::Wrapping::MirroredRepeat },
    { "repeat", Texture::Wrapping::Repeat },
    { "mirrored_clamp_edge", Texture::Wrapping::MirroredClampEdge }
  };

  static const map<utf8String, Texture::Filtering> c_filteringMap = {
    { "nearest", Texture::Filtering::Nearest },
    { "linear", Texture::Filtering::Linear },
    { "mipmapped", Texture::Filtering::Mipmapped }
  };

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
    mat->width_ = width;
    mat->height_ = height;
    MaterialLayer layer( move( Pixmap( width, height, format, static_cast<uint8_t*>( const_cast<void*>( data ) ) ) ) );
    layer.texture_ = make_shared<Texture>(
      renderer_, layer.width(), layer.height(), layer.image_.format(), layer.image_.data().data(), wrapping, filtering );
    mat->layers_.push_back( move( layer ) );
    mat->loaded_ = true;
    map_[name] = mat;
    return mat;
  }

  MaterialPtr MaterialManager::createTextureWithData( const utf8String& name, int width, int height, int depth,
    PixelFormat format, const void* data, const Texture::Wrapping wrapping, const Texture::Filtering filtering )
  {
    MaterialPtr mat = make_shared<Material>( name );
    mat->type_ = Material::UnlitSimple;
    mat->width_ = width;
    mat->height_ = height;
    MaterialLayer layer( move( Pixmap( width * depth, height, format, static_cast<uint8_t*>( const_cast<void*>( data ) ) ) ) );
    layer.texture_ = make_shared<Texture>(
      renderer_, width, layer.height(), depth, layer.image_.format(), layer.image_.data().data(), wrapping, filtering );
    mat->layers_.push_back( move( layer ) );
    mat->loaded_ = true;
    map_[name] = mat;
    return mat;
  }

  MaterialPtr MaterialManager::createMaterial( const utf8String& name )
  {
    MaterialPtr mat = make_shared<Material>( name );
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
      material->wantFiltering_ = Texture::Filtering::Mipmapped;
      material->wantWrapping_ = Texture::Wrapping::Repeat;
      const auto& type = obj["type"].get<utf8String>();
      if ( c_materialTypes.find( type ) == c_materialTypes.end() )
        NEKO_EXCEPT( "Unknown material type " + type );
      material->type_ = c_materialTypes.at( type );
      if ( obj.contains( "wrapping" ) )
      {
        const auto& wrap = obj["wrapping"].get<utf8String>();
        if ( c_wrappingMap.contains( wrap ) )
          material->wantWrapping_ = c_wrappingMap.at( wrap );
        else
          Locator::console().printf(
            srcGfx, R"(Warning: Unknown material wrapping "%s" for material "%s")", wrap.c_str(), name.c_str() );
      }
      if ( obj.contains( "filtering" ) )
      {
        const auto& flt = obj["filtering"].get<utf8String>();
        if ( c_filteringMap.contains( flt ) )
          material->wantFiltering_ = c_filteringMap.at( flt );
        else
          Locator::console().printf(
            srcGfx, R"(Warning: Unknown material filtering "%s" for material "%s")", flt.c_str(), name.c_str() );
      }
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
    auto input = Locator::fileSystem().openFile( Dir_Data, filename )->readFullString();
    loadJSON( input );
  }

  MaterialManager::~MaterialManager()
  {
  }

}