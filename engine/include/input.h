#pragma once

#include "neko_types.h"
#include "forwards.h"
#include "subsystem.h"
#include "director.h"
#include <nil.h>

namespace neko {

  template <size_t Count>
  class ButtonTracker {
  protected:
    static constexpr size_t c_buttonCount = Count;
    std::array<bool, c_buttonCount> states_ = { false };
    std::array<bool, c_buttonCount> previousStates_ = { false };
  public:
    inline void reset( const vector<nil::Button>& state )
    {
      size_t count = math::min( state.size(), c_buttonCount );
      for ( auto i = 0; i < count; ++i )
      {
        previousStates_[i] = states_[i];
        states_[i] = state[i].pushed;
      }
    }
    inline void reset()
    {
      std::copy( states_.begin(), states_.end(), previousStates_.begin() );
    }
    inline void markPressed( size_t index )
    {
      assert( index < c_buttonCount );
      states_[index] = true;
    }
    inline void markReleased( size_t index )
    {
      assert( index < c_buttonCount );
      states_[index] = false;
    }
    inline void update( const vector<nil::Button>& state )
    {
      size_t count = math::min( state.size(), c_buttonCount );
      previousStates.swap( states_ );
      for ( auto i = 0; i < count; ++i )
        states_[i] = state[i].pushed;
    }
    inline const bool wasPressed( size_t index ) const
    {
      assert( index < c_buttonCount );
      return ( states_[index] && !previousStates_[index] );
    }
    inline const bool wasReleased( size_t index ) const
    {
      assert( index < c_buttonCount );
      return ( !states_[index] && previousStates_[index] );
    }
    inline const bool isPressed( size_t index ) const
    {
      assert( index < c_buttonCount );
      return ( states_[index] );
    }
  };

  class GfxInput: public nocopy, public nil::SystemListener, public nil::MouseListener, public nil::KeyboardListener {
  public:
    HWND window_ = 0;
    bool moved_ = false;
    POINT mousePosition_ { 0, 0 };
    ConsolePtr console_;
    vec2i windowSize_ { 0, 0 };
    vec3i movement_ { 0, 0, 0 };
    ButtonTracker<5> mouseButtons_;
  protected:
    nil::SystemPtr system_;
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
      assert( index >= 0 && index < 5 );
      return mouseButtons_.isPressed( index );
    }
    inline const ButtonTracker<5>& mouseButtons() const { return mouseButtons_; }
    void resetMovement();
    void shutdown();
    void update();
    virtual ~GfxInput();
  };

}