#pragma once
#include "forwards.h"
#include "gfx_types.h"
#include "neko_types.h"
#include "neko_platform.h"
#include "messaging.h"
#include "input.h"
#include "camera.h"

#include <MyGUI_NekoPlatform.h>

namespace neko {

#ifndef NEKO_NO_GUI

  class GUI: public MyGUI::NekoImageLoader, public nocopy {
  protected:
    unique_ptr<MyGUI::NekoPlatform> guiPlatform_;
    unique_ptr<MyGUI::Gui> gui_;
    struct Elements {
      MyGUI::TextBox* installationInfoBox_ = nullptr;
      MyGUI::TextBox* debugStatsBox_ = nullptr;
      void reset()
      {
        installationInfoBox_ = nullptr;
        debugStatsBox_ = nullptr;
      }
    } elements;
    void fetchElements();
  protected:
    void* loadImage( int& width, int& height, MyGUI::PixelFormat& format, const utf8String& filename );
    void saveImage( int width, int height, MyGUI::PixelFormat format, void* texture, const utf8String& filename );
  public:
    GUI();
    void initialize( Gfx* gfx, const utf8String& documentsPath, sf::Window& window );
    void resize( int width, int height );
    void setInstallationInfo( const rainet::GameInstallationState& install );
    void setDebugStats( const utf8String& stats );
    void shutdown();
    inline MyGUI::NekoPlatform* platform() const { return guiPlatform_.get(); }
    ~GUI();
  };

#else

  class GUI: public nocopy {
  public:
    inline void initialize( Gfx* gfx, const utf8String& documentsPath, sf::Window& window ) {}
    inline void poop() {}
    inline void resize( int width, int height ) {}
    inline void setDebugStats( const utf8String& shit ) {}
    inline void shutdown() {}
  };

#endif

  using GUIPtr = shared_ptr<GUI>;

}