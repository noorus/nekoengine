#include "pch.h"
#include "components.h"
#include "camera.h"
#include "utilities.h"
#include "font.h"

namespace neko::ig {

  // clang-format off

  const char* c_predefNormalNames[] = { "+Y", "-Y", "+X", "-X", "+Z", "-Z" };
  const vec3 c_predefNormalValues[] = {
    {  0.0f,  1.0f,  0.0f },
    {  0.0f, -1.0f,  0.0f },
    {  1.0f,  0.0f,  0.0f },
    { -1.0f,  0.0f,  0.0f },
    {  0.0f,  0.0f,  1.0f },
    {  0.0f,  0.0f, -1.0f }
  };
  const quat c_predefQuatValues[] = {
    math::quaternionFrom( 0, { 1.0f, 0.0f, 0.0f })
  };

  // clang-format on

  vec3 valueForNormalIndex( int idx )
  {
    assert( idx >= 0 && idx < MAX_PredefNormal );
    return c_predefNormalValues[idx];
  }

  bool normalSelector( const char* label, int& in_out, vec3& value_out )
  {
    ImGuiComboFlags cflags = ImGuiComboFlags_None;
    auto prevSelected = in_out;

    auto selected = prevSelected;
    if ( ImGui::BeginCombo( label, c_predefNormalNames[prevSelected], cflags ) )
    {
      for ( int i = 0; i < MAX_PredefNormal; ++i )
      {
        bool s = ( i == prevSelected );
        if ( ImGui::Selectable( c_predefNormalNames[i], &s ) )
          selected = i;
        if ( selected )
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
      auto changed = ( selected != prevSelected );
      if ( changed )
        in_out = selected;
      value_out = valueForNormalIndex( in_out ); 
      return changed;
    }

    return false;
  }

  bool orientationEditor( const char* label, quat& in_out )
  {
    return false;
  }

  int imguiInputText_Callback( ImGuiInputTextCallbackData* data )
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

  bool imguiInputText( const char* label, utf8String* str, bool multiline, ImGuiInputTextCallback callback, void* user_data )
  {
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackResize;

    c::TextInputUserData ud;
    ud.str = str;
    ud.chaincb = callback;
    ud.chaincb_ud = user_data;
    if ( !multiline )
      return ImGui::InputText( label, ud.str->data(), str->capacity() + 1, flags, imguiInputText_Callback, &ud );
    else
      return ImGui::InputTextMultiline( label, ud.str->data(), str->capacity() + 1,
        ImVec2( -FLT_MIN, ImGui::GetTextLineHeight() * 4 ), flags, imguiInputText_Callback, &ud );
  }

  bool imguiPixelScaleSelector( PixelScale& scale )
  {
    auto scaleName = ( scale >= 0 && scale < MAX_PixelScale ? c_pixelScaleNames[scale] : "Unknown" );
    return ImGui::SliderInt( "pixel scale", reinterpret_cast<int*>( &scale ), PixelScale_1,
      MAX_PixelScale - 1, scaleName, ImGuiSliderFlags_NoInput );
  }

}