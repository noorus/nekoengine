#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "texture.h"

namespace neko {

  struct MaterialLayer: public nocopy {
  public:
    ImageData image_;
    TexturePtr texture_;
  public:
    inline const bool hasHostCopy() const { return !image_.data_.empty(); }
    inline const bool uploaded() const throw() { return ( texture_.get() != nullptr ); }
    MaterialLayer() {}
    // move constructor
    MaterialLayer( MaterialLayer&& rhs ): image_( move( rhs.image_ ) ), texture_( rhs.texture_ )
    {
    }
  };

  using MaterialLayers = vector<MaterialLayer>;

  struct Material: public nocopy
  {
    enum Type
    {
      UnlitSimple,
      WorldGround,
      WorldPBR,
      WorldUntexturedPBS
    } type_ = UnlitSimple;
    utf8String name_;
    bool loaded_ = false;
    MaterialLayers layers_;
    inline const bool uploaded() const throw()
    {
      if ( !loaded_ )
        return false;
      for ( const auto& layer : layers_ )
        if ( !layer.uploaded() )
          return false;
      return true;
    }
  };

  using MaterialPtr = shared_ptr<Material>;
  using MaterialVector = vector<MaterialPtr>;
  using MaterialMap = map<utf8String, MaterialPtr>;

  class MaterialManager : public nocopy {
  protected:
    ThreadedLoaderPtr loader_;
    Renderer* renderer_ = nullptr;
    MaterialMap map_;
  public:
    MaterialManager( Renderer* renderer, ThreadedLoaderPtr loader );
    MaterialPtr createTextureWithData( const utf8String& name, size_t width, size_t height, PixelFormat format, const void* data, const Texture::Wrapping wrapping, const Texture::Filtering filtering );
    void loadJSONRaw( const nlohmann::json& arr );
    void loadJSON( const utf8String& input );
    void loadFile( const utf8String& filename );
    inline const Material* get( const utf8String& name ) const
    {
      auto it = map_.find( name );
      if ( it == map_.end() )
        return nullptr;
      return ( ( *it ).second.get() );
    }
    ~MaterialManager();
  };

  using MaterialManagerPtr = shared_ptr<MaterialManager>;

}