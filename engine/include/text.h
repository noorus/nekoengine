#pragma once
#include "forwards.h"
#include "gfx_types.h"
#include "neko_types.h"
#include "neko_platform.h"
#include "messaging.h"
#include "input.h"
#include "camera.h"
#include "gui.h"

namespace neko {

  /*class DynamicText {
  public:
    DynamicMeshPtr mesh_;
    FontPtr font_;
    MaterialPtr material_;
  public:
    DynamicText( ThreadedLoaderPtr loader, MeshManagerPtr meshmgr, FontManagerPtr fontmgr )
    {
      font_ = fontmgr->createFont();
      loader->addLoadTask( { LoadTask( font_, R"(LuckiestGuy.ttf)", 32.0f ) } );
      mesh_ = meshmgr->createDynamic( GL_TRIANGLES, VBO_3D, true, false );
    }
    inline bool fontLoaded()
    {
      return font_ && font_->loaded_;
    }
    void addText( const utf8String& str, vec2 pen )
    {
      uint32_t prev_codepoint = 0;
      for ( size_t i = 0; i < str.length(); ++i )
      {
        auto codepoint = utils::utf8_to_utf32( &str[i] );
        auto glyph = font_->impl_->getGlyph( codepoint );
        if ( !glyph )
        {
          font_->impl_->loadGlyph( codepoint );
          glyph = font_->impl_->getGlyph( codepoint );
        }
        assert( glyph );
        Real kerning = 0.0f;
        if ( i > 0 )
          kerning = glyph->getKerning( prev_codepoint );
        prev_codepoint = codepoint;
        pen.x += kerning;
        auto p0 = vec2(
          ( pen.x + glyph->offset.x ),
          (int)( pen.y + glyph->offset.y ) );
        auto p1 = vec2(
          ( p0.x + glyph->width ),
          (int)( p0.y - glyph->height )
        );
        auto index = (GLuint)mesh_->vertsCount();
        auto normal = vec3( 1.0f, 0.0f, 0.0f );
        auto color = vec4( 0.0f, 1.0f, 0.0f, 1.0f );
        vector<Vertex3D> vertices = {
          { vec3( p0.x, p1.y, 0.0f ), normal, vec2( glyph->coords[0].x, glyph->coords[1].y ), color },
          { vec3( p0.x, p0.y, 0.0f ), normal, vec2( glyph->coords[0].x, glyph->coords[0].y ), color },
          { vec3( p1.x, p0.y, 0.0f ), normal, vec2( glyph->coords[1].x, glyph->coords[0].y ), color },
          { vec3( p1.x, p1.y, 0.0f ), normal, vec2( glyph->coords[1].x, glyph->coords[1].y ), color }
        };
        vector<GLuint> indices = {
          index + 0, index + 1, index + 2, index + 0, index + 2, index + 3
        };
        mesh_->pushIndices( move( indices ) );
        mesh_->pushVertices( move( vertices ) );
        pen.x += glyph->advance.x;
      }
    }
    void updateTexture( Renderer* rndr )
    {
      material_ = rndr->createTextureWithData( font_->impl_->atlas_->width_, font_->impl_->atlas_->height_,
        PixFmtColorR8, (const void*)font_->impl_->atlas_->data_.data(), Texture::ClampBorder, Texture::Mipmapped );
      /*platform::FileWriter writer("debug.png");
      vector<uint8_t> buffer;
      lodepng::encode( buffer, font_->impl_->atlas_->data_, font_->impl_->atlas_->width_, font_->impl_->atlas_->height_, LCT_GREY, 8 );
      writer.writeBlob( buffer.data(), buffer.size() );
    }
    void begin()
    {
      mesh_->begin();
    }
    void draw()
    {
      glBindTextureUnit( 0, material_->layers_[0].texture_->handle() );
      mesh_->draw();
      glBindTextureUnit( 0, 0 );
    }
    ~DynamicText()
    {
      //
    }
  };*/

  /*if ( g_testText && g_textAdded )
  {
    g_testText->begin();
    mat4 model( 1.0f );
    model = glm::scale( model, vec3( 1.0f, 1.0f, 1.0f ) );
    model = glm::translate( model, vec3( 1.0f, 1.0f, 0.0f ) );

    auto& pipeline = shaders_->usePipeline( "text3d" );
    pipeline.setUniform( "model", model );
    pipeline.setUniform( "tex", 0 );
    g_testText->draw();
  }*/

}