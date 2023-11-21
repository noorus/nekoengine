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

  class Editor;
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

  // clang-format on

  enum PaintBrushType : int
  {
    Brush_Classic = 0,
    Brush_Circle,
    Brush_Rhombus,
    MAX_Brush
  };

  struct PaintBrushToolOptions
  {
    PaintBrushType paintBrushType = Brush_Classic;
    int paintBrushSize = 8;
    float paintBrushOpacity = 0.3f;
    float paintBrushSoftness = 0.2f;
    bool paintBrushNoiseLockSeed = true;
    float paintBrushNoiseAmount = 0.8f;
    float paintBrushNoiseSeed = 125.0f;
    int paintBrushNoiseDetail = 115;
    float paintBrushNoiseOffset = 0.05f;
    bool paintBrushNoiseEdgesOnly = true;
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

    extern bool imguiIconButton( const char* label, const char* tooltip, bool& selected );

    struct SelectableButtonGroup
    {
      struct SelectableButton
      {
        int value = 0;
        utf8String text;
        utf8String tooltip;
        bool selected = false;
      };
      vector<SelectableButton> buttons;
      inline void addButton( int value, const utf8String& text, const utf8String& tooltip )
      {
        buttons.emplace_back( value, text, tooltip, false );
      }
      inline int selection() const
      {
        for ( const auto& b : buttons )
          if ( b.selected )
            return b.value;
        return 0;
      }
      inline void select( int value )
      {
        for ( auto& b : buttons )
          b.selected = ( b.value == value );
      }
      inline void draw()
      {
        for ( int i = 0; i < buttons.size(); ++i )
        {
          SelectableButton& b = buttons[i];
          if ( imguiIconButton( b.text.c_str(), b.tooltip.c_str(), b.selected ) && b.selected )
          {
            for ( int j = 0; j < buttons.size(); ++j )
              if ( i != j )
                buttons[j].selected = false;
          }
        }
      }
    };

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
      unique_ptr<Indexed3DVertexBuffer> mesh;
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
      void draw( Shaders& shaders, const Material& mat );
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

    // worldplanes

    struct worldplane;

    class WorldplaneLayer {
    protected:
      bool selected_ = false;
    public:
      inline bool selected() const { return selected_; }
      inline void selected( bool sel ) { selected_ = sel; }
      virtual void recreate( Renderer& renderer, vec2i dimensions ) = 0;
      virtual void draw(
        Renderer& renderer, const Camera& cam, const Indexed3DVertexBuffer& mesh, const mat4& model, Real pixelScale ) const = 0;
      virtual void applyBrush( Renderer& renderer, const vec2& pos, PaintBrushToolOptions& opts, bool erase ) = 0;
      virtual MaterialPtr icon() const = 0;
      virtual const char* caption() const = 0;
      virtual void setMaterial( MaterialPtr newmat ) = 0;
    };

    class WorldplaneTexturePaintLayer : public WorldplaneLayer {
    protected:
      vec2i dimensions_;
      utf8String materialName_;
      MaterialPtr material_;
      PaintableTexturePtr blendMap_;
    public:
      WorldplaneTexturePaintLayer( const utf8String& textureName );
      void recreate( Renderer& renderer, vec2i dimensions ) override;
      void draw( Renderer& renderer, const Camera& cam, const Indexed3DVertexBuffer& mesh, const mat4& model,
        Real pixelScale ) const override;
      void applyBrush( Renderer& renderer, const vec2& pos, PaintBrushToolOptions& opts, bool erase ) override;
      MaterialPtr icon() const override;
      const char* caption() const override;
      void setMaterial( MaterialPtr newmat ) override;
    };

    using WorldplaneLayerPtr = unique_ptr<WorldplaneLayer>;

    struct worldplane
    {
      PixelScale pixelScaleBase = PixelScale_32;
      unique_ptr<Indexed3DVertexBuffer> mesh;
      vector<WorldplaneLayerPtr> layers;
      unique_ptr<LineRenderBuffer<8>> viz_;
      vec2i dimensions { 0, 0 };
      vec2 worldDimensions { 0.0f, 0.0f };
      int normal_sel = ig::PredefNormal_PlusZ;
      vec3 normal { 0.0f, 0.0f, 1.0f };
      vec2i lastPaintPos = { 0, 0 };
      int paintSelectedLayerIndex = 0;
      worldplane();
      bool editorMouseClickTest( Editor& editor, manager* m, entity e, Renderer& renderer, const Ray& ray, const vec2i& mousepos, int button );
    };

    struct dirty_worldplane
    {
    };

    class worldplanes_system {
    protected:
      manager* mgr_ = nullptr;
      void addSurface( registry& r, entity e );
      void updateSurface( registry& r, entity e );
      void removeSurface( registry& r, entity e );
    public:
      worldplanes_system( manager* m );
      void update( Renderer& renderer );
      void draw( Renderer& renderer, const Camera& cam );
      ~worldplanes_system();
      void imguiWorldplaneEditor( Editor& editor, entity e, WorldplaneLayer** selection );
    };

    // hittestable

    struct hittestable
    {
    };

    // overall manager

    class manager {
    protected:
      registry registry_;
      entity root_ { null };
      set<entity> imguiSelectedNodes_;
      unique_ptr<camera_system> camsys_;
      unique_ptr<text_system> txtsys_;
      unique_ptr<primitive_system> primsys_;
      unique_ptr<sprite_system> sprsys_;
      unique_ptr<worldplanes_system> ptbsys_;
      WorldplaneLayer* selectedLayer_ = nullptr;
      void imguiSceneGraphRecurse( entity e, entity& clicked );
      void imguiNodeSelectorRecurse( entity e, entity& selected );
      void imguiNodeEditor( Editor& editor, entity e );
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
      inline worldplane& wp( entity e ) { return registry_.get<worldplane>( e ); }
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
      inline optional<entity> find( string_view name ) const
      {
        for ( const auto& e : registry_.view<node>() )
        {
          if ( e == null || e == tombstone )
            continue;
          if ( registry_.get<node>( e ).name == name )
            return e;
        }
        return {};
      }
      inline bool valid( entity e )
      {
        if ( e == null || e == tombstone )
          return false;
        return registry_.any_of<node>( e );
      }
      void executeEditorMouseClick( Editor& editor, Renderer& renderer, const Ray& ray, const vec2i& mousepos, int button );
      entity createNode( entity parent, string_view name );
      entity createRenderable( entity parent, string_view name );
      entity createNode( string_view name );
      void destroyNode( entity e );
      entity createCamera( string_view name );
      entity createText( string_view name );
      entity createPlane( string_view name );
      entity createSprite( string_view name );
      entity createPaintable( string_view name );
      inline entity& root() { return root_; }
      void update();
      void markDirty( entity e );
      void imguiSceneGraph();
      void imguiSelectedNodes( Editor& editor );
      void imguiNodeSelector( const char* title, entity& selected );
      inline camera_system& cams() const { return *camsys_; }
      inline text_system& texts() const { return *txtsys_; }
      inline primitive_system& primitives() const { return *primsys_; }
      inline sprite_system& sprites() const { return *sprsys_; }
      inline worldplanes_system& paintables() const { return *ptbsys_; }
    };

  }

}