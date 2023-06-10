#include "pch.h"
#include "components.h"

namespace neko {

  namespace c {

    manager::manager( vec2 viewportResolution )
    {
      registry_.on_construct<transform>().connect<&registry::emplace_or_replace<dirty_transform>>();
      registry_.on_update<transform>().connect<&registry::emplace_or_replace<dirty_transform>>();

      camsys_ = make_unique<camera_system>( this, viewportResolution );

      root_ = registry_.create();
      registry_.emplace<node>( root_, "root" );
      registry_.emplace<transform>( root_ );
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

    entity manager::createNode( string_view name )
    {
      return createNode( root_, name );
    }

    entity manager::createCamera( string_view name )
    {
      auto e = createNode( root_, name );
      registry_.emplace<transform>( e );
      registry_.emplace<camera>( e );
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
      ImGui::Text( "Children %i", node.children );
      ImGui::Text( "First %i", node.first );
      ImGui::Text( "Next %i", node.next );
      ImGui::Text( "Previous %i", node.prev );
      ImGui::TextUnformatted( "Components" );
      if ( registry_.any_of<transform>( e ) )
      {
        const auto& t = tn( e );
        ComponentImguiWrap wrap( "Transform", 100.0f );
        igDragVector( "Translate", t.translate, 0.1f, 0.0f, 0.0f, "%.4f", ImGuiSliderFlags_None );
        igDragVector( "Scale", t.scale, 0.01f, 0.0f, 0.0f, "%.4f", ImGuiSliderFlags_None );
        // a bit wasteful but hardly an issue
        markDirty( e );
      }
      if ( registry_.any_of<camera>( e ) )
      {
        camsys_->imguiCameraEditor( e );
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