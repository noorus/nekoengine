#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "texture.h"
#include "resources.h"

namespace neko {

  struct MaterialLayer: public nocopy {
  public:
    Pixmap image_;
    TexturePtr texture_;
  public:
    inline const bool hasHostCopy() const noexcept { return !image_.empty(); }
    inline void deleteHostCopy()
    {
      image_.reset();
    }
    inline const bool uploaded() const noexcept { return ( texture_.get() != nullptr ); }
    MaterialLayer(): image_( PixFmtColorRGBA8 ) {}
    MaterialLayer( Pixmap&& from ): image_( move( from ) ) {}
    // move constructor
    MaterialLayer( MaterialLayer&& rhs ) noexcept:
    image_( move( rhs.image_ ) ), texture_( rhs.texture_ )
    {
    }
    inline int width() const noexcept { return image_.width(); }
    inline int height() const noexcept { return image_.height(); }
    inline const Pixmap& image() const noexcept { return image_; }
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
    int width_ = 0;
    int height_ = 0;
    int arrayDepth_ = 1;
    MaterialLayers layers_;
    inline const bool uploaded() const noexcept
    {
      if ( !loaded_ )
        return false;
      for ( const auto& layer : layers_ )
        if ( !layer.uploaded() || !layer.texture_ )
          return false;
      return true;
    }
    inline int width() const noexcept { return width_; }
    inline int height() const noexcept { return height_; }
    inline int depth() const noexcept { return arrayDepth_; }
    inline GLuint textureHandle( size_t index ) const
    {
      assert( index < layers_.size() && layers_[index].uploaded() );
      return layers_[index].texture_->handle();
    }
    inline const MaterialLayer& layer( size_t index ) const
    {
      assert( index < layers_.size() );
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
    MaterialPtr createTextureWithData( const utf8String& name, int width, int height, PixelFormat format, const void* data,
      const Texture::Wrapping wrapping, const Texture::Filtering filtering );
    MaterialPtr createTextureWithData( const utf8String& name, int width, int height, int depth,
      PixelFormat format, const void* data, const Texture::Wrapping wrapping, const Texture::Filtering filtering );
    MaterialPtr createMaterial( const utf8String& name );
    void loadJSONRaw( const nlohmann::json& arr );
    void loadJSON( const utf8String& input );
    void loadFile( const utf8String& filename );
    ~MaterialManager();
  };

  using MaterialManagerPtr = shared_ptr<MaterialManager>;

}