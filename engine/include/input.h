#pragma once

#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"
#include "director.h"
#include <nil.h>

namespace neko {

  class Input: public Subsystem, public nil::SystemListener, public nil::MouseListener, public nil::KeyboardListener {
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
    Input( EnginePtr engine );
    void initialize();
    void postInitialize();
    void shutdown();
    void preUpdate( GameTime time ) override;
    void tick( GameTime tick, GameTime time ) override;
    void postUpdate( GameTime delta, GameTime tick ) override;
    virtual ~Input();
  };

}