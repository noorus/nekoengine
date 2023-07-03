#include "pch.h"
#include "components.h"
#include "camera.h"

namespace neko {

  namespace c {

    camera_system::camera_system( manager* m, vec2 resolution ): mgr_( m )
    {
      setResolution( resolution );

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

    void camera_system::setResolution( vec2 res )
    {
      resolution_ = res;
    }

    void camera_system::update()
    {
      if ( !valid( selectedCamera_ ) )
        return;
      cameras_.at( selectedCamera_ ).instance->update( *mgr_, 0.0f, 0.0f );
    }

    const shared_ptr<BasicGameCamera> camera_system::getActive() const
    {
      if ( !valid( selectedCamera_ ) )
        return {};
      return cameras_.at( selectedCamera_ ).instance;
    }

    void camera_system::setActive( shared_ptr<BasicGameCamera> cam )
    {
      for ( auto& [key, value] : cameras_ )
        if ( value.instance == cam )
        {
          selectedCamera_ = key;
          return;
        }
      NEKO_EXCEPT( "Camera doesn't exist in system map" );
    }

    void camera_system::setActive( entity e )
    {
      assert( valid( e ) );
      selectedCamera_ = e;
    }

    const CameraData* camera_system::getActiveData() const
    {
      if ( !valid( selectedCamera_ ) )
        return nullptr;
      return &cameras_.at( selectedCamera_ );
    }

    void camera_system::imguiCameraSelector()
    {
      ImGuiComboFlags cflags = ImGuiComboFlags_None;
      const c::CameraData* current = ( selectedCamera_ == c::null || cameras_.find( selectedCamera_ ) == cameras_.end() )
                                     ? nullptr
                                     : &cameras_.at( selectedCamera_ );

      auto prevSelected = selectedCamera_;

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
        if ( selectedCamera_ != prevSelected && valid( selectedCamera_ ) )
        {
          mgr_->markDirty( selectedCamera_ );
          cameras_.at( selectedCamera_ ).instance->setViewport( resolution_ );
        }
        ImGui::EndCombo();
      }
    }

    const char* c_projectionNames[camera::CameraProjection::MAX_CameraProjection] = { "perspective", "orthographic" };
    const char* c_trackingNames[camera::CameraTracking::MAX_CameraTracking] = { "free", "node" };

    void camera_system::imguiCameraEditor( entity e )
    {
      auto& c = mgr_->reg().get<camera>( e );
      ig::ComponentChildWrapper wrap( "Camera", 240.0f );

      {
        auto selected = static_cast<int>( c.projection );
        if ( ImGui::BeginCombo( "projection", c_projectionNames[selected], ImGuiComboFlags_None ) )
        {
          for ( int i = 0; i < camera::CameraProjection::MAX_CameraProjection; ++i )
          {
            bool dummy = ( selected == i );
            if ( ImGui::Selectable( c_projectionNames[i], &dummy ) )
              selected = i;
            if ( dummy )
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
        c.projection = static_cast<camera::CameraProjection>( selected );
      }

      if ( c.projection == camera::CameraProjection::Perspective )
      {
        ImGui::DragFloat( "fov", &c.perspective_fovy, 0.1f, 0.1f, 200.0f, "%.4f", ImGuiSliderFlags_None );
      }
      else if ( c.projection == camera::CameraProjection::Orthographic )
      {
        ImGui::DragFloat( "radius", &c.orthographic_radius, 0.1f, 0.1f, 2048.0f, "%.4f", ImGuiSliderFlags_None );
      }

      {
        auto selected = static_cast<int>( c.tracking );
        if ( ImGui::BeginCombo( "tracking", c_trackingNames[selected], ImGuiComboFlags_None ) )
        {
          for ( int i = 0; i < camera::CameraProjection::MAX_CameraProjection; ++i )
          {
            bool dummy = ( selected == i );
            if ( ImGui::Selectable( c_trackingNames[i], &dummy ) )
              selected = i;
            if ( dummy )
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
        c.tracking = static_cast<camera::CameraTracking>( selected );
      }

      if ( c.tracking == camera::CameraTracking::Free )
      {
        ImGui::DragFloat( "free dist", &c.free_dist, 0.1f, 0.1f, 1000.0f, "%.4f", ImGuiSliderFlags_None );
      }
      else if ( c.tracking == camera::CameraTracking::Node )
      {
        mgr_->imguiNodeSelector( "target note", c.node_target );
      }

      ig::normalSelector( "up", c.up_sel, c.up );

      ImGui::DragFloat( "nearclip", &c.nearDist, 0.1f, 0.1f, 100.0f, "%.4f", ImGuiSliderFlags_Logarithmic );
      ImGui::DragFloat( "farclip", &c.farDist, 0.1f, 0.1f, 500.0f, "%.4f", ImGuiSliderFlags_Logarithmic );
      ImGui::DragFloat( "exposure", &c.exposure, 0.1f, 0.0f, 100.0f, "%.4f", ImGuiSliderFlags_Logarithmic );
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
        d.instance = make_shared<BasicGameCamera>( resolution_, *mgr_, d.ent );
        cameras_[entity] = move( d );
      }
    }

  }

}