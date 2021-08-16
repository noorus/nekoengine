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

  GfxInput::GfxInput( ConsolePtr console ): console_( console ), movement_( 0 )
  {
  }

  void GfxInput::initialize( HWND window )
  {
    window_ = window;

    system_ = make_unique<nil::System>(
      platform::g_instance,
      window,
      nil::Cooperation::Foreground,
      this );

    for ( auto device : system_->getDevices() )
      if ( device->getType() != nil::Device::Device_Controller )
        device->enable();
  }

  void GfxInput::onDeviceConnected( nil::Device* device )
  {
    console_->printf( Console::srcInput, "Device connected: %s", device->getName().c_str() );
    if ( device->getType() != nil::Device::Device_Controller )
      device->enable();
  }

  void GfxInput::onDeviceDisconnected( nil::Device* device )
  {
    console_->printf( Console::srcInput, "Device disconnected: %s", device->getName().c_str() );
  }

  void GfxInput::onMouseEnabled( nil::Device* device, nil::Mouse* instance )
  {
    console_->printf( Console::srcInput, "Mouse enabled: %s", device->getName().c_str() );
    instance->addListener( this );
  }

  void GfxInput::onMouseDisabled( nil::Device* device, nil::Mouse* instance )
  {
    console_->printf( Console::srcInput, "Mouse disabled: %s", device->getName().c_str() );
  }

  void GfxInput::onKeyboardEnabled( nil::Device* device, nil::Keyboard* instance )
  {
    console_->printf( Console::srcInput, "Keyboard enabled: %s", device->getName().c_str() );
    instance->addListener( this );
  }

  void GfxInput::onKeyboardDisabled( nil::Device* device, nil::Keyboard* instance )
  {
    console_->printf( Console::srcInput, "Keyboard disabled: %s", device->getName().c_str() );
  }

  void GfxInput::onControllerEnabled( nil::Device* device, nil::Controller* instance )
  {
  }

  void GfxInput::onControllerDisabled( nil::Device* device, nil::Controller* instance )
  {
  }

  void GfxInput::onMouseMoved( nil::Mouse* mouse, const nil::MouseState& state )
  {
    if ( state.buttons[1].pushed )
      movement_ += vec3i( state.movement.relative.x, state.movement.relative.y, 0 );
    moved_ = true;
  }

  void GfxInput::resetMovement()
  {
    movement_ = vec3i( 0 );
  }

  void GfxInput::onMouseButtonPressed( nil::Mouse* mouse, const nil::MouseState& state, size_t button )
  {
#ifndef NEKO_NO_GUI
    platform::getCursorPosition( window_, mousePosition_ );
    MyGUI::InputManager::getInstance().injectMousePress(
      (int)mousePosition_.x, (int)mousePosition_.y,
      MyGUI::MouseButton( MyGUI::MouseButton::Enum( button ) ) );
#endif
  }

  void GfxInput::onMouseButtonReleased( nil::Mouse* mouse, const nil::MouseState& state, size_t button )
  {
#ifndef NEKO_NO_GUI
    platform::getCursorPosition( window_, mousePosition_ );
    MyGUI::InputManager::getInstance().
    MyGUI::InputManager::getInstance().injectMouseRelease(
      (int)mousePosition_.x, (int)mousePosition_.y,
      MyGUI::MouseButton( MyGUI::MouseButton::Enum( button ) ) );
#endif
  }

  void GfxInput::onMouseWheelMoved( nil::Mouse* mouse, const nil::MouseState& state )
  {
    moved_ = true;
    movement_.z += state.wheel.relative;
  }

  void GfxInput::onKeyPressed( nil::Keyboard* keyboard, const nil::VirtualKeyCode keycode )
  {
#ifndef NEKO_NO_GUI
    MyGUI::InputManager::getInstance().injectKeyPress( MyGUI::KeyCode( (MyGUI::KeyCode::Enum)keycode ) );
#endif
  }

  void GfxInput::onKeyRepeat( nil::Keyboard* keyboard, const nil::VirtualKeyCode keycode )
  {
    //
  }

  void GfxInput::onKeyReleased( nil::Keyboard* keyboard, const nil::VirtualKeyCode keycode )
  {
#ifndef NEKO_NO_GUI
    MyGUI::InputManager::getInstance().injectKeyRelease( MyGUI::KeyCode( (MyGUI::KeyCode::Enum)keycode ) );
#endif
  }

  void GfxInput::setWindowSize( vec2i size )
  {
    windowSize_ = size;
  }

  void GfxInput::update()
  {
    moved_ = false;
    resetMovement();
    system_->update();
    if ( moved_ )
    {
      platform::getCursorPosition( window_, mousePosition_ );
      if ( mousePosition_.x < 0 )
        mousePosition_.x = 0;
      else if ( mousePosition_.x >= windowSize_.x )
        mousePosition_.x = (LONG)windowSize_.x - 1;
      if ( mousePosition_.y < 0 )
        mousePosition_.y = 0;
      else if ( mousePosition_.y >= windowSize_.y )
        mousePosition_.y = (LONG)windowSize_.y - 1;
#ifndef NEKO_NO_GUI
      MyGUI::InputManager::getInstance().injectMouseMove(
        (int)mousePosition_.x, (int)mousePosition_.y, (int)movement_.z );
#endif
      moved_ = false;
    }
  }

  void GfxInput::shutdown()
  {
    system_.reset();
  }

  GfxInput::~GfxInput()
  {
    //
  }

}