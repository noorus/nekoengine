#include "pch.h"
#include "components.h"

namespace neko {

  namespace c {

    camera_system::camera_system( manager* m ): mgr_( m )
    {
      mgr_->reg().on_construct<camera>().connect<&camera_system::updateCameraList>( this );
      mgr_->reg().on_update<camera>().connect<&camera_system::updateCameraList>( this );
      mgr_->reg().on_destroy<camera>().connect<&camera_system::updateCameraList>( this );
    }

    camera_system::~camera_system()
    {
      mgr_->reg().on_construct<camera>().disconnect<&camera_system::updateCameraList>( this );
      mgr_->reg().on_update<camera>().disconnect<&camera_system::updateCameraList>( this );
      mgr_->reg().on_destroy<camera>().disconnect<&camera_system::updateCameraList>( this );
    }

    void camera_system::imguiCameraSelector()
    {
      ImGuiComboFlags cflags = ImGuiComboFlags_None;
      const c::CameraData* current = ( selectedCamera_ == c::null || cameras_.find( selectedCamera_ ) == cameras_.end() )
                                     ? nullptr
                                     : &cameras_.at( selectedCamera_ );
      if ( ImGui::BeginCombo( "Camera", current ? current->name.data() : "none", cflags ) )
      {
        bool selected = ( current ? true : false );
        if ( ImGui::Selectable( "none", &selected ) )
          selectedCamera_ = c::null;
        for ( const auto& [key, value] : cameras_ )
        {
          selected = ( current ? ( current->ent == key ) : false );
          if ( ImGui::Selectable( value.name.data(), &selected ) )
            selectedCamera_ = key;
          if ( selected )
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
    }

    void camera_system::imguiCameraEditor( entity e )
    {
      auto& c = mgr_->reg().get<camera>( e );
      ComponentImguiWrap wrap( "Camera", 200.0f );
      ImGui::DragFloat( "FOV", &c.fovy, 0.1f, 0.1f, 200.0f, "%.4f", ImGuiSliderFlags_None );
      ImGui::DragFloat( "Near clip", &c.nearDist, 0.1f, 0.1f, 100.0f, "%.4f", ImGuiSliderFlags_Logarithmic );
      ImGui::DragFloat( "Far clip", &c.farDist, 0.1f, 0.1f, 500.0f, "%.4f", ImGuiSliderFlags_Logarithmic );
      ImGui::DragFloat( "Exposure", &c.exposure, 0.1f, 0.0f, 100.0f, "%.4f", ImGuiSliderFlags_Logarithmic );
    }

    void camera_system::updateCameraList( registry& r, entity e )
    {
      cameras_.clear();
      auto view = mgr_->reg().view<camera>();
      for ( auto entity : view )
      {
        const auto& n = mgr_->nd( entity );
        CameraData d;
        d.ent = entity;
        d.name = n.name;
        cameras_[entity] = move( d );
      }
    }

  }

}