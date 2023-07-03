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

}