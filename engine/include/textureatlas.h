#pragma once
#include "utilities.h"
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"

namespace neko {

  struct TextureAtlas
  {
    vector<vec3i> nodes_;
    size_t width_;
    size_t height_;
    size_t depth_;
    size_t used_;
    GLuint id_;
    vector<uint8_t> data_;
    TextureAtlas( const size_t width, const size_t height, const size_t depth );
    void setRegion( const size_t x, const size_t y, const size_t width, const size_t height, const uint8_t* data, const size_t stride );
    int fit( const size_t index, const size_t width, const size_t height );
    void merge();
    vec4i getRegion( const size_t width, const size_t height );
    void clear();
  };

  using TextureAtlasPtr = shared_ptr<TextureAtlas>;

}