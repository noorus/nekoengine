#include "pch.h"
#include "components.h"

namespace neko {

  namespace c {

    // +Z, Y up
    const auto c_defaultCameraOrientation = math::quaternionFrom( radians( 0 ), vec3( 0.0f, 1.0f, 0.0f ) );
    // +Z, Y up
    const auto c_defaultNodeOrientation = math::quaternionFrom( radians( 0 ), vec3( 0.0f, 1.0f, 0.0f ) );

    manager::manager( vec2 viewportResolution )
    {
      registry_.on_construct<transform>().connect<&registry::emplace_or_replace<dirty_transform>>();
      registry_.on_update<transform>().connect<&registry::emplace_or_replace<dirty_transform>>();

      camsys_ = make_unique<camera_system>( this, viewportResolution );
      txtsys_ = make_unique<text_system>( this );
      primsys_ = make_unique<primitive_system>( this );
      sprsys_ = make_unique<sprite_system>( this );
      ptbsys_ = make_unique<worldplanes_system>( this );

      root_ = registry_.create();
      registry_.emplace<node>( root_, "root" );
      registry_.emplace<transform>( root_ );

      //auto txt = createText( "pooooop" );
      //auto plane = createPlane( "lolplane" );
      //auto spr = createSprite( "lolsprite" );
      auto e = createPaintable( "lolpaint" );
      registry_.emplace<hittestable>( e );
    }

    entity manager::createNode( entity parent, string_view name )
    {
      auto e = registry_.create();
      auto& en = registry_.emplace<node>( e, name );
      en.parent = parent;
      auto& pn = registry_.get<node>( parent );
      if ( !pn.children )
      {
        pn.children = 1;
        pn.first = e;
      }
      else
      {
        pn.children++;
        auto last = pn.first;
        while ( true )
        {
          const auto& lnode = registry_.get<node>( last );
          if ( lnode.next == null )
            break;
          last = lnode.next;
        }
        registry_.get<node>( last ).next = e;
        en.prev = last;
      }
      return e;
    }

    entity manager::createRenderable( entity parent, string_view name )
    {
      auto e = createNode( root_, name );
      auto& sn = registry_.emplace<transform>( e );
      sn.rotate = c_defaultNodeOrientation;
      registry_.emplace<renderable>( e );
      return e;
    }

    entity manager::createNode( string_view name )
    {
      return createNode( root_, name );
    }

    void manager::destroyNode( entity e )
    {
      registry_.destroy( e );
    }

    entity manager::createCamera( string_view name )
    {
      auto e = createNode( root_, name );
      auto& tn = registry_.emplace<transform>( e );
      auto id = quat::identity();
      tn.rotate = c_defaultCameraOrientation;
      auto& c = registry_.emplace<camera>( e );
      c.up = ig::valueForNormalIndex( c.up_sel );
      return e;
    }

    entity manager::createText( string_view name )
    {
      auto e = createNode( root_, name );
      registry_.emplace<transform>( e );
      auto& t = registry_.emplace<text>( e );
      t.fontName = "demo_font";
      t.size = 12.0f;
      t.content = "abcdefghijklmnopqrstuvwxyz";
      t.int_ud_.str = &t.content;
      return e;
    }

    entity manager::createPlane( string_view name )
    {
      auto e = createRenderable( root_, name );
      auto& p = registry_.emplace<primitive>( e );
      p.type = primitive::PrimitiveType::Plane;
      p.values.plane.dimensions = { 10.0f, 10.0f };
      p.values.plane.segments = { 1, 1 };
      p.values.plane.normal_sel = ig::PredefNormal_PlusZ;
      p.values.plane.normal = ig::valueForNormalIndex( p.values.plane.normal_sel );
      return e;
    }

    entity manager::createSprite( string_view name )
    {
      auto e = createRenderable( root_, name );
      auto& s = registry_.emplace<sprite>( e );
      s.matName = "mushroom_idle-left";
      return e;
    }

    entity manager::createPaintable( string_view name )
    {
      auto e = createRenderable( root_, name );
      auto& p = registry_.emplace<worldplane>( e );
      p.dimensions = vec2i( 256, 256 );
      return e;
    }

    void manager::imguiSceneGraphRecurse( entity e, entity& clicked )
    {
      const auto& en = nd( e );

      ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
      auto leaf = ( !en.children || en.first == null );
      if ( leaf )
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
      if ( imguiSelectedNodes_.find( e ) != imguiSelectedNodes_.end() )
        flags |= ImGuiTreeNodeFlags_Selected;
      auto open = ImGui::TreeNodeEx( (void*)(intptr_t)e, flags, en.name.c_str() );
      if ( ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() )
        clicked = e;
      if ( open )
      {
        auto sub = en.first;
        while ( sub != null )
        {
          imguiSceneGraphRecurse( sub, clicked );
          sub = nd( sub ).next;
        }
        if ( !leaf )
          ImGui::TreePop();
      }
    }

    void manager::imguiNodeSelectorRecurse( entity e, entity& selected )
    {
      const auto& en = nd( e );

      auto is_selected = ( selected == e );
      if ( ImGui::Selectable( en.name.c_str(), &is_selected ) )
        selected = e;
      if ( is_selected )
        ImGui::SetItemDefaultFocus();

      {
        auto sub = en.first;
        while ( sub != null )
        {
          imguiNodeSelectorRecurse( sub, selected );
          sub = nd( sub ).next;
        }
      }
    }

    void manager::imguiNodeSelector( const char* title, entity& val )
    {
      ImGuiComboFlags cflags = ImGuiComboFlags_None;

      auto prevSelected = val;

      if ( ImGui::BeginCombo( title, val != c::null ? nd( val ).name.c_str() : "[null]", cflags ) )
      {
        bool selected = ( val == c::null ? true : false );
        if ( ImGui::Selectable( "[null]", &selected ) )
          val = c::null;
        imguiNodeSelectorRecurse( root_, val );
        ImGui::EndCombo();
      }
    }

    void manager::imguiSceneGraph()
    {
      entity clicked = null;
      imguiSceneGraphRecurse( root_, clicked );
      if ( clicked != null )
      {
        if ( ImGui::GetIO().KeyCtrl )
          if ( imguiSelectedNodes_.find( clicked ) != imguiSelectedNodes_.end() )
            imguiSelectedNodes_.erase( clicked );
          else
            imguiSelectedNodes_.insert( clicked );
        else
        {
          imguiSelectedNodes_.clear();
          imguiSelectedNodes_.insert( clicked );
        }
      }
    }

    void manager::imguiNodeEditor( entity e )
    {
      if ( e == null )
        return;

      auto& node = nd( e );
      ImGui::Text( "ID: %i", static_cast<uint32_t>( e ) );
      ImGui::Text( "Name: %s", node.name.c_str() );
      ImGui::TextUnformatted( "Components" );
      
      if ( registry_.any_of<transform>( e ) )
      {
        auto& t = tn( e );
        auto ww = ImGui::GetContentRegionAvail().x * 0.5f;
        ig::ComponentChildWrapper wrap( "Transform", 120.0f + ww );
        ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 4, 4 ) );
        ig::dragVector( "translate", t.translate, 0.1f, 0.0f, 0.0f, "%.4f", ImGuiSliderFlags_None );
        ig::dragVector( "scale", t.scale, 0.01f, 0.0f, 0.0f, "%.4f", ImGuiSliderFlags_None );
        ImGui::gizmo3D( "rotation", t.rotate, std::min( ww, 150.0f ) );
        ImGui::SameLine();
        ImGui::BeginGroup();
        if ( ImGui::Button( "+X", ImVec2( 36, 0 ) ) )
          t.rotate = math::quaternionFrom( radians( 90 ), vec3( 0.0f, 1.0f, 0.0f ) );
        ImGui::SameLine();
        if ( ImGui::Button( "-X", ImVec2( 36, 0 ) ) )
          t.rotate = math::quaternionFrom( radians( 90 ), vec3( 0.0f, -1.0f, 0.0f ) );
        if ( ImGui::Button( "+Y", ImVec2( 36, 0 ) ) )
          t.rotate = math::quaternionFrom( radians( 90 ), vec3( -1.0f, 0.0f, 0.0f ) );
        ImGui::SameLine();
        if ( ImGui::Button( "-Y", ImVec2( 36, 0 ) ) )
          t.rotate = math::quaternionFrom( radians( 90 ), vec3( 1.0f, 0.0f, 0.0f ) );
        if ( ImGui::Button( "+Z", ImVec2( 36, 0 ) ) )
          t.rotate = math::quaternionFrom( radians( 0 ), vec3( 0.0f, 1.0f, 0.0f ) );
        ImGui::SameLine();
        if ( ImGui::Button( "-Z", ImVec2( 36, 0 ) ) )
          t.rotate = math::quaternionFrom( radians( 180 ), vec3( 0.0f, -1.0f, 0.0f ) );
        ImGui::EndGroup();
        ImGui::PopStyleVar();
        // a bit wasteful but hardly an issue
        markDirty( e );
      }
      if ( registry_.any_of<camera>( e ) )
      {
        camsys_->imguiCameraEditor( e );
      }
      if ( registry_.any_of<text>( e ) )
      {
        txtsys_->imguiTextEditor( e );
      }
      if ( registry_.any_of<primitive>( e ) )
      {
        primsys_->imguiPrimitiveEditor( e );
      }
      if ( registry_.any_of<sprite>( e ) )
      {
        sprsys_->imguiSpriteEditor( e );
      }
      if ( registry_.any_of<worldplane>( e ) )
      {
        ptbsys_->imguiPaintableSurfaceEditor( e );
      }
    }

    void manager::imguiSelectedNodes()
    {
      for ( auto e : imguiSelectedNodes_ )
      {
        imguiNodeEditor( e );
        ImGui::Separator();
      }
    }

    void manager::executeEditorMouseClick(
      Editor& editor, Renderer& renderer, const Ray& ray, const vec2i& mousepos, int button )
    {
      auto view = registry_.view<hittestable>();
      for ( auto e : view )
      {
        if ( registry_.any_of<worldplane>( e ) )
        {
          auto& pt = wp( e );
          if ( pt.editorMouseClickTest( editor, this, e, renderer, ray, mousepos, button ) )
            return;
        }
      }
    }

    void manager::update()
    {
      // if strange transform bugs start appearing when our hierarchy gets more complicated,
      // it's probably an issue in this sort logic
      registry_.sort<node>( [this]( const entity lhs, const entity rhs )
      {
        const auto& ln = nd( lhs );
        const auto& rn = nd( rhs );
        return ( rn.parent == lhs || ln.next == rhs
          || ( !( ln.parent == rhs || rn.next == lhs ) && ( ln.parent < rn.parent || ( ln.parent == rn.parent && &ln < &rn ) ) ) );
      } );

      registry_.view<dirty_transform>().each( [this]( const auto entity )
      {
        const auto& n = nd( entity );
        auto& t = tn( entity );
        if ( n.parent != null )
        {
          const auto& pt = tn( n.parent );
          t.derived_rotate = ( pt.derived_rotate * t.rotate );
          t.derived_scale = ( pt.derived_scale * t.scale );
          t.derived_translate = ( pt.derived_rotate * ( pt.derived_scale * t.translate ) ) + pt.derived_translate;
        }
        else
        {
          t.derived_rotate = t.rotate;
          t.derived_scale = t.scale;
          t.derived_translate = t.translate;
        }
        auto tmp = mat4( numbers::one );
        tmp = glm::translate( tmp, t.derived_translate );
        tmp = glm::scale( tmp, t.derived_scale );
        tmp *= glm::toMat4( t.derived_rotate );
        t.cached_transform = tmp;
      } );

      registry_.clear<dirty_transform>();
    }

    void manager::markDirty( entity e )
    {
      registry_.emplace_or_replace<dirty_transform>( e );
    }

  }

}