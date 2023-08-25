#pragma once
#include <entt.hpp>
#include "neko_types.h"
#include "buffers.h"
#include "spriteanim.h"
#include "font.h"
#include "imgui.h"

#undef near
#undef far

namespace neko {

  class BasicGameCamera;

  // clang-format off
  
  enum PixelScale: int
  {
    PixelScale_1 = 0,
    PixelScale_8,
    PixelScale_10,
    PixelScale_16,
    PixelScale_20,
    PixelScale_24,
    PixelScale_30,
    PixelScale_32,
    PixelScale_40,
    PixelScale_48,
    PixelScale_50,
    PixelScale_64,
    MAX_PixelScale
  };

  constexpr const Real c_pixelScaleValues[MAX_PixelScale] = {
    ( 1.0f / 1.0f ),
    ( 1.0f / 8.0f ),
    ( 1.0f / 10.0f ),
    ( 1.0f / 16.0f ),
    ( 1.0f / 20.0f ),
    ( 1.0f / 24.0f ),
    ( 1.0f / 30.0f ),
    ( 1.0f / 32.0f ),
    ( 1.0f / 40.0f ),
    ( 1.0f / 48.0f ),
    ( 1.0f / 50.0f ),
    ( 1.0f / 64.0f )
  };

  constexpr const char* c_pixelScaleNames[MAX_PixelScale] = {
    "1 = 1px",
    "1 = 8px",
    "1 = 10px",
    "1 = 16px",
    "1 = 20px",
    "1 = 24px",
    "1 = 30px",
    "1 = 32px",
    "1 = 40px",
    "1 = 48px",
    "1 = 50px",
    "1 = 64px"
  };

  namespace ig {

    enum PredefinedNormal : int
    {
      PredefNormal_PlusY,
      PredefNormal_MinusY,
      PredefNormal_PlusX,
      PredefNormal_MinusX,
      PredefNormal_PlusZ,
      PredefNormal_MinusZ,
      MAX_PredefNormal
    };

    vec3 valueForNormalIndex( int idx );
    bool normalSelector( const char* label, int& in_out, vec3& value_out );

    bool orientationEditor( const char* label, quat& in_out );

    extern int imguiInputText_Callback( ImGuiInputTextCallbackData* data );
    extern bool imguiInputText( const char* label, utf8String* str, bool multiline, ImGuiInputTextCallback callback, void* user_data );
    extern bool imguiPixelScaleSelector( PixelScale& scale );

    template <typename T>
    inline bool dragVector( const char* label, const T& v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
      const char* format = "%.3f", ImGuiSliderFlags flags = 0 )
    {
      if constexpr ( std::is_same_v<typename T::value_type, float> )
        return ImGui::DragScalarN( label, ImGuiDataType_Float, const_cast<void*>( reinterpret_cast<const void*>( &v[0] ) ),
          static_cast<int>( v.length() ), v_speed, &v_min, &v_max, format, flags );
      if constexpr ( std::is_same_v<typename T::value_type, double> )
        return ImGui::DragScalarN( label, ImGuiDataType_Double, const_cast<void*>( reinterpret_cast<const void*>( &v[0] ) ),
          static_cast<int>( v.length() ), v_speed, &v_min, &v_max, format, flags );
    }

    class ComponentChildWrapper {
    public:
      ComponentChildWrapper( const char* title, float height )
      {
        ImGui::SeparatorText( title );
        /* ImGui::BeginChild( title, ImVec2( 0, height ), true, ImGuiWindowFlags_MenuBar );
        if ( ImGui::BeginMenuBar() )
        {
          if ( ImGui::BeginMenu( title ) )
            ImGui::EndMenu();
          ImGui::EndMenuBar();
        }*/
      }
      ~ComponentChildWrapper()
      {
        //ImGui::EndChild();
      }
    };

  }

  namespace c {

    using entt::registry;
    using entt::entity;
    using entt::null;
    using entt::get;
    using entt::tombstone;

    class manager;

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
      quat rotate = quat::identity();
      vec3 translate { numbers::zero, numbers::zero, numbers::zero };
      vec3 derived_scale { numbers::one, numbers::one, numbers::one };
      quat derived_rotate = quat::identity();
      vec3 derived_translate { numbers::zero, numbers::zero, numbers::zero };
      mat4 cached_transform { numbers::one };
      inline const mat4 model() const { return cached_transform; } //!< Model matrix e.g. the cached transform matrix
    };

    struct renderable
    {
    };

    struct TextInputUserData
    {
      utf8String* str = nullptr;
      ImGuiInputTextCallback chaincb = nullptr;
      void* chaincb_ud = nullptr;
    };

    struct camera
    {
      enum CameraProjection
      {
        Perspective = 0,
        Orthographic,
        MAX_CameraProjection
      } projection = CameraProjection::Perspective;
      Real perspective_fovy = 30.0f;
      Real orthographic_radius = 16.0f;
      enum CameraTracking
      {
        Free = 0,
        Node,
        MAX_CameraTracking
      } tracking = CameraTracking::Free;
      Real free_dist = 100.0f;
      entity node_target = null;
      Real nearDist = 0.1f;
      Real farDist = 100.0f;
      Real exposure = 1.0f;
      int up_sel = ig::PredefNormal_PlusY;
      vec3 up = { 0.0f, 1.0f, 0.0f };
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
        return ( cam == null || cam == tombstone || cameras_.find( cam ) == cameras_.end() ) ? false : true;
      }
      void updateCameraList( registry& r, entity e );
    public:
      camera_system( manager* m, vec2 resolution );
      void update();
      const shared_ptr<BasicGameCamera> getActive() const;
      void setActive( shared_ptr<BasicGameCamera> cam );
      void setActive( entity e );
      const CameraData* getActiveData() const;
      ~camera_system();
      inline const CameraDataMap& cameras() const { return cameras_; }
      void setResolution( vec2 res );
      void imguiCameraSelector();
      void imguiCameraEditor( entity e );
    };

    // primitives

    struct primitive
    {
      enum class PrimitiveType
      {
        Plane = 0,
        Box,
        Sphere
      } type;
      unique_ptr<BasicIndexedVertexbuffer> mesh;
      union Values
      {
        struct Plane
        {
          vec2 dimensions;
          glm::i32vec2 segments;
          int normal_sel = ig::PredefNormal_PlusZ;
          vec3 normal;
        } plane;
        struct Box
        {
          vec3 dimensions;
          vec2u segments;
          bool inverted;
        } box;
        struct Sphere
        {
          Real diameter;
          vec2u segments;
        } sphere;
      } values;
    };

    struct dirty_primitive
    {
    };

    class primitive_system {
    protected:
      manager* mgr_ = nullptr;
      void addPrimitive( registry& r, entity e );
      void updatePrimitive( registry& r, entity e );
      void removePrimitive( registry& r, entity e );
    public:
      primitive_system( manager* m );
      void update();
      void draw( shaders::Shaders& shaders, const Material& mat );
      ~primitive_system();
      void imguiPrimitiveEditor( entity e );
    };

    // text

    struct dirty_text
    {
    };

    struct text
    {
      PixelScale pixelScaleBase = PixelScale_32;
      utf8String content;
      utf8String fontName;
      Real size = 10.0f;
      vec2 offset = { 0.0f, 0.0f };
      int alignHorizontal = 0;
      int alignVertical = 0;
      TextInputUserData int_ud_;
    };

    struct TextData
    {
      entity ent = null;
      TextPtr instance {};
      bool dirty = false;
      TextData(): ent( null ), dirty( false ) {}
      TextData( entity e ): ent( e ) {}
    };

    using TextDataMap = map<entity, TextData>;

    class text_system {
    protected:
      manager* mgr_ = nullptr;
      TextDataMap texts_;
      void addText( registry& r, entity e );
      void updateText( registry& r, entity e );
      void removeText( registry& r, entity e );
    public:
      text_system( manager* m );
      void update( FontManager& fntmgr );
      void draw( Renderer& renderer );
      ~text_system();
      inline const TextDataMap& texts() const { return texts_; }
      void imguiTextEditor( entity e );
    };

    // sprites

    struct sprite
    {
      PixelScale pixelScaleBase = PixelScale_32;
      Real size = 1.0f;
      unique_ptr<SpriteVertexbuffer> mesh;
      bool billboard = true;
      int frame = 0;
      MaterialPtr material;
      vec2 dimensions { 0.0f, 0.0f };
      utf8String matName;
      TextInputUserData int_ud_;
    };

    struct dirty_sprite
    {
    };

    class sprite_system {
    protected:
      manager* mgr_ = nullptr;
      void addSprite( registry& r, entity e );
      void updateSprite( registry& r, entity e );
      void removeSprite( registry& r, entity e );
    public:
      sprite_system( manager* m );
      void update( MaterialManager& mats );
      void draw( Renderer& renderer, const Camera& cam );
      ~sprite_system();
      void imguiSpriteEditor( entity e );
    };

    // paintables

    struct paintable
    {
      PixelScale pixelScaleBase = PixelScale_32;
      unique_ptr<BasicIndexedVertexbuffer> mesh;
      PaintableTexturePtr texture;
      vec2i dimensions { 0, 0 };
      int normal_sel = ig::PredefNormal_PlusZ;
      vec3 normal { 0.0f, 0.0f, 1.0f };
      vec2i lastPaintPos = { 0, 0 };
      int paintBrushSize = 32;
      float paintBrushSoftness = 0.2f;
      float paintBrushOpacity = 0.4f;
      void applyPaint( Renderer& renderer, const vec2& pos );
      void mouseClickTest( manager* m, entity e, Renderer& renderer, const Ray& ray, const vec2i& mousepos, int button );
    };

    struct dirty_paintable
    {
    };

    class paintables_system {
    protected:
      manager* mgr_ = nullptr;
      void addSurface( registry& r, entity e );
      void updateSurface( registry& r, entity e );
      void removeSurface( registry& r, entity e );
    public:
      paintables_system( manager* m );
      void update( Renderer& renderer );
      void draw( Renderer& renderer, const Camera& cam );
      ~paintables_system();
      void imguiPaintableSurfaceEditor( entity e );
    };

    struct hittestable
    {
    };

    class manager {
    protected:
      registry registry_;
      entity root_ { null };
      set<entity> imguiSelectedNodes_;
      unique_ptr<camera_system> camsys_;
      unique_ptr<text_system> txtsys_;
      unique_ptr<primitive_system> primsys_;
      unique_ptr<sprite_system> sprsys_;
      unique_ptr<paintables_system> ptbsys_;
      void imguiSceneGraphRecurse( entity e, entity& clicked );
      void imguiNodeSelectorRecurse( entity e, entity& selected );
      void imguiNodeEditor( entity e );
    public:
      manager( vec2 viewportResolution );
      inline registry& reg() { return registry_; }
      inline node& nd( entity e ) { return registry_.get<node>( e ); } //!< Get node by entity
      inline entity en( const node& n ) const { return entt::to_entity( registry_, n ); } //!< Get entity by node
      inline transform& tn( entity e ) { return registry_.get<transform>( e ); } //!< Get transform by entity
      inline camera& cam( entity e ) { return registry_.get<camera>( e ); } //!< Get camera by entity
      inline text& tt( entity e ) { return registry_.get<text>( e ); } //!< Get text by entity
      inline primitive& pt( entity e ) { return registry_.get<primitive>( e ); }
      inline sprite& s( entity e ) { return registry_.get<sprite>( e ); }
      inline paintable& paintable2d( entity e ) { return registry_.get<paintable>( e ); }
      inline bool validAndTransform( entity e )
      {
        if ( e == null || e == tombstone )
          return false;
        return registry_.any_of<transform>( e );
      }
      inline const bool nodeSelected( entity e ) const
      {
        return imguiSelectedNodes_.contains( e );
      }
      void executeMouseClick( Renderer& renderer, const Ray& ray, const vec2i& mousepos, int button );
      entity createNode( entity parent, string_view name );
      entity createRenderable( entity parent, string_view name );
      entity createNode( string_view name );
      entity createCamera( string_view name );
      entity createText( string_view name );
      entity createPlane( string_view name );
      entity createSprite( string_view name );
      entity createPaintable( string_view name );
      inline entity& root() { return root_; }
      void update();
      void markDirty( entity e );
      void imguiSceneGraph();
      void imguiSelectedNodes();
      void imguiNodeSelector( const char* title, entity& selected );
      inline camera_system& cams() const { return *camsys_; }
      inline text_system& texts() const { return *txtsys_; }
      inline primitive_system& primitives() const { return *primsys_; }
      inline sprite_system& sprites() const { return *sprsys_; }
      inline paintables_system& paintables() const { return *ptbsys_; }
    };

  }

  using SManager = c::manager;

  using SManagerPtr = shared_ptr<SManager>;

}