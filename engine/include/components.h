#pragma once
#include <entt.hpp>
#include "neko_types.h"

#undef near
#undef far

namespace neko {

  class BasicGameCamera;

  namespace c {

    using entt::registry;
    using entt::entity;
    using entt::null;
    using entt::get;
    using entt::tombstone;

    class manager;

    template <typename T>
    inline bool igDragVector( const char* label, const T& v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
      const char* format = "%.3f", ImGuiSliderFlags flags = 0 )
    {
      if constexpr ( std::is_same_v<T::value_type, float> )
        return ImGui::DragScalarN( label, ImGuiDataType_Float, const_cast<void*>( reinterpret_cast<const void*>( &v[0] ) ),
          static_cast<int>( v.length() ), v_speed, &v_min, &v_max, format, flags );
      if constexpr ( std::is_same_v<T::value_type, double> )
        return ImGui::DragScalarN( label, ImGuiDataType_Double, const_cast<void*>( reinterpret_cast<const void*>( &v[0] ) ),
          static_cast<int>( v.length() ), v_speed, &v_min, &v_max, format, flags );
    }

    class ComponentImguiWrap {
    public:
      ComponentImguiWrap( const char* title, float height )
      {
        ImGui::BeginChild( title, ImVec2( 0, height ), true, ImGuiWindowFlags_MenuBar );
        if ( ImGui::BeginMenuBar() )
        {
          if ( ImGui::BeginMenu( title ) )
            ImGui::EndMenu();
          ImGui::EndMenuBar();
        }
      }
      ~ComponentImguiWrap() { ImGui::EndChild(); }
    };

    struct node
    {
      utf8String name; //!< Node name
      size_t children { 0 }; //!< Number of children
      entity first { null }; //!< First child
      entity prev { null }; //!< Previous sibling
      entity next { null }; //!< Next sibling
      entity parent { null }; //!< Parent
      node( string_view name_ ): name( name_ ) {}
    };

    struct dirty_transform
    {
    };

    struct transform
    {
      vec3 scale { numbers::one, numbers::one, numbers::one };
      quat rotate = quatIdentity;
      vec3 translate { numbers::zero, numbers::zero, numbers::zero };
      vec3 derived_scale { numbers::one, numbers::one, numbers::one };
      quat derived_rotate = quatIdentity;
      vec3 derived_translate { numbers::zero, numbers::zero, numbers::zero };
      mat4 cached_transform { numbers::one };
      inline const mat4 model() const { return cached_transform; } //!< Model matrix e.g. the cached transform matrix
    };

    struct camera
    {
      Real fovy = 60.0f;
      Real nearDist = 0.1f;
      Real farDist = 100.0f;
      Real exposure = 1.0f;
    };

    struct CameraData
    {
      entity ent;
      utf8String name;
      shared_ptr<BasicGameCamera> instance;
    };

    using CameraDataMap = map<entity, CameraData>;

    class camera_system {
    protected:
      manager* mgr_ = nullptr;
      CameraDataMap cameras_;
      Entity selectedCamera_ = c::null;
      vec2 resolution_ { 0.0f, 0.0f };
      inline bool valid( entity cam ) const
      {
        return ( cam == null || cam == tombstone || cameras_.find( selectedCamera_ ) == cameras_.end() ) ? false : true;
      }
      void updateCameraList( registry& r, entity e );
    public:
      camera_system( manager* m, vec2 resolution );
      void update();
      const shared_ptr<BasicGameCamera> getActive() const;
      const CameraData* getActiveData() const;
      ~camera_system();
      inline const CameraDataMap& cameras() const { return cameras_; }
      void setResolution( vec2 res );
      void imguiCameraSelector();
      void imguiCameraEditor( entity e );
    };

    class manager {
    protected:
      registry registry_;
      entity root_ { null };
      set<entity> imguiSelectedNodes_;
      unique_ptr<camera_system> camsys_;
      void imguiSceneGraphRecurse( entity e, entity& clicked );
      void imguiNodeEditor( entity e );
    public:
      manager( vec2 viewportResolution );
      inline registry& reg() { return registry_; }
      inline node& nd( entity e ) { return registry_.get<node>( e ); } //!< Get node by entity
      inline entity en( const node& n ) const { return entt::to_entity( registry_, n ); } //!< Get entity by node
      inline transform& tn( entity e ) { return registry_.get<transform>( e ); } //!< Get transform by entity
      inline camera& cam( entity e ) { return registry_.get<camera>( e ); } //!< Get camera by entity
      entity createNode( entity parent, string_view name );
      entity createNode( string_view name );
      entity createCamera( string_view name );
      void update();
      void markDirty( entity e );
      void imguiSceneGraph();
      void imguiSelectedNodes();
      inline camera_system& cams() const { return *camsys_; }
    };

  }

  using SManager = c::manager;

  using SManagerPtr = shared_ptr<SManager>;

}