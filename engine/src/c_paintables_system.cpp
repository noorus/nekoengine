#include "pch.h"
#include "components.h"
#include "camera.h"
#include "utilities.h"
#include "renderer.h"

namespace neko {

  using namespace gl;

  namespace c {

    static const utf8String c_toolPipelineName = "paint2d_tool";
    static const utf8String c_drawPipelineName = "paint2d_default";

    paintables_system::paintables_system( manager* m ): mgr_( m )
    {
      mgr_->reg().on_construct<paintable>().connect<&paintables_system::addSurface>( this );
      mgr_->reg().on_update<paintable>().connect<&paintables_system::updateSurface>( this );
      mgr_->reg().on_destroy<paintable>().connect<&paintables_system::removeSurface>( this );
    }

    paintables_system::~paintables_system()
    {
      mgr_->reg().on_construct<paintable>().disconnect<&paintables_system::addSurface>( this );
      mgr_->reg().on_update<paintable>().disconnect<&paintables_system::updateSurface>( this );
      mgr_->reg().on_destroy<paintable>().disconnect<&paintables_system::removeSurface>( this );
    }

    void paintables_system::addSurface( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_paintable>( e );
    }

    void paintables_system::updateSurface( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_paintable>( e );
    }

    void paintables_system::removeSurface( registry& r, entity e )
    {
      mgr_->reg().emplace_or_replace<dirty_paintable>( e );
    }

    void paintable::mouseClickTest(
      manager* m, entity e, Renderer& renderer, const Ray& ray, const vec2i& mousepos, int button )
    {
      bool hit = false;
      vec2 hitpos;
      if ( mesh && mesh->indices().size() == 6 )
      {
        auto& t = m->tn( e );
        const auto& verts = mesh->buffer().lock();
        const auto& indices = mesh->indices().lock();
        auto reverse = false;
        for ( int i = 0; i < 2; ++i )
        {
          // this will definitely not work for any other purpose
          auto triangle = ( i * 3 );
          vec3 v[3];
          if ( reverse )
          {
            v[0] = vec3( t.model() * vec4( verts.data()[indices.data()[triangle + 0]].position, 1.0f ) );
            v[1] = vec3( t.model() * vec4( verts.data()[indices.data()[triangle + 1]].position, 1.0f ) );
            v[2] = vec3( t.model() * vec4( verts.data()[indices.data()[triangle + 2]].position, 1.0f ) );
            hit = math::rayTriangleIntersection( ray, v[0], v[1], v[2], hitpos.y, hitpos.x );
          }
          else
          {
            v[0] = vec3( t.model() * vec4( verts.data()[indices.data()[triangle + 1]].position, 1.0f ) );
            v[1] = vec3( t.model() * vec4( verts.data()[indices.data()[triangle + 2]].position, 1.0f ) );
            v[2] = vec3( t.model() * vec4( verts.data()[indices.data()[triangle + 0]].position, 1.0f ) );
            hit = math::rayTriangleIntersection( ray, v[0], v[1], v[2], hitpos.y, hitpos.x );
            hitpos.x = ( 1.0f - hitpos.x );
            hitpos.y = ( 1.0f - hitpos.y );
          }
          reverse = ( !reverse );
          if ( hit )
            break;
        }
        mesh->buffer().unlock();
        mesh->indices().unlock();
      }
      if ( hit && lastPaintPos != mousepos )
      {
        lastPaintPos = mousepos;
        applyPaint( renderer, hitpos );
      }
    }

    static const vec4 c_layerPaintColors[4] = {
      { 1.0f, 0.0f, 0.0f, 0.0f },
      { 0.0f, 1.0f, 0.0f, 0.0f },
      { 0.0f, 0.0f, 1.0f, 0.0f },
      { 0.0f, 0.0f, 0.0f, 1.0f }
    };

    static const char* c_layerTextures[4] = {
      "ground_grasslands1",
      "ground_grasslands3",
      "ground_grasslands5",
      "ground_grasslands2"
    };

    void paintable::applyPaint( Renderer& renderer, const vec2& pos )
    {
      if ( !blendMap )
        return;

      auto& pipeline = renderer.shaders().usePipeline( c_toolPipelineName );
      auto pxpos = vec2i(
        math::min( math::max( math::iround( pos.x * static_cast<Real>( blendMap->width() ) ), 0 ), blendMap->width() - 1 ),
        math::min( math::max( math::iround( pos.y * static_cast<Real>( blendMap->height() ) ), 0 ), blendMap->height() - 1 )
      );
      pipeline.setUniform( "paint_mousepos", pxpos );
      pipeline.setUniform( "paint_brushtype", (int)paintBrushType );
      pipeline.setUniform( "paint_brushsize", paintBrushSize );
      pipeline.setUniform( "paint_brushsoften", paintBrushSoftness );
      pipeline.setUniform( "paint_brushopacity", paintBrushOpacity );
      pipeline.setUniform( "paint_brushcolor", c_layerPaintColors[paintSelectedLayerIndex] );
      pipeline.setUniform( "paint_noiseseed", paintBrushNoiseSeed );
      pipeline.setUniform( "paint_noisedetail", paintBrushNoiseDetail );
      pipeline.setUniform( "paint_noiseoffset", paintBrushNoiseOffset );
      pipeline.setUniform( "paint_noiseedgesonly", paintBrushNoiseEdgesOnly );
      pipeline.setUniform( "paint_noisemultiplier", paintBrushNoiseAmount );
      renderer.bindImageTexture( 0, blendMap->texture() );
      renderer.bindTexture( 0, blendMap->texture() );
      glDispatchCompute( blendMap->width() / 8, blendMap->height() / 8, 1 );
      glMemoryBarrier( gl::MemoryBarrierMask::GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
    }

    void PaintableVerterbuffer::draw(
      Renderer& renderer, const span<GLuint> textures, const mat4& model, const vec2& pxdimensions, float pxscale )
    {
      assert( textures.size() == 5 );
      gl::glBindVertexArray( vao_ );
      auto& pipeline = renderer.shaders().usePipeline( c_drawPipelineName );
      gl::glBindTextures( 0, static_cast<GLsizei>( textures.size() ), textures.data() );
      pipeline.setUniform( "texture_layer0_diffuse", 0 );
      glActiveTexture( GL_TEXTURE0 );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
      pipeline.setUniform( "texture_layer1_diffuse", 1 );
      pipeline.setUniform( "texture_layer2_diffuse", 2 );
      pipeline.setUniform( "texture_layer3_diffuse", 3 );
      pipeline.setUniform( "texture_blendmap", 4 );
      pipeline.setUniform( "texture_dimensions", pxdimensions );
      pipeline.setUniform( "pixelscale", pxscale );
      pipeline.setUniform( "model", model );
      gl::glDrawElements( gl::GL_TRIANGLES, static_cast<gl::GLsizei>( indices_->size() ), gl::GL_UNSIGNED_INT, nullptr );
      gl::glBindVertexArray( 0 );
    }

    void paintables_system::update( Renderer& renderer )
    {
      set<entity> bad;

      auto view = mgr_->reg().view<dirty_paintable>();
      for ( auto e : view )
      {
        auto& pt = mgr_->paintable2d( e );
        pt.textures.resize( 5 );
        bool okay = true;
        for ( auto i = 0; i < 4; ++i )
        {
          auto mat = renderer.materials().getPtr( c_layerTextures[i] );
          if ( ( !mat || !mat->uploaded() ) && okay )
          {
            okay = false;
            break;
          }
          pt.textures[i] = mat->textureHandle( 0 );
          glTextureParameteri( pt.textures[i], GL_TEXTURE_WRAP_S, GL_REPEAT );
          glTextureParameteri( pt.textures[i], GL_TEXTURE_WRAP_T, GL_REPEAT );
        }
        if ( !okay )
        {
          bad.insert( e );
          pt.textures.clear();
          continue;
        }
        auto parts =
          Locator::meshGenerator().makePlane(
            vec2( pt.dimensions ) * c_pixelScaleValues[pt.pixelScaleBase],
            { 1, 1 },
            { 0.0f, 0.0f, 1.0f } );
        if ( !pt.mesh || pt.mesh->buffer().size() != parts.first.size() ||
              pt.mesh->indices().size() != parts.second.size() )
          pt.mesh = make_unique<PaintableVerterbuffer>( parts.first.size(), parts.second.size() );
        const auto& verts = pt.mesh->buffer().lock();
        const auto& indces = pt.mesh->indices().lock();
        memcpy( verts.data(), parts.first.data(), parts.first.size() * sizeof( Vertex3D ) );
        memcpy( indces.data(), parts.second.data(), parts.second.size() * sizeof( GLuint ) );
        pt.mesh->buffer().unlock();
        pt.mesh->indices().unlock();
        if ( !pt.blendMap || pt.blendMap->width() != pt.dimensions.x || pt.blendMap->height() != pt.dimensions.y )
          pt.blendMap = make_shared<PaintableTexture>( renderer, pt.dimensions.x, pt.dimensions.y, PixFmtColorRGBA32f );
        pt.textures[4] = pt.blendMap->handle();
      }

      mgr_->reg().clear<dirty_paintable>();
      for ( auto& e : bad )
        mgr_->reg().emplace_or_replace<dirty_paintable>( e );
    }

    void paintables_system::draw( Renderer& renderer, const Camera& cam )
    {
      auto view = mgr_->reg().view<paintable>();
      for ( auto e : view )
      {
        auto& pt = mgr_->paintable2d( e );
        if ( !pt.mesh || !pt.blendMap || pt.textures.size() < 5 )
          continue;
        auto& t = mgr_->tn( e );
        pt.mesh->draw( renderer, pt.textures, t.model(),
          vec2( pt.blendMap->width(), pt.blendMap->height() ),
          c_pixelScaleValues[pt.pixelScaleBase] );
      }
    }

    const char* c_brushNames[PaintBrushType::MAX_Brush] = { "classic", "circle", "rhombus" };

    void paintables_system::imguiPaintableSurfaceEditor( entity e )
    {
      auto& pt = mgr_->reg().get<paintable>( e );

      bool changed = false;
      {
        ig::ComponentChildWrapper wrap( "Paintable", 200.0f );
        changed |= ig::imguiPixelScaleSelector( pt.pixelScaleBase );
        changed |= ImGui::DragInt2( "dimensions", &pt.dimensions[0], 1.0f, 0, 4096 );
        changed |= ig::normalSelector( "normal", pt.normal_sel, pt.normal );
      }
      {
        ig::ComponentChildWrapper wrap( "Brush", 200.0f );
        changed |= ImGui::SliderInt( "layer", &pt.paintSelectedLayerIndex, 0, 3 );
        
        {
          auto selected = static_cast<int>( pt.paintBrushType );
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
          pt.paintBrushType = static_cast<PaintBrushType>( selected );
        }

        changed |= ImGui::SliderInt( "brush size", &pt.paintBrushSize, 1, 50 );
        changed |= ImGui::SliderFloat( "brush opacity", &pt.paintBrushOpacity, 0.0f, 1.0f );
        changed |= ImGui::SliderFloat( "brush softness", &pt.paintBrushSoftness, 0.0f, 1.0f );
        changed |= ImGui::SliderFloat( "noise amount", &pt.paintBrushNoiseAmount, 0.0f, 1.0f );
        changed |= ImGui::SliderFloat( "noise seed", &pt.paintBrushNoiseSeed, 0.1f, 1000.0f );
        changed |= ImGui::SliderInt( "noise detail", &pt.paintBrushNoiseDetail, 10, 200 );
        changed |= ImGui::SliderFloat( "noise offset", &pt.paintBrushNoiseOffset, -1.0f, 1.0f, "%.2f" );
        if ( ImGui::IsItemClicked( 1 ) )
          pt.paintBrushNoiseOffset = 0.0f;
        changed |= ImGui::Checkbox( "edges only", &pt.paintBrushNoiseEdgesOnly );
      }
      if ( changed )
        mgr_->reg().emplace_or_replace<dirty_paintable>( e );
    }

  }

}