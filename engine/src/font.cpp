#include "pch.h"
#include "locator.h"
#include "font.h"
#include "neko_exception.h"
#include "console.h"
#include "lodepng.h"
#include "renderer.h"
#include "engine.h"

namespace neko {

  Font::Font( FontManagerPtr manager, IDType i, const utf8String& name ):
    LoadedResourceBase<Font>( name ), manager_( manager ), id_( i )
  {
  }

  FontFacePtr Font::loadFace( span<uint8_t> source, FaceID faceIndex, Real size )
  {
    // Make a safety copy in our own memory
    // Loading new styles on the fly later could still access this memory
    // and we won't rely on the host keeping that available and alive
    // Of course, if multiple faces are actually loaded from different blobs
    // despite belonging to the same font, this copy will only contain the
    // last loaded one - but that's pretty suspect behavior anyway, don't do it
    data_ = make_unique<Buffer>( source );

    auto ftlib = manager_->ft();

    FT_Open_Args args = { 0 };
    args.flags = FT_OPEN_MEMORY;
    args.memory_base = data_->data();
    args.memory_size = (FT_Long)data_->length();

    auto fc = make_shared<FontFace>( ptr(), ftlib, &args, faceIndex, size );
    faces_[faceIndex] = fc;

    loaded_ = true;

    return move( fc );
  }

  void Font::unload()
  {
    faces_.clear();
    data_.reset();
    loaded_ = false;
  }

  FontLoadState Font::loaded() const
  {
    return ( loaded_ ? FontLoad_Loaded : FontLoad_NotReady );
  }

  Font::~Font()
  {
    unload();
  }

  void Font::update( Renderer* renderer )
  {
    if ( !loaded() || faces_.empty() )
      return;
    for ( auto& face : faces_ )
    {
      if ( !face.second || face.second->styles().empty() )
        continue;
      for ( auto& pr : face.second->styles() )
      {
        if ( pr.second->dirty() || !pr.second->material_ )
        {
          char tmp[64];
          sprintf_s( tmp, 64, "font/%I64i/%016I64x", id_, pr.first );
          pr.second->material_ =
            renderer->createTextureWithData( tmp, pr.second->atlas().dimensions().x, pr.second->atlas().dimensions().y,
              PixFmtColorR8, pr.second->atlas().data(), Texture::ClampBorder, Texture::Nearest );
          pr.second->markClean();
        }
      }
    }
  }

}