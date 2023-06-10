#include "pch.h"
#include "neko_exception.h"
#include "engine.h"
#include "gfx.h"
#include "shaders.h"
#include "renderer.h"
#include "camera.h"
#include "font.h"
#include "console.h"
#include "messaging.h"
#include "gui.h"
#include "neko_types.h"
#include "loader.h"

namespace neko {

  using namespace gl;

  // clang-format off

  static const vector<EditorViewportDefinition> g_editorViewportDefs =
  {
    {
      .name = "top",
      .position = vec3( 0.0f, 10.0f, 0.0f ),
      .eye = vec3( 0.0f, -1.0f, 0.0f ),
      .up = vec3( 0.0f, 0.0f, 1.0f )
    },
    {
      .name = "front",
      .position = vec3( 0.0f, 0.0f, 10.0f ),
      .eye = vec3( 0.0f, 0.0f, -1.0f ),
      .up = vec3( 0.0f, 1.0f, 0.0f )
    },
    {
      .name = "left",
      .position = vec3( 10.0f, 0.0f, 0.0f ),
      .eye = vec3( -1.0f, 0.0f, 0.0f ),
      .up = vec3( 0.0f, 1.0f, 0.0f )
    }
  };

  // clang-format on

  void Editor::initialize( RendererPtr renderer, const vec2& realResolution )
  {
    viewports_.clear();

    for ( const auto& def : g_editorViewportDefs )
    {
      auto vp = make_shared<EditorViewport>( shared_from_this(), realResolution, def );
      viewports_.push_back( vp );
    }
  }

  void Editor::resize( const Viewport& windowViewport, GameViewport& gameViewport )
  {
    const auto width = windowViewport.size().x;
    const auto height = windowViewport.size().y;

    const auto halfsize = vec2i( width / 2, height / 2 );

    for ( int i = 0; i < viewports_.size(); ++i )
    {
      auto& viewport = viewports_[i];
      if ( i == 0 )
        viewport->move( 0, 0 );
      else if ( i == 1 )
        viewport->move( static_cast<int>( halfsize.x ), 0 );
      else if ( i == 2 )
        viewport->move( 0, static_cast<int>( halfsize.y ) );
      else if ( i == 3 )
        viewport->move( static_cast<int>( halfsize.x ), static_cast<int>( halfsize.y ) );
      viewport->resize( static_cast<GLsizei>( halfsize.x ), static_cast<GLsizei>( halfsize.y ), windowViewport );
      viewport->camera()->setViewport( vec2( static_cast<Real>( halfsize.x ), static_cast<Real>( halfsize.y ) ) );
    }

    gameViewport.move( static_cast<int>( halfsize.x ), static_cast<int>( halfsize.y ) );
    //gameViewport.resize( static_cast<GLsizei>( halfsize.x ), static_cast<GLsizei>( halfsize.y ), windowViewport );
    //gameViewport.camera()->setViewport( vec2( static_cast<Real>( halfsize.x ), static_cast<Real>( halfsize.y ) ) );
  }

  void Editor::updateRealtime( GameTime realTime, GameTime delta, GfxInputPtr input, SManager& scene,
    const Viewport& window, GameViewport& gameViewport )
  {
    size_t PANBTN = 2; // = input->mousebtn( 2 );
    size_t GAMEVP = 3;
    mousePos_ = vec2( static_cast<Real>( input->mousePosition_.x ), static_cast<Real>( input->mousePosition_.y ) );

    auto& igIO = ImGui::GetIO();
    if ( !igIO.WantCaptureMouse )
    {
      auto vpidx = GAMEVP;
      if ( !dragOp_.ongoing() )
      {
        const auto halfsize = vec2i( window.sizef() * 0.5f );
        if ( input->mousePosition_.x < halfsize.x )
          vpidx = ( input->mousePosition_.y < halfsize.y ? 0 : 2 );
        else
          vpidx = ( input->mousePosition_.y < halfsize.y ? 1 : 3 );
        if ( input->mouseButtons().wasPressed( PANBTN ) && vpidx != GAMEVP )
        {
          dragOp_.begin( viewports_[vpidx], mousePos_ );
          lastDragopPos_ = { 0.0f, 0.0f };
        }
      }
      if ( dragOp_.ongoing() )
      {
        dragOp_.update( mousePos_ );
        auto np = dragOp_.getRelative();
        auto diff = ( np - lastDragopPos_ );
        lastDragopPos_ = np;
        dragOp_.viewport()->camera()->applyInputPanning( diff );
        if ( input->mouseButtons().wasReleased( PANBTN ) )
          dragOp_.end();
      }
      else if ( vpidx == 3 )
      {
        /* auto gamecam = static_cast<OrbitCamera*>( gameViewport.camera().get() );
        if ( input->mouseButtons().isPressed( PANBTN ) )
          gamecam->applyInputPanning( input->movement() );
        else if ( input->mousebtn( 1 ) )
          gamecam->applyInputRotation( input->movement() );
        else if ( input->mouseButtons().wasPressed( 0 ) )
        {
          auto ret = gameViewport.windowPointToWorld( vec3( mousePos_, 0.8f ) );
          Locator::console().printf( Console::srcGame, "Gameviewport click %.2f %.2f %.2f (camera %.2f %.2f %.2f)",
            ret.x, ret.y, ret.z, gamecam->position().x, gamecam->position().y, gamecam->position().z );
        }

        gamecam->applyInputZoom( static_cast<int>( input->movement().z ) );*/
      }
      else
      {
        auto& vp = viewports_[vpidx];
        vp->camera()->applyInputZoom( static_cast<int>( input->movement().z ) );
      }
    }

    for ( auto& vp : viewports_ )
      vp->camera()->update( scene, delta, realTime );
  }

  bool Editor::draw( RendererPtr renderer, GameTime time, const Viewport& window, GameViewport& gameViewport )
  {
    if ( !enabled_ )
      return false;

    const auto halfsize = vec2i( window.sizef() * 0.5f );

    ImGui::SetNextWindowPos( { 0, 0 } );
    ImGui::SetNextWindowSize( { window.sizef().x, window.sizef().y } );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
    ImGui::Begin( "Invisible", nullptr,
      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoSavedSettings );

    for ( int i = 0; i < 3; ++i )
    {
      renderer->draw( time, *viewports_[i]->camera(), *viewports_[i], renderer->builtins().screenFourthQuads_[i] );
      auto& vp = viewports_[i];
      auto topleft = vp->posf();
      auto bottomright = topleft + vp->sizef();
      if ( topleft.y < mainMenuHeight_ )
        topleft.y = mainMenuHeight_;

      ImGui::GetBackgroundDrawList()->AddRect( topleft, bottomright, ImColor( 0.0f, 0.5f, 0.8f ) );
      auto mousepoint = vp->mapPointByWindow( mousePos_ );
      auto ndc = vp->ndcPointToWorld( { 0.0f, 0.0f, 0.0f } );
      ImGui::GetBackgroundDrawList()->AddText( topleft + 10.0f, ImColor( 1.0f, 1.0f, 1.0f ),
        utils::ilprinf( "%s - mouse %.2f %.2f camera %.2f %.2f %.2f dir %.2f %.2f %.2f aspect %.2f radius %.2f ndc %.2f %.2f %.2f", vp->name().c_str(),
          mousepoint.x, mousepoint.y,
          vp->camera()->position().x, vp->camera()->position().y, vp->camera()->position().z,
          vp->camera()->direction().x, vp->camera()->direction().y, vp->camera()->direction().z,
          vp->camera()->aspect(), vp->camera()->radius(),
          ndc.x, ndc.y, ndc.z )
          .c_str() );
    }

    auto vp = &gameViewport;
    utf8String title = "game";
    if ( vp->camera() )
    {
      title = utils::ilprinf( "game - camera %s", vp->cameraData()->name.c_str() );
      renderer->draw( time, *vp->camera(), *vp, renderer->builtins().screenFourthQuads_[3] );

    }
    ImGui::GetBackgroundDrawList()->AddText( gameViewport.posf() + 10.0f, ImColor( 1.0f, 1.0f, 1.0f ), title.c_str() );

    ImGui::End();
    ImGui::PopStyleVar( 2 );

    return true;
  }

  void Editor::shutdown()
  {
    viewports_.clear();
  }

}