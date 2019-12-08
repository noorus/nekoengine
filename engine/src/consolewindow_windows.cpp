#include "stdafx.h"

#ifdef NEKO_PLATFORM_WINDOWS

#include "console.h"
#include "consolewindow_windows.h"
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

    namespace gdip {
      using namespace Gdiplus;
    }

    const wchar_t cRichEditControl[] = L"RichEdit50W";
    // const wchar_t cRichEditControl[] = L"RichEdit60W";
    const float cNativeDPI = 96.0f;

    //! Get generic window text as widestring
    inline wstring getWindowText( HWND wnd ) throw( )
    {
      wstring str;
      int length = GetWindowTextLengthW( wnd );
      if ( !length )
        return str;
      str.resize( length + 1, '\0' );
      GetWindowTextW( wnd, &str[0], length + 1 );
      str.resize( length );
      return str;
    }

    // Window

    Window::Window( HINSTANCE instance, WNDPROC wndproc, void* userdata ):
      instance_( instance ), wndProc_( wndproc ), userData_( userdata ), handle_( nullptr ), class_( 0 )
    {
    }

    Window::~Window()
    {
      if ( handle_ )
        DestroyWindow( handle_ );
      if ( class_ )
        UnregisterClassW( (LPCWSTR)class_, instance_ );
    }

    void Window::create( const string& classname, const string& title, int x, int y, int w, int h )
    {
      wstring wideClass = utf8ToWide( classname );
      wstring wideTitle = utf8ToWide( title );

      WNDCLASSEXW cls = { 0 };
      cls.cbSize = sizeof( WNDCLASSEXW );
      cls.lpfnWndProc = wndProc_;
      cls.hInstance = instance_;
      cls.hIcon = nullptr;
      cls.lpszClassName = wideClass.c_str();
      class_ = RegisterClassExW( &cls );
      if ( !class_ )
        NEKO_EXCEPT( "Window class registration failed" );

      bool sizable = true;
      bool maximizable = true;
      DWORD style = WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW | WS_POPUP;
      sizable ? style |= WS_SIZEBOX : style &= ~WS_SIZEBOX;
      maximizable ? style |= WS_MAXIMIZEBOX : style &= ~WS_MAXIMIZEBOX;

      handle_ = CreateWindowExW( 0, (LPCWSTR)class_, wideTitle.c_str(), style, x, y, w, h, nullptr, nullptr, instance_, userData_ );
      if ( !handle_ )
        NEKO_EXCEPT( "Window creation failed" );

      ShowWindow( handle_, SW_SHOWNORMAL );
      UpdateWindow( handle_ );
    }

    void Window::messageLoop( Event& stopEvent )
    {
      HANDLE handles[] = { stopEvent.get() };
      bool quitting = false;
      while ( !quitting )
      {
        auto wait = MsgWaitForMultipleObjects( 1, handles, FALSE, INFINITE, QS_ALLINPUT );
        if ( wait == WAIT_OBJECT_0 )
        {
          DestroyWindow( handle_ );
        }
        else if ( wait == WAIT_OBJECT_0 + 1 )
        {
          MSG msg;
          while ( PeekMessageW( &msg, nullptr, 0, 0, PM_REMOVE ) )
          {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
            if ( msg.message == WM_QUIT )
              quitting = true;
          }
        } else
          NEKO_EXCEPT( "Wait for multiple objects failed" );
      }
    }

    // ConsoleWindow

#   define WM_HIVE_CONSOLEFLUSHBUFFER (WM_USER + 1)

    const COLORREF c_consoleBackground = RGB( 255, 255, 255 );
    const COLORREF c_consoleForeground = RGB( 10, 13, 20 );

    const string c_consoleFont = "Lucida Console";
    const long c_consoleFontSize = 160; // in TWIPs. if you can figure out what the fuck that means in human terms, congratulations.

    const string c_consoleClassname = "nekoConsole"; // does not matter

    const int headerHeight = 6;
    const int ctrlMargin = 3;
    const int cmdlineHeight = 20;
    const long minWidth = 320;
    const long minHeight = 240;

    static bool g_haveSysDpiCall = false;

    void ConsoleWindow::Autocomplete::reset()
    {
      matches.clear();
      base.clear();
      suggestion = nullptr;
    }

    void ConsoleWindow::History::reset()
    {
      stack.clear();
      browsing = false;
      position = 0;
    }

    ConsoleWindow::ConsoleWindow( ConsolePtr console, const string& title, int x, int y, int w, int h ):
      Window( g_instance, wndProc, this ),
      cmdline_( nullptr ), log_( nullptr ),
      console_( move( console ) )
    {
      // Check and set a global flag for whether GetDpiForSystem API exists on this system (Windows 10 only)
      g_haveSysDpiCall = ( GetProcAddress( LoadLibraryW( L"user32.dll" ), "GetDpiForSystem" ) != nullptr );
      console_->addListener( this );
      create( c_consoleClassname, title, x, y, w, h );
    }

    ConsoleWindow::~ConsoleWindow()
    {
      console_->removeListener( this );
    }

    void ConsoleWindow::onConsolePrint( Console* console, vec3 color, const string& str )
    {
      lock_.lock();
      COLORREF clr = RGB(
        (uint8_t)( color.x * 255.0f ),
        (uint8_t)( color.y * 255.0f ),
        (uint8_t)( color.z * 255.0f ) );
      linesBuffer_.push_back({ utf8ToWide( str ), clr });
      lock_.unlock();
      PostMessageW( handle_, WM_HIVE_CONSOLEFLUSHBUFFER, 0, 0 );
    }

    void ConsoleWindow::flushBuffer()
    {
      lock_.lockShared();
      for ( auto& line : linesBuffer_ )
        print( line.clr, line.str );
      lock_.unlockShared();
      lock_.lock();
      linesBuffer_.clear();
      lock_.unlock();
    }

    void ConsoleWindow::initTextControl( HWND ctrl, bool lineinput )
    {
      SendMessageW( ctrl, EM_LIMITTEXT, lineinput ? 128 : -1, 0 );

      if ( lineinput )
      {
        SendMessageW( ctrl, EM_SETEVENTMASK, NULL, ENM_CHANGE );
      }
      else
      {
        SendMessageW( ctrl, EM_AUTOURLDETECT, TRUE, 0 );
        SendMessageW( ctrl, EM_SETEVENTMASK, NULL, ENM_SELCHANGE | ENM_SCROLL );
        SendMessageW( ctrl, EM_SETOPTIONS, ECOOP_OR,
          ECO_AUTOVSCROLL | ECO_NOHIDESEL | ECO_SAVESEL | ECO_SELECTIONBAR );
      }

      CHARFORMAT2W format;
      memset( &format, 0, sizeof( format ) );
      format.cbSize = sizeof( CHARFORMAT2W );
      format.dwMask = CFM_SIZE | CFM_OFFSET | CFM_EFFECTS | CFM_COLOR | CFM_BACKCOLOR | CFM_CHARSET | CFM_UNDERLINETYPE | CFM_FACE;
      format.yHeight = c_consoleFontSize;
      format.crTextColor = c_consoleForeground;
      format.crBackColor = c_consoleBackground;
      format.bCharSet = DEFAULT_CHARSET;
      format.bUnderlineType = CFU_UNDERLINENONE;

      wcscpy_s( format.szFaceName, LF_FACESIZE, utf8ToWide( c_consoleFont ).c_str() );

      SendMessageW( ctrl, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&format );
      SendMessageW( ctrl, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN | EC_USEFONTINFO, NULL );

      CHARRANGE range = { -1, -1 };
      SendMessageW( ctrl, EM_EXSETSEL, 0, (LPARAM)&range );

      SETTEXTEX textex = { ST_DEFAULT, 1200 };
      SendMessageW( ctrl, EM_SETTEXTEX, (WPARAM)&textex, NULL );
    }

    void ConsoleWindow::print( COLORREF color, const wstring& line )
    {
      CHARRANGE range = { -1, -1 };
      SendMessage( log_, EM_EXSETSEL, 0, (LPARAM)&range );
      CHARFORMAT2W format;
      format.cbSize = sizeof( CHARFORMAT2W );
      format.dwEffects = NULL;
      format.dwMask = CFM_COLOR | CFM_EFFECTS;
      format.crTextColor = color;
      SendMessage( log_, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&format );
      SETTEXTEX textex = { ST_SELECTION | ST_UNICODE, 1200 };
      if ( line.length() < 3 || 0x000a000d != *(uint32_t*)( line.c_str() + ( line.length() - 2 ) ) )
        SendMessage( log_, EM_SETTEXTEX, (WPARAM)&textex, (LPARAM)line.c_str() );
      else
      {
        wstring feedFixedLine = L"\r\n" + line.substr( 0, line.length() - 2 );
        SendMessage( log_, EM_SETTEXTEX, (WPARAM)&textex, (LPARAM)feedFixedLine.c_str() );
      }
    }

    void ConsoleWindow::print( const wstring& line )
    {
      print( c_consoleForeground, line );
    }

    void ConsoleWindow::print( const string& line )
    {
      print( utf8ToWide( line ) );
    }

    void ConsoleWindow::clearCmdline()
    {
      SETTEXTEX textex = { ST_DEFAULT, 1200 };
      SendMessageW( cmdline_, EM_SETEVENTMASK, NULL, ENM_NONE );
      SendMessageW( cmdline_, EM_SETTEXTEX, (WPARAM)&textex, (LPARAM)NULL );
      SendMessageW( cmdline_, EM_SETEVENTMASK, NULL, ENM_CHANGE );
    }

    void ConsoleWindow::setCmdline( const string& line )
    {
      auto wideLine = utf8ToWide( line );

      SETTEXTEX textex = { ST_DEFAULT, 1200 };
      SendMessageW( cmdline_, EM_SETEVENTMASK, NULL, ENM_NONE );
      SendMessageW( cmdline_, EM_SETTEXTEX, (WPARAM)&textex, (LPARAM)wideLine.c_str() );
      SendMessageW( cmdline_, EM_SETEVENTMASK, NULL, ENM_CHANGE );
    }

    inline gdip::RectF makeRectf( LONG left, LONG top, LONG right, LONG bottom )
    {
      return gdip::RectF( (float)left, (float)top, (float)right - left, (float)bottom - top );
    }

    void ConsoleWindow::paint( HWND wnd, HDC hdc, RECT& area )
    {
      auto width = ( area.right - area.left );
      auto height = ( area.bottom - area.top );

      HDC memdc = CreateCompatibleDC( hdc );
      BITMAPINFOHEADER bmpInfoHdr = { sizeof( BITMAPINFOHEADER ), width, -height, 1, 32 };
      HBITMAP hbitmap = CreateDIBSection( memdc, (BITMAPINFO*)( &bmpInfoHdr ), DIB_RGB_COLORS, nullptr, nullptr, 0 );
      HGDIOBJ oldbitmap = SelectObject( memdc, hbitmap );

      // ---

      gdip::Graphics gfx( memdc );

      auto header = makeRectf( area.left, area.top, area.right, headerHeight );
      auto client = makeRectf( area.left, area.top + headerHeight, area.right, area.bottom );

      gdip::SolidBrush brush_consoleHeader( gdip::Color( 255, 255, 0, 99 ) );
      gdip::SolidBrush brush_consoleBg( gdip::Color( 255, 240, 240, 240 ) );
      gdip::SolidBrush brush_transWhite( gdip::Color( 175, 255, 255, 255 ) );
      gdip::SolidBrush brush_transBlack( gdip::Color( 125, 0, 0, 0 ) );
      gdip::SolidBrush brush_editBg( gdip::Color( 255, 255, 255, 255 ) );
      gdip::SolidBrush brush_editBorder( gdip::Color( 255, 163, 163, 163 ) );

      gfx.FillRectangle( &brush_consoleHeader, header );

      gfx.FillRectangle( &brush_transWhite, 0, 0, width, 1 );
      gfx.FillRectangle( &brush_transBlack, 0, headerHeight - 1, width, 1 );

      gfx.FillRectangle( &brush_consoleBg, client );

      int editLeft = area.left + ctrlMargin;
      int editTop = area.bottom - ctrlMargin - cmdlineHeight;
      int editWidth = area.right - area.left - ( ctrlMargin * 2 );
      gfx.FillRectangle( &brush_editBorder, editLeft, editTop, editWidth, cmdlineHeight );
      gfx.FillRectangle( &brush_transWhite, editLeft + 1, editTop + 1, editWidth - 1, cmdlineHeight - 1 );
      gfx.FillRectangle( &brush_editBg, editLeft + 1, editTop + 1, editWidth - 2, cmdlineHeight - 2 );

      // ---

      BitBlt( hdc, 0, 0, area.right, area.bottom, memdc, 0, 0, SRCCOPY );
      SelectObject( memdc, oldbitmap );
      DeleteObject( hbitmap );
      DeleteDC( memdc );
    }

    inline RECT fitLogControl( int w, int h )
    {
      RECT ret;
      ret.left = ctrlMargin;
      ret.right = w - ret.left - ctrlMargin;
      ret.top = headerHeight + ctrlMargin;
      ret.bottom = h - ret.top - cmdlineHeight - ( ctrlMargin * 2 );
      return ret;
    }

    const int unpauseButtonWidth = 68;
    const int unpauseButtonHeight = 24;
    const int unpauseButtonMargin = 6;

    inline RECT fitUnpauseButton( int w, int h )
    {
      int scrollbarWidth = GetSystemMetrics( SM_CXVSCROLL );
      RECT ret;
      ret.left = ( w - ctrlMargin - scrollbarWidth - unpauseButtonMargin - unpauseButtonWidth );
      ret.right = unpauseButtonWidth;
      ret.top = headerHeight + ctrlMargin + unpauseButtonMargin;
      ret.bottom = unpauseButtonHeight;
      return ret;
    }

    const int cmdlinePadding = 2;
    const int cmdlineHeightFix = 2;

    inline RECT fitCmdlineControl( int w, int h )
    {
      RECT ret;
      ret.left = ctrlMargin + cmdlinePadding;
      ret.right = ( w - ret.left - ctrlMargin ) - cmdlinePadding; // really width
      ret.top = ( h - cmdlineHeight - ctrlMargin ) + cmdlinePadding + cmdlineHeightFix;
      ret.bottom = cmdlineHeight - ( cmdlinePadding * 2 ) - cmdlineHeightFix; // really height
      return ret;
    }

    void ConsoleWindow::forwardExecute( const wstring& command )
    {
      console_->queueCommand( wideToUtf8( command ) );
    }

    LRESULT ConsoleWindow::cmdlineProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
    {
      auto window = (ConsoleWindow*)GetWindowLongPtrW( wnd, GWLP_USERDATA );
      auto console = window->console_;

      CHARRANGE range = { 0, -1 };

      if ( msg == WM_CHAR && wparam == TAB )
        return 0;
      else if ( msg == WM_KEYDOWN )
      {
        if ( wparam == VK_TAB )
        {
          auto typed = wideToUtf8( getWindowText( window->cmdline_ ) );
          if ( typed.length() < 1 )
          {
            window->autocomplete_.suggestion = nullptr;
            return 0;
          }
          if ( window->autocomplete_.suggestion )
            console->autoComplete( window->autocomplete_.base, window->autocomplete_.matches );
          else
          {
            window->autocomplete_.base = typed;
            console->autoComplete( typed, window->autocomplete_.matches );
          }
          if ( window->autocomplete_.matches.empty() )
          {
            window->autocomplete_.suggestion = nullptr;
            return 0;
          }
          if ( window->autocomplete_.suggestion )
          {
            auto match = window->autocomplete_.matches.begin();
            while ( match != window->autocomplete_.matches.end() )
            {
              if ( ( *match ) == window->autocomplete_.suggestion )
              {
                ++match;
                if ( match == window->autocomplete_.matches.end() )
                  break;
                window->autocomplete_.suggestion = ( *match );
                window->setCmdline( window->autocomplete_.suggestion->name() + " " );
                SendMessageW( window->cmdline_, EM_EXSETSEL, 0, (LPARAM)&range );
                return 0;
              }
              ++match;
            }
          }
          window->autocomplete_.suggestion = window->autocomplete_.matches.front();
          window->setCmdline( window->autocomplete_.suggestion->name() + " " );
          SendMessageW( window->cmdline_, EM_EXSETSEL, 0, (LPARAM)&range );
          return 0;
        }
        else if ( wparam == VK_UP && !window->history_.stack.empty() )
        {
          if ( window->history_.browsing )
          {
            if ( window->history_.position <= 0 )
            {
              window->history_.position = 0;
              SendMessageW( window->cmdline_, EM_EXSETSEL, 0, (LPARAM)&range );
              return 0;
            }
            window->history_.position--;
            auto backline = window->history_.stack[window->history_.position];
            window->setCmdline( backline.c_str() );
            SendMessageW( window->cmdline_, EM_EXSETSEL, 0, (LPARAM)&range );
          }
          else
          {
            window->history_.browsing = true;
            window->history_.position = window->history_.stack.size() - 1;
            auto backline = window->history_.stack.back();
            auto strtemp = wideToUtf8( getWindowText( window->cmdline_ ) );
            window->history_.stack.push_back( strtemp );
            window->setCmdline( backline );
            SendMessageW( window->cmdline_, EM_EXSETSEL, 0, (LPARAM)&range );
          }
          return 0;
        }
        else if ( wparam == VK_DOWN && ( !window->history_.stack.empty() || window->history_.browsing ) )
        {
          window->history_.position++;
          if ( window->history_.position >= window->history_.stack.size() )
          {
            window->history_.position = window->history_.stack.size() - 1;
            SendMessageW( window->cmdline_, EM_EXSETSEL, 0, (LPARAM)&range );
            window->history_.browsing = false;
            return 0;
          }
          auto backline = window->history_.stack[window->history_.position];
          window->setCmdline( backline );
          SendMessageW( window->cmdline_, EM_EXSETSEL, 0, (LPARAM)&range );
          if ( window->history_.position >= window->history_.stack.size() - 1 )
          {
            window->history_.stack.pop_back();
            window->history_.position = window->history_.stack.size() - 1;
            window->history_.browsing = false;
          }
          return 0;
        }
        else if ( wparam == VK_RETURN )
        {
          if ( window->history_.browsing )
          {
            window->history_.stack.pop_back();
            window->history_.position = window->history_.stack.size() - 1;
            window->history_.browsing = false;
          }
          window->autocomplete_.reset();
          auto line = getWindowText( window->cmdline_ );
          if ( !line.empty() )
          {
            window->forwardExecute( line );
            window->history_.stack.push_back( wideToUtf8( line ) );
            window->history_.position = window->history_.stack.size() - 1;
          }
          window->clearCmdline();
          range.cpMin = -1;
          SendMessageW( window->cmdline_, EM_EXSETSEL, 0, (LPARAM)&range );
          return 0;
        }
      }

      return CallWindowProcW( window->baseCmdlineProc_, wnd, msg, wparam, lparam );
    }

    LRESULT ConsoleWindow::logProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
    {
      auto window = (ConsoleWindow*)GetWindowLongPtrW( wnd, GWLP_USERDATA );
      auto console = window->console_;

      if ( msg == WM_CHAR )
      {
        SetFocus( window->cmdline_ );
        SendMessageW( window->cmdline_, WM_CHAR, wparam, lparam );
        return 0;
      }

      return CallWindowProcW( window->baseLogProc_, wnd, msg, wparam, lparam );
    }

    void ConsoleWindow::recheckLogState()
    {
      bool shouldPause = false;

      //POINT scroll;
      //SendMessageW( log_, EM_GETSCROLLPOS, 0, (LPARAM)&scroll );
      logState_.linecount = SendMessageW( log_, EM_GETLINECOUNT, 0, 0 );
      logState_.currentline = SendMessageW( log_, EM_LINEFROMCHAR, -1, 0 );
      logState_.firstvisibleline = SendMessageW( log_, EM_GETFIRSTVISIBLELINE, 0, 0 );

      auto lastlinechar = SendMessageW( log_, EM_LINEINDEX, logState_.linecount - 1, 0 );
      POINTL lastlinept;
      SendMessageW( log_, EM_POSFROMCHAR, (WPARAM)&lastlinept, lastlinechar );

      if ( logState_.selection || lastlinept.y > logFit_.bottom )
        shouldPause = true;

      if ( shouldPause && !logState_.paused )
        logPause();
      else if ( !shouldPause && logState_.paused )
        logUnpause();
    }

    void ConsoleWindow::logPause()
    {
      logState_.paused = true;
      ShowWindow( unpauseButton_, SW_SHOWNA );
    }

    void ConsoleWindow::logUnpause()
    {
      logState_.paused = false;
      ShowWindow( unpauseButton_, SW_HIDE );
      CHARRANGE range = { -1, -1 };
      SendMessageW( log_, EM_EXSETSEL, 0, (LPARAM)&range );
      SendMessageW( log_, EM_SCROLLCARET, 0, 0 );
      PostMessageW( handle_, WM_HIVE_CONSOLEFLUSHBUFFER, 0, 0 );
    }

    void ConsoleWindow::paintUnpauseButton( HDC hdc, RECT& rc )
    {
      gdip::Graphics gfx( hdc );

      gdip::SolidBrush brush_text( gdip::Color( 220, 25, 25, 25 ) );

      auto rect = makeRectf( rc.left, rc.top, rc.right, rc.bottom );

      gdip::FontFamily fontFamily( L"Segoe UI" );
      gdip::Font font( &fontFamily, 11, gdip::FontStyleBold, gdip::UnitPixel );

      gdip::StringFormat format;
      format.SetAlignment( gdip::StringAlignmentCenter );
      format.SetLineAlignment( gdip::StringAlignmentCenter );

      gfx.DrawString( L"unpause »", -1, &font, rect, &format, &brush_text );
    }

    LRESULT ConsoleWindow::wndProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
    {
      if ( msg == WM_CREATE )
      {
        auto self = (ConsoleWindow*)( (LPCREATESTRUCTW)lparam )->lpCreateParams;
        SetWindowLongPtrW( wnd, GWLP_USERDATA, (LONG_PTR)self );
        RECT rect;
        GetClientRect( wnd, &rect );

        self->logFit_ = fitLogControl( rect.right, rect.bottom );
        self->log_ = CreateWindowExW( WS_EX_LEFT | WS_EX_STATICEDGE, cRichEditControl, L"",
          WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD | WS_VSCROLL
          | ES_LEFT | ES_MULTILINE | ES_WANTRETURN | ES_READONLY
          | ES_SELECTIONBAR | ES_NOOLEDRAGDROP | ES_DISABLENOSCROLL | ES_AUTOVSCROLL,
          self->logFit_.left, self->logFit_.top, self->logFit_.right, self->logFit_.bottom, wnd, nullptr,
          self->instance_, (void*)self );

        RECT fit = fitCmdlineControl( rect.right, rect.bottom );
        self->cmdline_ = CreateWindowExW( WS_EX_LEFT,
          cRichEditControl, L"", WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD | ES_LEFT | ES_NOOLEDRAGDROP,
          fit.left, fit.top, fit.right, fit.bottom, wnd, nullptr,
          self->instance_, (void*)self );

        fit = fitUnpauseButton( rect.right, rect.bottom );
        self->unpauseButton_ = CreateWindowExW( 0, L"BUTTON", nullptr, WS_CLIPSIBLINGS | WS_CHILD | BS_PUSHBUTTON,
          fit.left, fit.top, fit.right, fit.bottom, wnd, nullptr, self->instance_, (void*)self );

        if ( !self->log_ || !self->cmdline_ || !self->unpauseButton_ )
          NEKO_EXCEPT( "Window control creation failed" );

        SetWindowLongPtrW( self->log_, GWLP_USERDATA, (LONG_PTR)self );
        self->baseLogProc_ = (WNDPROC)SetWindowLongPtrW(
          self->log_, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)logProc );

        SetWindowLongPtrW( self->cmdline_, GWLP_USERDATA, (LONG_PTR)self );
        self->baseCmdlineProc_ = (WNDPROC)SetWindowLongPtrW(
          self->cmdline_, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)cmdlineProc );

        self->initTextControl( self->log_, false );
        self->initTextControl( self->cmdline_, true );

#ifdef NEKO_TEST_WINVISTACOMPAT
        auto dpi = cNativeDPI;
#else
        auto dpi = ( g_haveSysDpiCall ? GetDpiForSystem() : cNativeDPI );
#endif
        self->dpiScaling_ = static_cast<float>( dpi ) / cNativeDPI;

        SetWindowPos( wnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE );
        SetWindowPos( self->unpauseButton_, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );

        return 0;
      }

      auto self = (ConsoleWindow*)GetWindowLongPtrW( wnd, GWLP_USERDATA );

      if ( msg == WM_ERASEBKGND )
      {
        /*RECT clientRect, topRect;
        GetClientRect( wnd, &clientRect );
        topRect = clientRect;
        topRect.bottom = headerHeight;
        clientRect.top += headerHeight;

        auto hdc = (HDC)wParam;
        FillRect( hdc, &topRect, (HBRUSH)GetStockObject( BLACK_BRUSH ) );
        FillRect( hdc, &clientRect, (HBRUSH)GetStockObject( WHITE_BRUSH ) );*/

        return 1;
      }
      else if ( msg == WM_ACTIVATE )
      {
        InvalidateRect( wnd, nullptr, true );
        return 0;
      }
      else if ( msg == WM_COMMAND )
      {
        if ( HIWORD( wparam ) == EN_CHANGE && (HANDLE)lparam == self->cmdline_ )
        {
          self->autocomplete_.reset();
          return 0;
        } else if ( HIWORD( wparam ) == EN_VSCROLL && (HANDLE)lparam == self->log_ )
        {
          self->recheckLogState();
        } else if ( HIWORD( wparam ) == BN_CLICKED && (HANDLE)lparam == self->unpauseButton_ )
        {
          self->logUnpause();
        }
      }
      else if ( msg == WM_NOTIFY )
      {
        auto selchange = (SELCHANGE*)lparam;
        if ( selchange->nmhdr.code == EN_SELCHANGE && selchange->nmhdr.hwndFrom == self->log_ )
        {
          self->logState_.selection = ( selchange->seltyp != SEL_EMPTY );
          self->recheckLogState();
        }
        auto custom = (NMCUSTOMDRAW*)lparam;
        if ( custom->hdr.code == NM_CUSTOMDRAW && custom->hdr.hwndFrom == self->unpauseButton_ )
        {
          if ( custom->dwDrawStage == CDDS_PREPAINT )
          {
            return CDRF_NOTIFYPOSTPAINT;
          } else if ( custom->dwDrawStage == CDDS_POSTPAINT )
          {
            self->paintUnpauseButton( custom->hdc, custom->rc );
            return CDRF_DODEFAULT;
          }
        }
      }
      else if ( msg == WM_PAINT )
      {
        PAINTSTRUCT ps;
        auto dc = BeginPaint( wnd, &ps );

        RECT clientRect;
        GetClientRect( wnd, &clientRect );

        self->paint( wnd, dc, clientRect );

        EndPaint( wnd, &ps );

        return 0;
      }
      else if ( msg == WM_GETMINMAXINFO )
      {
        auto minmax = (LPMINMAXINFO)lparam;
        minmax->ptMinTrackSize.x = minWidth;
        minmax->ptMinTrackSize.y = minHeight;
      }
      else if ( msg == WM_EXITSIZEMOVE )
      {
        InvalidateRect( wnd, nullptr, FALSE );
        return 0;
      }
      else if ( msg == WM_SIZE )
      {
        self->logFit_ = fitLogControl( LOWORD( lparam ), HIWORD( lparam ) );
        MoveWindow( self->log_, self->logFit_.left, self->logFit_.top, self->logFit_.right, self->logFit_.bottom, TRUE );
        RECT fit = fitCmdlineControl( LOWORD( lparam ), HIWORD( lparam ) );
        MoveWindow( self->cmdline_, fit.left, fit.top, fit.right, fit.bottom, TRUE );
        fit = fitUnpauseButton( LOWORD( lparam ), HIWORD( lparam ) );
        MoveWindow( self->unpauseButton_, fit.left, fit.top, fit.right, fit.bottom, TRUE );
        return 0;
      }
      else if ( msg == WM_SETFOCUS )
      {
        SetFocus( self->cmdline_ );
        return 0;
      }
      else if ( msg == WM_DESTROY )
      {
        PostQuitMessage( EXIT_SUCCESS );
        return 0;
      }
      else if ( msg == WM_HIVE_CONSOLEFLUSHBUFFER )
      {
        if ( !self->logState_.paused )
        {
          self->flushBuffer();
        }
        return 0;
      }

      return DefWindowProcW( wnd, msg, wparam, lparam );
    }

  }

}

#endif