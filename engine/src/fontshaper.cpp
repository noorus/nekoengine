#include "stdafx.h"
#include "locator.h"
#include "fontmanager.h"
#include "neko_exception.h"
#include "renderer.h"

namespace neko {

  HBShaper::HBShaper( const string& fontFile, FontManagerPtr fontLib ): lib( fontLib )
  {
    float size = 50;
    vector<uint8_t> input;
    platform::FileReader( fontFile ).readFullVector( input );
    face = move( lib->loadFace( input.data(), input.size(), size * 64, 72, 72 ) );
  }

  void HBShaper::addFeature( hb_feature_t feature )
  {
    features.push_back( feature );
  }

  vector<Mesh*> HBShaper::drawText( RendererPtr renderer, HBText& text, float x, float y )
  {
    vector<Mesh*> meshes;

    hb_buffer_reset( buffer );

    hb_buffer_set_direction( buffer, text.direction );
    hb_buffer_set_script( buffer, text.script );
    hb_buffer_set_language( buffer, hb_language_from_string( text.language.c_str(), text.language.size() ) );
    size_t length = text.data.size();

    hb_buffer_add_utf8( buffer, text.c_data(), length, 0, length );

    // harfbuzz shaping
    hb_shape( font, buffer, features.empty() ? NULL : &features[0], features.size() );

    unsigned int glyphCount;
    hb_glyph_info_t *glyphInfo = hb_buffer_get_glyph_infos( buffer, &glyphCount );
    hb_glyph_position_t *glyphPos = hb_buffer_get_glyph_positions( buffer, &glyphCount );

    for ( int i = 0; i < glyphCount; ++i )
    {
      auto glyph = move( lib->rasterize( face, glyphInfo[i].codepoint ) );

      int twidth = pow( 2, ceil( log( glyph.width ) / log( 2 ) ) );
      int theight = pow( 2, ceil( log( glyph.height ) / log( 2 ) ) );

      auto tdata = new unsigned char[twidth * theight]();

      for ( int iy = 0; iy < glyph.height; ++iy )
      {
        memcpy( tdata + iy * twidth, glyph.buffer + iy * glyph.width, glyph.width );
      }

      float s0 = 0.0;
      float t0 = 0.0;
      float s1 = (float)glyph.width / twidth;
      float t1 = (float)glyph.height / theight;
      float xa = (float)glyphPos[i].x_advance / 64;
      float ya = (float)glyphPos[i].y_advance / 64;
      float xo = (float)glyphPos[i].x_offset / 64;
      float yo = (float)glyphPos[i].y_offset / 64;
      float x0 = x + xo + glyph.bearing_x;
      float y0 = floor( y + yo + glyph.bearing_y );
      float x1 = x0 + glyph.width;
      float y1 = floor( y0 - glyph.height );

      auto vertices = new Vertex2D[4];
      vertices[0] = Vertex2D( x0, y0, s0, t0 );
      vertices[1] = Vertex2D( x0, y1, s0, t1 );
      vertices[2] = Vertex2D( x1, y1, s1, t1 );
      vertices[3] = Vertex2D( x1, y0, s1, t0 );

      unsigned short* indices = new unsigned short[6];
      indices[0] = 0; indices[1] = 1;
      indices[2] = 2; indices[3] = 0;
      indices[4] = 2; indices[5] = 3;

      Mesh* m = new Mesh;

      m->indices = indices;
      m->textureData = tdata;

      auto mat = renderer->createTextureWithData( twidth, theight, Surface::PixelFormat::PixFmtColorRGBA8_A8Input, tdata );

      // don't do this!! use atlas texture instead
      m->textureId = mat->texture_->handle();

      m->vertices = vertices;
      m->nbIndices = 6;
      m->nbVertices = 4;

      meshes.push_back( m );

      x += xa;
      y += ya;
    }

    return meshes;
  }

  void HBShaper::init()
  {
    font = hb_ft_font_create( face->face_, NULL );
    buffer = hb_buffer_create();

    hb_buffer_allocation_successful( buffer );
  }

  HBShaper::~HBShaper()
  {
    lib->freeFace( face );

    hb_buffer_destroy( buffer );
    hb_font_destroy( font );
  }

}