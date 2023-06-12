#include "pch.h"
#include "components.h"
#include "camera.h"
#include "utilities.h"
#include "font.h"

namespace neko {

  static int InputTextCallback( ImGuiInputTextCallbackData* data )
  {
    auto ud = static_cast<c::TextInputUserData*>( data->UserData );
    if ( data->EventFlag == ImGuiInputTextFlags_CallbackResize )
    {
      assert( data->Buf == ud->str->data() );
      ud->str->resize( data->BufTextLen );
      data->Buf = ud->str->data();
    }
    else if ( ud->chaincb )
    {
      data->UserData = ud->chaincb_ud;
      return ud->chaincb( data );
    }
    return 0;
  }

  bool imguiInputText(
    const char* label, utf8String* str, bool multiline, ImGuiInputTextCallback callback, void* user_data )
  {
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;

    c::TextInputUserData ud;
    ud.str = str;
    ud.chaincb = callback;
    ud.chaincb_ud = user_data;
    if ( !multiline )
      return ImGui::InputText( label, ud.str->data(), str->capacity() + 1, flags, InputTextCallback, &ud );
    else
      return ImGui::InputTextMultiline( label, ud.str->data(), str->capacity() + 1,
        ImVec2( -FLT_MIN, ImGui::GetTextLineHeight() * 4 ), flags, InputTextCallback, &ud );
  } 

  namespace c {

    text_system::text_system( manager* m ): mgr_( m )
    {
      mgr_->reg().on_construct<text>().connect<&text_system::addText>( this );
      mgr_->reg().on_update<text>().connect<&text_system::updateText>( this );
      mgr_->reg().on_destroy<text>().connect<&text_system::removeText>( this );
    }

    text_system::~text_system()
    {
      mgr_->reg().on_construct<text>().disconnect<&text_system::addText>( this );
      mgr_->reg().on_update<text>().disconnect<&text_system::updateText>( this );
      mgr_->reg().on_destroy<text>().disconnect<&text_system::removeText>( this );
    }

    void text_system::addText( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_text>( e );
    }

    void text_system::updateText( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_text>( e );
    }

    void text_system::removeText( registry& r, entity e )
    {
      if ( texts_.find( e ) != texts_.end() )
      {
        if ( texts_[e].instance )
          texts_[e].instance->markDead();
        texts_.erase( e );
      }
    }

    void text_system::draw( Renderer& renderer )
    {
      for ( auto& [eid, data] : texts_ )
      {
        if ( !data.instance || data.instance->dead() )
          continue;
        const auto& tfm = mgr_->tn( eid );
        data.instance->draw( renderer, tfm.model() );
      }
    }

    void text_system::update( FontManager& fntmgr )
    {
      mgr_->reg().view<dirty_text>().each( [this]( const auto entity )
      {
        const auto& t = mgr_->tt( entity );
        if ( texts_.find( entity ) == texts_.end() )
          texts_[entity] = TextData( entity );
        auto& te = texts_[entity];
        te.dirty = true;
      } );

      mgr_->reg().clear<dirty_text>();

      for ( auto& [eid, data] : texts_ )
      {
        if ( !data.dirty || ( data.instance && data.instance->dead() ) )
          continue;

        const auto& t = mgr_->tt( eid );

        auto fnt = fntmgr.font( t.fontName );
        if ( !fnt )
        {
          data.instance.reset();
          continue;
        }

        Real bestDelta = 0.0f;
        FontFacePtr bestFace;
        for ( const auto& [fid, face] : fnt->faces() )
        {
          auto delta = math::abs( t.size - face->size() );
          if ( !bestFace || delta < bestDelta )
          {
            bestFace = face;
            bestDelta = delta;
          }
        }

        if ( !bestFace )
          continue;

        auto style = makeStyleID( 0, bestFace->size(), FontRender_Normal, 0.0f );

        if ( !data.instance )
          data.instance = fntmgr.createText( bestFace, style );
        else
          data.instance->face( bestFace, style );

        data.instance->content( utils::uniFrom( t.content ) );

        data.dirty = false;
      }
    }

    void text_system::imguiTextEditor( entity e )
    {
      ComponentImguiWrap wrap( "Text", 200.0f );

      auto& c = mgr_->reg().get<text>( e );

      bool changed = false;
      changed |= imguiInputText( "fontname", &c.fontName, false, nullptr, &c.int_ud_ );
      changed |= ImGui::SliderFloat( "size", &c.size, 0.0f, 100.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp );
      changed |= imguiInputText( "content", &c.content, true, nullptr, &c.int_ud_ );

      if ( changed )
        mgr_->reg().emplace_or_replace<dirty_text>( e );
    }

  }

}