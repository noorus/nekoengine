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
    MyGUI::TextBox* shit_ = nullptr;
  protected:
    void* loadImage( int& width, int& height, MyGUI::PixelFormat& format, const utf8String& filename );
    void saveImage( int width, int height, MyGUI::PixelFormat format, void* texture, const utf8String& filename );
  public:
    GUI();
    void initialize( Gfx* gfx, const utf8String& documentsPath, sf::Window& window );
    void poop();
    void resize( int width, int height );
    void setshit( const utf8String& shit );
    void shutdown();
    inline MyGUI::NekoPlatform* platform() const { return guiPlatform_.get(); }
    ~GUI();
  };

  using GUIPtr = shared_ptr<GUI>;

#endif

}