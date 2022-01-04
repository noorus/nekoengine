#include "pch.h"
#include "gui.h"
#include "lodepng.h"
#include "gfx.h"
#include "filesystem.h"

#ifndef NEKO_NO_GUI

namespace neko {

  const char c_guiBaseDirectory[] = R"(assets\gui\)";

  GUI::GUI()
  {
  }

  void GUI::initialize( Gfx* gfx, const utf8String& documentsPath, sf::Window& window )
  {
    guiPlatform_ = make_unique<MyGUI::NekoPlatform>();

    guiPlatform_->getDataManagerPtr()->setDataPath( c_guiBaseDirectory );

    guiPlatform_->initialise( &gfx->renderer(), this, documentsPath + "mygui.log" );

    gui_ = make_unique<MyGUI::Gui>();
    gui_->initialise( "gui_core.xml" );

    guiPlatform_->getRenderManagerPtr()->setViewSize( window.getSize().x, window.getSize().y );

    MyGUI::PointerManager::getInstance().setVisible( true );
    window.setMouseCursorVisible( false );

    fetchElements();
  }

  void GUI::fetchElements()
  {
    //auto loginRoot = MyGUI::LayoutManager::getInstance().loadLayout( "layout_login.layout" );
    //MyGUI::ButtonPtr button = gui_->createWidget<MyGUI::Button>( "Button", 10, 10, 300, 26, MyGUI::Align::Default, "Main" );
    //button->setCaption( "exit" );

    auto buildinfoRoot = MyGUI::LayoutManager::getInstance().loadLayout( "layout_mainscreen.layout" );
    for ( auto& root : buildinfoRoot )
    {
      if ( root->getName() == "g_pnlBuildInfo" )
      {
        elements.installationInfoBox_ = dynamic_cast<MyGUI::TextBox*>( root->getChildAt( 0 ) );
      }
      else if ( root->getName() == "g_pnlStats" )
      {
        elements.debugStatsBox_ = dynamic_cast<MyGUI::TextBox*>( root->getChildAt( 0 ) );
      }
    }
  }

  void GUI::setInstallationInfo( const rainet::GameInstallationState& install )
  {
    if ( !elements.installationInfoBox_ )
      return;

    char data[256];
    sprintf_s( data, 256,
      "build: %s\nbuild id: %lld\nbranch: %s\nsku: %s",
      install.host_ == rainet::InstallationHost::Local ? "local"
        : install.host_ == rainet::InstallationHost::Steam ? "steam"
        : "discord",
      install.buildId_,
      install.branch_.c_str(),
      install.ownership_ == rainet::GameOwnership::Owned ? "owned"
      : install.ownership_ == rainet::GameOwnership::TempFreeWeekend ? "free weekend"
      : install.ownership_ == rainet::GameOwnership::TempFamilySharing ? "family sharing"
      : "unowned" );

    elements.installationInfoBox_->setCaption( data );
  }

  void GUI::setDebugStats( const utf8String& stats )
  {
    if ( elements.debugStatsBox_ )
      elements.debugStatsBox_->setCaption( stats );
  }

  void GUI::resize( int width, int height )
  {
    if ( guiPlatform_ )
      guiPlatform_->getRenderManagerPtr()->setViewSize( width, height );
  }

  void* GUI::loadImage( int& width, int& height, MyGUI::PixelFormat& format, const utf8String& filename )
  {
    vector<uint8_t> input, output;
    unsigned int wo, ho;
    Locator::fileSystem().openFile( Dir_GUI, filename )->readFullVector( input );

    if ( lodepng::decode( output, wo, ho, input.data(), input.size(), LCT_RGBA, 8 ) != 0 )
      NEKO_EXCEPT( "Lodepng image load failed" );

    width = wo;
    height = ho;
    format = MyGUI::PixelFormat::R8G8B8A8;
    auto outbuf = new uint8_t[output.size()];
    memcpy( outbuf, output.data(), output.size() );
    return outbuf;
  }

  void GUI::saveImage( int width, int height, MyGUI::PixelFormat format, void* texture, const utf8String& filename )
  {
    //
  }

  void GUI::shutdown()
  {
    elements.reset();

    if ( gui_ )
    {
      gui_->shutdown();
      gui_.reset();
    }

    if ( guiPlatform_ )
    {
      guiPlatform_->shutdown();
      guiPlatform_.reset();
    }
  }

  GUI::~GUI()
  {

  }

}

#endif