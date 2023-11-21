#include "pch.h"
#include "components.h"
#include "camera.h"
#include "utilities.h"
#include "renderer.h"

namespace ImGui {

  bool iconListButton( const char* str_id, const char* caption, bool selected, ImTextureID texture_id, ImVec2 btnsize, const ImVec2& iconsize, const ImVec2& uv0, const ImVec2& uv1,
    ImGuiButtonFlags flags )
  {
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if ( window->SkipItems )
      return false;

    auto id = window->GetID( str_id );

    const ImVec2 padding = g.Style.FramePadding;
    const ImRect bb( window->DC.CursorPos, window->DC.CursorPos + btnsize );
    btnsize -= padding * 2.0f;
    ItemSize( bb );
    if ( !ItemAdd( bb, id ) )
      return false;

    bool hovered, held;
    bool pressed = ButtonBehavior( bb, id, &hovered, &held, flags );

    const ImU32 col = GetColorU32( ( held && hovered ) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button );

    ImVec4 background = ImVec4( 0.5f, 0.5f, 0.5f, ( held && hovered ) ? 0.65f : hovered ? 0.5f : 0.2f );
    ImVec4 tint = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );

    RenderNavHighlight( bb, id );
    RenderFrame( bb.Min, bb.Max, selected ? col : GetColorU32( background ), true,
      ImClamp( (float)ImMin( padding.x, padding.y ), 0.0f, g.Style.FrameRounding ) );

    //window->DrawList->AddRectFilled( bb.Min + padding, bb.Max - padding, GetColorU32( background ) );
    auto offset = ( ( btnsize.y - iconsize.y ) * 0.5f );
    auto icmin = bb.Min + padding + ImVec2( offset, offset );
    window->DrawList->AddImage( texture_id, icmin, icmin + iconsize, uv0, uv1, GetColorU32( tint ) );

    RenderTextClipped( icmin + ImVec2( offset + iconsize.x + padding.x, 0.0f ), bb.Max, caption, nullptr, nullptr );

    return pressed;
  }

}

namespace neko {

  using namespace gl;

  const char* c_brushNames[PaintBrushType::MAX_Brush] = { "classic", "circle", "rhombus" };

  void EditorToolOptionsWindow::draw( EditorTool activeTool )
  {
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 7.0f, 6.0f ) );
    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 6, 4 ) );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0 );
    ImGui::PushStyleVar( ImGuiStyleVar_FrameBorderSize, 1 );
    ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 4, 6 ) );
    ImGui::Begin( "tool options" );
    bool changed = false;
    if ( activeTool == Tool_Pencil )
    {
      ImGui::Text( "tool: pen" );
      ImGui::Separator();
    }
    else if ( activeTool == Tool_Brush )
    {
      ImGui::Text( "tool: brush" );
      ImGui::Separator();
      {
        auto selected = static_cast<int>( paintBrushType );
        if ( ImGui::BeginCombo( "type", c_brushNames[selected], ImGuiComboFlags_None ) )
        {
          for ( int i = 0; i < MAX_Brush; ++i )
          {
            bool dummy = ( selected == i );
            if ( ImGui::Selectable( c_brushNames[i], &dummy ) )
              selected = i;
            if ( dummy )
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
        paintBrushType = static_cast<PaintBrushType>( selected );
      }
      changed |= ImGui::SliderInt( "brush size", &paintBrushSize, 1, 50 );
      changed |= ImGui::SliderFloat( "brush opacity", &paintBrushOpacity, 0.0f, 1.0f );
      changed |= ImGui::SliderFloat( "brush softness", &paintBrushSoftness, 0.0f, 1.0f );
      changed |= ImGui::SliderFloat( "noise amount", &paintBrushNoiseAmount, 0.0f, 1.0f );
      changed |= ImGui::SliderFloat( "noise seed", &paintBrushNoiseSeed, 0.1f, 1000.0f );
      changed |= ImGui::SliderInt( "noise detail", &paintBrushNoiseDetail, 10, 200 );
      changed |= ImGui::SliderFloat( "noise offset", &paintBrushNoiseOffset, -1.0f, 1.0f, "%.2f" );
      if ( ImGui::IsItemClicked( 1 ) )
        paintBrushNoiseOffset = 0.0f;
      changed |= ImGui::Checkbox( "edges only", &paintBrushNoiseEdgesOnly );
    }
    else if ( activeTool == Tool_Eraser )
    {
      ImGui::Text( "tool: eraser" );
      ImGui::Separator();
    }
    else
    {
      ImGui::Text( "no active tool" );
      ImGui::Separator();
    }
    ImGui::End();
    ImGui::PopStyleVar( 5 );
  }

  namespace c {

    static const utf8String c_toolPipelineName = "paint2d_tool";
    static const utf8String c_drawPipelineName = "paint2d_default";

    worldplanes_system::worldplanes_system( manager* m ): mgr_( m )
    {
      mgr_->reg().on_construct<worldplane>().connect<&worldplanes_system::addSurface>( this );
      mgr_->reg().on_update<worldplane>().connect<&worldplanes_system::updateSurface>( this );
      mgr_->reg().on_destroy<worldplane>().connect<&worldplanes_system::removeSurface>( this );
    }

    worldplanes_system::~worldplanes_system()
    {
      mgr_->reg().on_construct<worldplane>().disconnect<&worldplanes_system::addSurface>( this );
      mgr_->reg().on_update<worldplane>().disconnect<&worldplanes_system::updateSurface>( this );
      mgr_->reg().on_destroy<worldplane>().disconnect<&worldplanes_system::removeSurface>( this );
    }

    void worldplanes_system::addSurface( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_worldplane>( e );
    }

    void worldplanes_system::updateSurface( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_worldplane>( e );
    }

    void worldplanes_system::removeSurface( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_worldplane>( e );
    }

    void WorldplaneTexturePaintLayer::applyBrush( Renderer& renderer, const vec2& pos, PaintBrushToolOptions& opts )
    {
      if ( !blendMap_ )
        return;

      auto& pipeline = renderer.shaders().usePipeline( c_toolPipelineName );
      auto pxpos = vec2i(
        math::min( math::max( math::iround( pos.x * static_cast<Real>( blendMap_->width() ) ), 0 ), blendMap_->width() - 1 ),
        math::min( math::max( math::iround( pos.y * static_cast<Real>( blendMap_->height() ) ), 0 ), blendMap_->height() - 1 )
      );

      pipeline.setUniform( "paint_mousepos", pxpos );
      pipeline.setUniform( "paint_brushtype", (int)opts.paintBrushType );
      pipeline.setUniform( "paint_brushsize", opts.paintBrushSize );
      pipeline.setUniform( "paint_brushsoften", opts.paintBrushSoftness );
      pipeline.setUniform( "paint_brushopacity", opts.paintBrushOpacity );
      pipeline.setUniform( "paint_noiseseed", opts.paintBrushNoiseSeed );
      pipeline.setUniform( "paint_noisedetail", opts.paintBrushNoiseDetail );
      pipeline.setUniform( "paint_noiseoffset", opts.paintBrushNoiseOffset );
      pipeline.setUniform( "paint_noiseedgesonly", opts.paintBrushNoiseEdgesOnly );
      pipeline.setUniform( "paint_noisemultiplier", opts.paintBrushNoiseAmount );
      renderer.bindImageTexture( 0, blendMap_->texture() );
      renderer.bindTexture( 0, blendMap_->texture() );
      glDispatchCompute( blendMap_->width() / 8, blendMap_->height() / 8, 1 );
      glMemoryBarrier( gl::MemoryBarrierMask::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
    }

    WorldplaneTexturePaintLayer::WorldplaneTexturePaintLayer( const utf8String& textureName ):
      materialName_( textureName )
    {
    }

    void WorldplaneTexturePaintLayer::recreate( Renderer& renderer, vec2i dimensions )
    {
      dimensions_ = dimensions;
      material_ = renderer.materials().getPtr( materialName_ );
      if ( !material_ || !material_->uploaded() )
      {
        material_.reset();
        return;
      }
      auto handle = material_->textureHandle( 0 );
      glTextureParameteri( handle, GL_TEXTURE_WRAP_S, GL_REPEAT );
      glTextureParameteri( handle, GL_TEXTURE_WRAP_T, GL_REPEAT );
      if ( !blendMap_ || blendMap_->width() != dimensions_.x || blendMap_->height() != dimensions_.y )
        blendMap_ = make_shared<PaintableTexture>( renderer, dimensions_.x, dimensions_.y, PixFmtColorRGBA32f );
    }

    MaterialPtr WorldplaneTexturePaintLayer::icon() const
    {
      return material_;
    }

    const char* WorldplaneTexturePaintLayer::caption() const
    {
      return materialName_.c_str();
    }

    void WorldplaneTexturePaintLayer::setMaterial( MaterialPtr newmat )
    {
      material_ = newmat;
      materialName_ = material_->name();
    }

    void WorldplaneTexturePaintLayer::draw(
      Renderer& renderer, const Camera& cam, const Indexed3DVertexBuffer& mesh, const mat4& model, Real pixelScale ) const
    {
      if ( !blendMap_ || !material_ )
        return;
      mesh.begin();
      auto& pipeline = renderer.shaders().usePipeline( c_drawPipelineName );
      vector<GLuint> textures = { material_->textureHandle( 0 ), blendMap_->handle() };
      gl::glBindTextures( 0, static_cast<GLsizei>( textures.size() ), textures.data() );
      pipeline.setUniform( "diffuse_tex", 0 );
      pipeline.setUniform( "diffuse_dimensions", vec2( material_->width(), material_->height() ) );
      glActiveTexture( GL_TEXTURE0 );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
      // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      pipeline.setUniform( "blendmap_tex", 1 );
      pipeline.setUniform( "blendmap_dimensions", vec2( blendMap_->width(), blendMap_->height() ) );
      pipeline.setUniform( "pixelscale", pixelScale );
      pipeline.setUniform( "model", model );
      mesh.draw();
    }

    static const char* c_layerTextures[4] =
    {
      "ground_grasslands1",
      "ground_grasslands3",
      "ground_grasslands5",
      "ground_grasslands2"
    };

    worldplane::worldplane()
    {
      for ( int i = 0; i < 4; ++i )
        layers.push_back( make_unique<WorldplaneTexturePaintLayer>( c_layerTextures[i] ) );
      viz_ = make_unique<LineRenderBuffer<8>>();
    }

    bool worldplane::editorMouseClickTest( Editor& editor,  manager* m, entity e,
      Renderer& renderer, const Ray& ray, const vec2i& mousepos, int button )
    {
      if ( editor.activeTool() != Tool_Brush )
        return false;
      bool hit = false;
      auto hit_uv = vec2( 0.0f );
      // auto hit_pos = vec3( 0.0f );
      if ( mesh && mesh->indices().size() % 6 == 0 )
      {
        auto& t = m->tn( e );
        const auto& verts = mesh->buffer().lock();
        const auto& indices = mesh->indices().lock();
        for ( int i = 0; i < ( mesh->indices().size() / 3 ); ++i )
        {
          auto triangle = ( i * 3 );
          float u, v = 0.0f;
          Vertex3D* A = &verts.data()[indices.data()[triangle + 0]];
          Vertex3D* B = &verts.data()[indices.data()[triangle + 1]];
          Vertex3D* C = &verts.data()[indices.data()[triangle + 2]];
          hit = math::rayTriangleIntersection( ray, t.model() * vec4( A->position, 1.0f ),
            t.model() * vec4( B->position, 1.0f ), t.model() * vec4( C->position, 1.0f ), u, v );
          if ( hit )
          {
            auto w = ( 1.0f - u - v );
            hit_uv = u * A->texcoord + v * B->texcoord + w * C->texcoord;
            // hit_pos = t.model() * vec4( u * A->position + v * B->position + w * C->position, 1.0f );
            break;
          }
        }
        mesh->buffer().unlock();
        mesh->indices().unlock();
      }
      if ( hit && lastPaintPos != mousepos )
      {
        lastPaintPos = mousepos;
        for ( auto& l : layers )
          if ( l->selected() )
            l->applyBrush( renderer, hit_uv, editor.toolOptsWindow() );
        return true;
      }
      return false;
    }

    void worldplanes_system::update( Renderer& renderer )
    {
      set<entity> bad;

      auto view = mgr_->reg().view<dirty_worldplane>();
      for ( auto e : view )
      {
        auto& pt = mgr_->wp( e );
        pt.worldDimensions = vec2( pt.dimensions ) * c_pixelScaleValues[pt.pixelScaleBase];
        auto parts = Locator::meshGenerator().makePlane(
          pt.worldDimensions, vec2u( math::ceil( pt.worldDimensions ) ), { 0.0f, 0.0f, 1.0f } );
        if ( !pt.mesh || pt.mesh->buffer().size() != parts.first.size() ||
              pt.mesh->indices().size() != parts.second.size() )
          pt.mesh = make_unique<Indexed3DVertexBuffer>( parts.first.size(), parts.second.size() );
        pt.mesh->setFrom( parts );
        auto visverts = pt.viz_->buffer().lock();
        visverts[0].pos = vec3( -pt.worldDimensions.x * 0.5f, -pt.worldDimensions.y * 0.5f, 0.0f );
        visverts[1].pos = vec3( pt.worldDimensions.x * 0.5f, -pt.worldDimensions.y * 0.5f, 0.0f );
        visverts[2].pos = vec3( pt.worldDimensions.x * 0.5f, -pt.worldDimensions.y * 0.5f, 0.0f );
        visverts[3].pos = vec3( pt.worldDimensions.x * 0.5f, pt.worldDimensions.y * 0.5f, 0.0f );
        visverts[4].pos = vec3( pt.worldDimensions.x * 0.5f, pt.worldDimensions.y * 0.5f, 0.0f );
        visverts[5].pos = vec3( -pt.worldDimensions.x * 0.5f, pt.worldDimensions.y * 0.5f, 0.0f );
        visverts[6].pos = vec3( -pt.worldDimensions.x * 0.5f, pt.worldDimensions.y * 0.5f, 0.0f );
        visverts[7].pos = vec3( -pt.worldDimensions.x * 0.5f, -pt.worldDimensions.y * 0.5f, 0.0f );
        for ( int i = 0; i < 8; ++i )
          visverts[i].color = vec4( 1.0f, 0.0f, 0.0f, 1.0f );
        pt.viz_->buffer().unlock();
        for ( auto& l : pt.layers )
        {
          l->recreate( renderer, pt.dimensions );
        }
      }
      mgr_->reg().clear<dirty_worldplane>();
      for ( auto& e : bad )
        mgr_->reg().emplace_or_replace<dirty_worldplane>( e );
    }

    inline ImTextureID imtex( GLuint tex )
    {
      return reinterpret_cast<ImTextureID>( static_cast<uintptr_t>( tex ) );
    }

    void worldplanes_system::draw( Renderer& renderer, const Camera& cam )
    {
      auto view = mgr_->reg().view<worldplane>();
      for ( auto e : view )
      {
        auto& pt = mgr_->wp( e );
        if ( !pt.mesh )
          continue;
        auto& t = mgr_->tn( e );
        for ( const auto& l : pt.layers )
        {
          l->draw( renderer, cam, *pt.mesh, t.model(), c_pixelScaleValues[pt.pixelScaleBase] );
        }
        auto pl = &renderer.shaders().usePipeline( "dbg_line" );
        pt.viz_->draw( *pl, t.model(), 8, 0, gl::GL_LINES );
      }
    }

    void worldplanes_system::imguiWorldplaneEditor( Editor& editor, entity e, WorldplaneLayer** selection )
    {
      auto& pt = mgr_->reg().get<worldplane>( e );

      bool changed = false;
      {
        ig::ComponentChildWrapper wrap( "worldplane", 200.0f );
        changed |= ig::imguiPixelScaleSelector( pt.pixelScaleBase );
        changed |= ImGui::DragInt2( "dimensions", &pt.dimensions[0], 1.0f, 0, 4096 );
        changed |= ig::normalSelector( "normal", pt.normal_sel, pt.normal );
      }
      {
        float pad = 8.0f;
        float sep = 4.0f;
        ig::ComponentChildWrapper wrap( "layers", 200.0f );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( pad + 1.0f, pad ) );
        ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( pad, pad ) );
        ImGui::BeginChild( "current layer", ImVec2( 0, 102.0f ), true, ImGuiWindowFlags_MenuBar );
        if ( ImGui::BeginMenuBar() )
        {
          if ( ImGui::BeginMenu( "selected" ) )
            ImGui::EndMenu();
          ImGui::EndMenuBar();
          if ( *selection )
          {
            ImGui::Image( imtex( ( *selection )->icon()->textureHandle( 0 ) ), ImVec2( 64.0f, 64.0f ) );
            ImGui::SameLine();
            ImGui::Text( ( *selection )->caption() );
          }
          if ( (*selection) )
          {
            ImGui::SameLine( ImGui::GetContentRegionAvail().x - 80.0f );
            auto ptp = ImGui::GetCursorPos();
            if ( ImGui::Button( "deselect", ImVec2( 80.0f, 24.0f ) ) )
            {
              *selection = nullptr;
              for ( auto& l : pt.layers )
                l->selected( false );
            }
            ImGui::SetCursorPos( ptp + ImVec2( 0.0f, 24.0f + sep ) );
            auto key = (uintptr_t)( *selection );
            if ( editor.hasSelectOp( key ) )
            {
              if ( ImGui::Button( "cancel", ImVec2( 80.0f, 24.0f ) ) )
                editor.eraseSelectOp( key );
              else if ( editor.getSelectOp( key ).result )
              {
                ( *selection )->setMaterial( editor.getSelectOp( key ).result );
                editor.eraseSelectOp( key );
              }
              else if ( !editor.getSelectOp( key ).open )
                editor.eraseSelectOp( key );
            }
            else
            {
              if ( ImGui::Button( "change", ImVec2( 80.0f, 24.0f ) ) )
                editor.startSelectOp( key );
            }
          }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar( 2 );
        ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 2.0f, 2.0f ) );
        //ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 2.0f, 2.0f ) );
        if ( !pt.layers.empty() )
        {
          auto fullw = ImGui::GetContentRegionAvail().x;
          ImGui::BeginChild( "list", ImVec2( fullw, 200.0f ), false, 0 );
          for ( auto i = static_cast<int>( pt.layers.size() - 1 ); i >= 0; --i )
          {
            const auto& l = pt.layers[i];
            if ( !l->icon() )
              continue;
            ImVec2 btnsize = ImVec2( fullw - 16.0f, 40.0f );
            ImVec2 iconsize = ImVec2( 32.0f, 32.0f );
            ImVec2 uv0 = ImVec2( 0.0f, 0.0f );
            ImVec2 uv1 = ImVec2(
              iconsize.x / static_cast<Real>( l->icon()->width() ), iconsize.y / static_cast<Real>( l->icon()->height() ) );
            if ( ImGui::iconListButton( utils::ilprintf( "wp/%i/lr/%i", e, i ).c_str(),
              utils::ilprintf( "layer %i: %s", i, l->caption() ).c_str(), l->selected(),
                imtex( l->icon()->textureHandle( 0 ) ), btnsize, iconsize, uv0, uv1, 0 ) )
            {
              l->selected( !l->selected() );
              *selection = ( l->selected() ? l.get() : nullptr );
              for ( int j = 0; j < pt.layers.size(); ++j )
                if ( j != i )
                  pt.layers[j]->selected( false );
            }
          }
          ImGui::EndChild();
        }
        ImGui::PopStyleVar();
      }
      if ( changed )
        mgr_->reg().emplace_or_replace<dirty_worldplane>( e );
    }

  }

}