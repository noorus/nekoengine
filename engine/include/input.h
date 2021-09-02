#pragma once

#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"
#include "director.h"
#include <nil.h>

namespace neko {

  class GfxInput: public nocopy, public nil::SystemListener, public nil::MouseListener, public nil::KeyboardListener {
  public:
    HWND window_;
    bool moved_;
    POINT mousePosition_;
    ConsolePtr console_;
    vec2i windowSize_;
    vec3i movement_;
    bool mouseButtons_[5] = { false };
  protected:
    unique_ptr<nil::System> system_;
    // nil::SystemListener events
    void onDeviceConnected( nil::Device* device ) override;
    void onDeviceDisconnected( nil::Device* device ) override;
    void onMouseEnabled( nil::Device* device, nil::Mouse* instance ) override;
    void onKeyboardEnabled( nil::Device* device, nil::Keyboard* instance ) override;
    void onControllerEnabled( nil::Device* device, nil::Controller* instance ) override;
    void onMouseDisabled( nil::Device* device, nil::Mouse* instance ) override;
    void onKeyboardDisabled( nil::Device* device, nil::Keyboard* instance ) override;
    void onControllerDisabled( nil::Device* device, nil::Controller* instance ) override;
    // nil::MouseListener events
    void onMouseMoved( nil::Mouse* mouse, const nil::MouseState& state ) override;
    void onMouseButtonPressed( nil::Mouse* mouse, const nil::MouseState& state, size_t button ) override;
    void onMouseButtonReleased( nil::Mouse* mouse, const nil::MouseState& state, size_t button ) override;
    void onMouseWheelMoved( nil::Mouse* mouse, const nil::MouseState& state ) override;
    // nil::KeyboardListener events
    void onKeyPressed( nil::Keyboard* keyboard, const nil::VirtualKeyCode keycode ) override;
    void onKeyRepeat( nil::Keyboard* keyboard, const nil::VirtualKeyCode keycode ) override;
    void onKeyReleased( nil::Keyboard* keyboard, const nil::VirtualKeyCode keycode ) override;
  public:
    GfxInput( ConsolePtr console );
    void initialize( HWND window );
    void setWindowSize( vec2i size );
    inline vec3i movement() const { return movement_; }
    inline bool mousebtn( size_t index ) const
    {
      assert( index >= 0 && index < 6 );
      return mouseButtons_[index];
    }
    void resetMovement();
    void shutdown();
    void update();
    virtual ~GfxInput();
  };

}