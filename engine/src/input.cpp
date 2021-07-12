#include "stdafx.h"

#include "neko_types.h"
#include "forwards.h"
#include "input.h"
#include "neko_platform.h"
#include "engine.h"
#include "console.h"
#include "locator.h"
#include "memory.h"

namespace neko {

  Input::Input( EnginePtr engine ): Subsystem( move( engine ) )
  {
  }

  void Input::initialize()
  {
    system_ = make_unique<nil::System>(
      platform::g_instance,
      platform::RenderWindowHandler::get().getWindow(),
      nil::Cooperation::Background,
      this );
  }

  void Input::onDeviceConnected( nil::Device* device )
  {
    engine_->console()->printf( Console::srcInput, "Device connected: %s", device->getName().c_str() );
    if ( device->getType() != nil::Device::Device_Controller )
      device->enable();
  }

  void Input::onDeviceDisconnected( nil::Device* device )
  {
    engine_->console()->printf( Console::srcInput, "Device disconnected: %s", device->getName().c_str() );
  }

  void Input::onMouseEnabled( nil::Device* device, nil::Mouse* instance )
  {
    engine_->console()->printf( Console::srcInput, "Mouse enabled: %s", device->getName().c_str() );
    instance->addListener( this );
  }

  void Input::onMouseDisabled( nil::Device* device, nil::Mouse* instance )
  {
    engine_->console()->printf( Console::srcInput, "Mouse disabled: %s", device->getName().c_str() );
  }

  void Input::onKeyboardEnabled( nil::Device* device, nil::Keyboard* instance )
  {
    engine_->console()->printf( Console::srcInput, "Keyboard enabled: %s", device->getName().c_str() );
    instance->addListener( this );
  }

  void Input::onKeyboardDisabled( nil::Device* device, nil::Keyboard* instance )
  {
    engine_->console()->printf( Console::srcInput, "Keyboard disabled: %s", device->getName().c_str() );
  }

  void Input::onControllerEnabled( nil::Device* device, nil::Controller* instance )
  {
  }

  void Input::onControllerDisabled( nil::Device* device, nil::Controller* instance )
  {
  }

  void Input::postInitialize()
  {
    for ( auto device : system_->getDevices() )
      if ( device->getType() != nil::Device::Device_Controller )
        device->enable();
  }

  void Input::shutdown()
  {
    system_.reset();
  }

  void Input::onMouseMoved( nil::Mouse* mouse, const nil::MouseState& state )
  {
    //
  }

  void Input::onMouseButtonPressed( nil::Mouse* mouse, const nil::MouseState& state, size_t button )
  {
    //
  }

  void Input::onMouseButtonReleased( nil::Mouse* mouse, const nil::MouseState& state, size_t button )
  {
    //
  }

  void Input::onMouseWheelMoved( nil::Mouse* mouse, const nil::MouseState& state )
  {
    //
  }

  void Input::onKeyPressed( nil::Keyboard* keyboard, const nil::VirtualKeyCode keycode )
  {
    //
  }

  void Input::onKeyRepeat( nil::Keyboard* keyboard, const nil::VirtualKeyCode keycode )
  {
    //
  }

  void Input::onKeyReleased( nil::Keyboard* keyboard, const nil::VirtualKeyCode keycode )
  {
    //
  }


  void Input::preUpdate( GameTime time )
  {
    system_->update();
  }

  void Input::tick( GameTime tick, GameTime time )
  {
    //
  }

  void Input::postUpdate( GameTime delta, GameTime tick )
  {
    //
  }

  Input::~Input()
  {
    //
  }

}