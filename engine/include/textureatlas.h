#pragma once
#include "neko_types.h"
#include "gfx_types.h"
#include "surface.h"

namespace neko {

  class TextureAtlas {
  private:
    vector<vec3i> nodes_;
    vec2i size_;
    int depth_;
    size_t used_;
    vector<uint8_t> data_;
    bool dirty_;
  public:
    TextureAtlas( const vec2i& size, int depth );
    ~TextureAtlas();
    void setRegion( int x, int y, int width, int height, const uint8_t* data, size_t stride );
    int fit( size_t index, int width, int height );
    void merge();
    vec4i getRegion( int width, int height );
    void clear();
  public:
    inline int depth() const noexcept { return depth_; }
    inline uint8_t* data() { return data_.data(); }
    inline vec2 fdimensions() const { return { static_cast<Real>( size_.x ), static_cast<Real>( size_.y ) }; }
    PixelFormat format() const;
    vec2i dimensions() const;
    const uint8_t* data() const;
    int bytesize() const;
    bool dirty() const;
    void markClean();
  };

  using TextureAtlasPtr = shared_ptr<TextureAtlas>;

}