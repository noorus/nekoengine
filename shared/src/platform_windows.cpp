#include "stdafx.h"

#ifdef NEKO_PLATFORM_WINDOWS

#include "neko_platform_windows.h"
#include "neko_exception.h"

#include <windows.h>
#include <unknwn.h>
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#include <gdiplus.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib")

namespace neko {

  namespace platform {

    HINSTANCE g_instance = nullptr;

    const wchar_t cRichEditDLL[] = L"msftedit.dll";

    static HMODULE g_richEditLibrary = nullptr;
    static ULONG_PTR g_gdiplusToken = 0;
    static DWORD g_avrtGameTask = 0;

    namespace gdip {
      using namespace Gdiplus;
    }

    void initializeGUI()
    {
      INITCOMMONCONTROLSEX ctrls;
      ctrls.dwSize = sizeof( INITCOMMONCONTROLSEX );
      ctrls.dwICC = ICC_STANDARD_CLASSES | ICC_NATIVEFNTCTL_CLASS;

      if ( !InitCommonControlsEx( &ctrls ) )
        NEKO_EXCEPT( "Common controls init failed, missing manifest?" );

      g_richEditLibrary = LoadLibraryW( cRichEditDLL );
      if ( !g_richEditLibrary )
        NEKO_EXCEPT( "RichEdit library load failed" );

      gdip::GdiplusStartupInput gdiplusStartupInput;
      gdiplusStartupInput.SuppressExternalCodecs = TRUE;
      gdip::GdiplusStartup( &g_gdiplusToken, &gdiplusStartupInput, nullptr );
    }

    void shutdownGUI()
    {
      if ( g_gdiplusToken )
        gdip::GdiplusShutdown( g_gdiplusToken );

      if ( g_richEditLibrary )
        FreeLibrary( g_richEditLibrary );
    }

    void initialize()
    {
      initializeGUI();
    }

    void prepareProcess()
    {
      SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX );

      SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );

      AvSetMmThreadCharacteristicsW( L"Games", &g_avrtGameTask );
    }

    void shutdown()
    {
      shutdownGUI();
    }

    // Thread

    Thread::Thread( const string& name, Callback callback, void* argument ):
      thread_( nullptr ), id_( 0 ), name_( name ), callback_( callback ), argument_( argument )
    {
    }

    DWORD Thread::threadProc( void* argument )
    {
      auto self = (Thread*)argument;
      auto ret = self->callback_( self->run_, self->stop_, self->argument_ );
      return ( ret ? EXIT_SUCCESS : EXIT_FAILURE );
    }

    bool Thread::start()
    {
      thread_ = CreateThread( nullptr, 0, threadProc, this, CREATE_SUSPENDED, &id_ );
      if ( !thread_ )
        return false;

      setDebuggerThreadName( id_, name_ );

      if ( ResumeThread( thread_ ) == -1 )
        NEKO_EXCEPT( "Thread resume failed" );

      HANDLE events[2] = {
        run_.get(), thread_
      };

      auto wait = WaitForMultipleObjects( 2, events, FALSE, INFINITE );
      if ( wait == WAIT_OBJECT_0 )
      {
        return true;
      }
      else if ( wait == WAIT_OBJECT_0 + 1 )
      {
        return false;
      }
      else
        NEKO_EXCEPT( "Thread wait failed" );

      return false;
    }

    void Thread::stop()
    {
      if ( thread_ )
      {
        stop_.set();
        WaitForSingleObject( thread_, INFINITE );
      }
      stop_.reset();
      run_.reset();
      thread_ = nullptr;
      id_ = 0;
    }

    bool Thread::waitFor( uint32_t milliseconds ) const
    {
      auto retval = WaitForSingleObject( thread_, milliseconds );
      return ( retval == WAIT_TIMEOUT );
    }

    Thread::~Thread()
    {
      stop();
    }

    // RenderWindowHandler

    RenderWindowHandler* RenderWindowHandler::instance_ = nullptr;

    RenderWindowHandler::RenderWindowHandler():
      window_( 0 ), originalProc_( nullptr ),
      vaspect_( 0.0f ), haspect_( 0.0f ), resolution_(), borders_()
    {
    }

    void RenderWindowHandler::changeTargetResolution( const size2i targetResolution )
    {
      resolution_ = targetResolution;
      vaspect_ = ( (float)resolution_.w / (float)resolution_.h );
      haspect_ = ( (float)resolution_.h / (float)resolution_.w );
    }

    void RenderWindowHandler::getWindowBordersSize( HWND window, size2i& size, bool withClient )
    {
      auto style = (DWORD)GetWindowLongW( window, GWL_STYLE );
      auto exstyle = (DWORD)GetWindowLongW( window, GWL_EXSTYLE );
      RECT want;
      want.left = 100;
      want.right = 100 + resolution_.w;
      want.top = 100;
      want.bottom = 100 + resolution_.h;
      AdjustWindowRectEx( &want, style, FALSE, exstyle );
      size.w = ( want.right - want.left - resolution_.w );
      size.h = ( want.bottom - want.top - resolution_.h );
    }

    void RenderWindowHandler::wmSizing( WPARAM wparam, LPARAM lparam )
    {
      checkStartSizing();
      auto rect = (RECT*)lparam;
      auto dw = float( rect->right - rect->left );
      auto dh = float( rect->bottom - rect->top );
      auto w = dw - borders_.w;
      auto h = dh - borders_.h;
      bool sel = false;
      if ( wparam == WMSZ_LEFT || wparam == WMSZ_RIGHT )
        sel = true;
      else if ( wparam == WMSZ_TOP || wparam == WMSZ_BOTTOM )
        sel = false;
      else
        sel = ( ( w / h ) > vaspect_ );
      float nw = w, nh = h;
      if ( sel )
        nh = ( w * haspect_ );
      else
        nw = ( h * vaspect_ );
      nw += borders_.w;
      nh += borders_.h;
      if ( wparam == WMSZ_TOPLEFT || wparam == WMSZ_TOPRIGHT )
      {
        if ( wparam == WMSZ_TOPRIGHT )
          rect->right = rect->left + (LONG)math::round( nw );
        else
          rect->left = rect->right - (LONG)math::round( nw );
        rect->top = rect->bottom - (LONG)math::round( nh );
        return;
      }
      switch ( wparam )
      {
        case WMSZ_TOP:
        case WMSZ_BOTTOM:
          rect->left = (LONG)math::round( (float)rect->left + ( ( dw - nw ) / 2.0f ) );
        break;
        case WMSZ_BOTTOMLEFT:
          rect->left = (LONG)math::round( (float)rect->right - nw );
          rect->top = sel ? rect->top : (LONG)math::round( (float)rect->bottom - nh );
        break;
      }
      rect->right = rect->left + (LONG)math::round( nw );
      rect->bottom = rect->top + (LONG)math::round( nh );
      lastSize_ = size2i( *rect );
      RECT newClientRect = { 0, 0, (LONG)lastSize_.w, (LONG)lastSize_.h };
      InvalidateRect( window_, &newClientRect, FALSE );
    }

    void RenderWindowHandler::checkStartSizing()
    {
      if ( !resizing_ )
      {
        resizing_ = true;
        auto img = &target_->renderWindowReadPixels();
        auto dc = GetDC( window_ );
        snapshotPainter_.reset();
        snapshotPainter_.store( dc, img->size_, img->buffer_ );
        resizing_ = true;
        ReleaseDC( window_, dc );
      }
    }

    void RenderWindowHandler::wmEnterSizeMove()
    {
      checkStartSizing();
    }

    void RenderWindowHandler::wmExitSizeMove()
    {
      resizing_ = false;
      InvalidateRect( window_, nullptr, FALSE );
    }

    bool RenderWindowHandler::wmPaint()
    {
      if ( resizing_ )
      {
        PAINTSTRUCT ps;
        auto dc = BeginPaint( window_, &ps );
        snapshotPainter_.paint( dc, lastSize_ );
        EndPaint( window_, &ps );
        return true;
      } else
        return false;
    }

    LRESULT RenderWindowHandler::wndProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
    {
      if ( msg == WM_SIZING )
      {
        get().wmSizing( wparam, lparam );
        return TRUE;
      }
      else if ( msg == WM_ENTERSIZEMOVE )
      {
        get().wmEnterSizeMove();
      }
      else if ( msg == WM_EXITSIZEMOVE )
      {
        get().wmExitSizeMove();
      }
      else if ( msg == WM_ERASEBKGND )
      {
        return 1;
      }
      else if ( msg == WM_PAINT )
      {
        if ( get().wmPaint() )
          return 0;
      }
      return get().originalProc_( wnd, msg, wparam, lparam );
    }

    void RenderWindowHandler::setWindow( RenderWindowEventRecipient* callback, HWND window )
    {
      target_ = callback;
      if ( window_ )
      {
        SetWindowLongPtrW( window_, GWLP_WNDPROC, reinterpret_cast<DWORD_PTR>( originalProc_ ) );
      }
      window_ = window;
      if ( window_ )
      {
        getWindowBordersSize( window_, borders_, true );
        originalProc_ = reinterpret_cast<WNDPROC>( GetWindowLongPtrW( window_, GWLP_WNDPROC ) );
        SetWindowLongPtrW( window_, GWLP_WNDPROC, reinterpret_cast<DWORD_PTR>( RenderWindowHandler::wndProc ) );
      }
    }

  }

}

#endif