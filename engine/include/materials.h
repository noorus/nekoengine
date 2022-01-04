#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "texture.h"
#include "resources.h"

namespace neko {

  struct MaterialLayer: public nocopy {
  public:
    ImageData image_;
    TexturePtr texture_;
  public:
    inline const bool hasHostCopy() const noexcept { return !image_.data_.empty(); }
    inline void deleteHostCopy()
    {
      image_.data_.clear();
    }
    inline const bool uploaded() const noexcept { return ( texture_.get() != nullptr ); }
    MaterialLayer() {}
    // move constructor
    MaterialLayer( MaterialLayer&& rhs ) noexcept:
    image_( move( rhs.image_ ) ), texture_( rhs.texture_ )
    {
    }
    inline size_t width() const noexcept { return image_.width_; }
    inline size_t height() const noexcept { return image_.height_; }
  };

  using MaterialLayers = vector<MaterialLayer>;

  class Material: public LoadedResourceBase<Material> {
  public:
    using Base = LoadedResourceBase<Material>;
    using Base::Ptr;
  public:
    friend class MaterialManager;
    friend class ThreadedLoader;
  public:
    Material() = delete;
    explicit Material( const utf8String& name ):
      LoadedResourceBase<Material>( name ) {}
    enum Type
    {
      UnlitSimple,
      WorldParticle
    } type_ = UnlitSimple;
    Texture::Wrapping wantWrapping_ = Texture::Repeat;
    Texture::Filtering wantFiltering_ = Texture::Mipmapped;
    MaterialLayers layers_;
    inline const bool uploaded() const noexcept
    {
      if ( !loaded_ )
        return false;
      for ( const auto& layer : layers_ )
        if ( !layer.uploaded() )
          return false;
      return true;
    }
    inline GLuint textureHandle( size_t index ) const
    {
      assert( index < layers_.size() && layers_[index].uploaded() );
      return layers_[index].texture_->handle();
    }
    inline const MaterialLayer& layer( size_t index ) const
    {
      assert( index < layers_.size() && layers_[index].uploaded() );
      return layers_[index];
    }
  };

  using MaterialPtr = Material::Ptr;
  using MaterialVector = vector<MaterialPtr>;
  using MaterialMap = map<utf8String, MaterialPtr>;

  class MaterialManager: public LoadedResourceManagerBase<Material> {
  public:
    using Base = LoadedResourceManagerBase<Material>;
    using ResourcePtr = Base::ResourcePtr;
    using MapType = Base::MapType;
  private:
    Renderer* renderer_;
  public:
    MaterialManager( Renderer* renderer, ThreadedLoaderPtr loader );
    MaterialPtr createTextureWithData( const utf8String& name, size_t width, size_t height, PixelFormat format, const void* data, const Texture::Wrapping wrapping, const Texture::Filtering filtering );
    void loadJSONRaw( const nlohmann::json& arr );
    void loadJSON( const utf8String& input );
    void loadFile( const utf8String& filename );
    ~MaterialManager();
  };

  using MaterialManagerPtr = shared_ptr<MaterialManager>;

}