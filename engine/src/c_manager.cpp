#include "pch.h"
#include "components.h"

namespace neko {

  namespace c {

    manager::manager( vec2 viewportResolution )
    {
      registry_.on_construct<transform>().connect<&registry::emplace_or_replace<dirty_transform>>();
      registry_.on_update<transform>().connect<&registry::emplace_or_replace<dirty_transform>>();

      camsys_ = make_unique<camera_system>( this, viewportResolution );
      txtsys_ = make_unique<text_system>( this );
      primsys_ = make_unique<primitive_system>( this );

      root_ = registry_.create();
      registry_.emplace<node>( root_, "root" );
      registry_.emplace<transform>( root_ );

      auto txt = createText( "pooooop" );
      auto plane = createPlane( "lolplane" );
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
      registry_.emplace<transform>( e );
      registry_.emplace<renderable>( e );
      return e;
    }

    entity manager::createNode( string_view name )
    {
      return createNode( root_, name );
    }

    entity manager::createCamera( string_view name )
    {
      auto e = createNode( root_, name );
      registry_.emplace<transform>( e );
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
      t.size = 18.0f;
      t.int_ud_.str = &t.content;
      return e;
    }

    entity manager::createPlane( string_view name )
    {
      auto e = createRenderable( root_, name );
      auto& p = registry_.emplace<primitive>( e );
      p.type = primitive::PrimitiveType::Plane;
      p.values.plane.dimensions = { 10.0f, 10.0f };
      p.values.plane.segments = { 10, 10 };
      p.values.plane.normal_sel = ig::PredefNormal_PlusY;
      p.values.plane.normal = ig::valueForNormalIndex( p.values.plane.normal_sel );
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

      auto node = nd( e );
      ImGui::Text( "ID: %i", static_cast<uint32_t>( e ) );
      ImGui::Text( "Name: %s", node.name.c_str() );
      ImGui::TextUnformatted( "Components" );
      
      if ( registry_.any_of<transform>( e ) )
      {
        auto& t = tn( e );
        auto ww = ImGui::GetContentRegionAvail().x * 0.5f;
        ig::ComponentChildWrapper wrap( "Transform", 120.0f + ww );
        ig::dragVector( "translate", t.translate, 0.1f, 0.0f, 0.0f, "%.4f", ImGuiSliderFlags_None );
        ig::dragVector( "scale", t.scale, 0.01f, 0.0f, 0.0f, "%.4f", ImGuiSliderFlags_None );
        // ImGui::gizmo3D( "rotation", t.rotate, ww );
        //ImGui::SameLine();
        //ImGui::Button( "+X" )
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
    }

    void manager::imguiSelectedNodes()
    {
      for ( auto e : imguiSelectedNodes_ )
      {
        imguiNodeEditor( e );
        ImGui::Separator();
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
        tmp = glm::scale( tmp, t.derived_scale );
        tmp *= glm::toMat4( t.derived_rotate );
        tmp = glm::translate( tmp, t.derived_translate );
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