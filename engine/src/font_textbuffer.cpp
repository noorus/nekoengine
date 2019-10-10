#include "stdafx.h"
#include "locator.h"
#include "fontmanager.h"
#include "neko_exception.h"
#include "console.h"

namespace neko {

  namespace ftgl {

    // TextBuffer

    TextBuffer::TextBuffer()
    {
      //buffer = make_shared<VertexBuffer>();
      line_start = 0;
      line_ascender = 0;
      base_color = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
      line_descender = 0;
      bounds = vec4( 0.0f, 0.0f, 0.0f, 0.0f );
    }

    TextBuffer::~TextBuffer()
    {
      clear();
      //buffer.reset();
    }

    void TextBuffer::addText( vec2 pen, Markup& markup, const char* text, size_t length )
    {
    }

    void TextBuffer::addChar( vec2 pen, Markup& markup, const char* current, const char* previous )
    {
      //
    }

    void TextBuffer::bufferAlign( vec2 pen, Alignment alignment )
    {
      //
    }

    vec4 TextBuffer::getBounds( vec2 pen )
    {
      return vec4();
    }

    void TextBuffer::clear()
    {
      // buffer_clear
      line_start = 0;
      line_ascender = 0;
      line_descender = 0;
      lines.clear();
      bounds = vec4( 0.0f );
    }

  }

}