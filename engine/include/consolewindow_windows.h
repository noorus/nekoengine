#pragma once
#include "neko_types.h"
#include "neko_config.h"

#ifdef NEKO_PLATFORM_WINDOWS

#include "neko_exception.h"
#include "neko_platform.h"
#include "consolelistener.h"
#include "console.h"
#include "utilities.h"

namespace neko {

  namespace platform {

    class Window {
    protected:
      ATOM class_;
      WNDPROC wndProc_;
      HINSTANCE instance_;
      HWND handle_;
      void* userData_;
    public:
      Window( HINSTANCE instance, WNDPROC wndproc, void* userdata );
      virtual ~Window();
      void create( const string& classname, const string& title, int x, int y, int w, int h );
      void messageLoop( Event& stopEvent );
    };

    class ConsoleWindow: public Window, public ConsoleListener, public enable_shared_from_this<ConsoleWindow> {
    public:
      struct LineEntry {
        wstring str;
        COLORREF clr;
      };
      using LineEntryVector = vector<LineEntry>;
    private:
      HWND log_;
      HWND cmdline_;
      float dpiScaling_;
      WNDPROC baseCmdlineProc_;
      WNDPROC baseLogProc_;
      ConsolePtr console_;
      LineEntryVector linesBuffer_;
      platform::RWLock lock_;
      HWND unpauseButton_;
      RECT logFit_;
      struct LogState {
        bool paused;
        bool selection;
        uint64_t linecount;
        uint64_t currentline;
        uint64_t firstvisibleline;
        LogState(): paused( false ), selection( false ), linecount( 0 ), currentline( 0 ), firstvisibleline( 0 ) {}
      } logState_;
      struct Autocomplete {
        CVarList matches; //!< Autocomplete matches vector
        ConBase* suggestion; //!< Autocomplete last suggestion
        string base; //!< Search string for autocomplete
        Autocomplete() { reset(); }
        void reset();
      } autocomplete_;
      struct History {
        StringVector stack; //!< Command history stack
        bool browsing; //!< Is the user browsing through the history?
        size_t position; //!< Position of currently located command
        History() { reset(); }
        void reset();
      } history_;
      void recheckLogState();
      void logPause();
      void logUnpause();
      static LRESULT CALLBACK wndProc( HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam );
      static LRESULT CALLBACK cmdlineProc( HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam );
      static LRESULT CALLBACK logProc( HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam );
      void initTextControl( HWND ctrl, bool lineinput );
      void paint( HWND wnd, HDC hdc, RECT& client );
      void paintUnpauseButton( HDC hdc, RECT& rc );
      void flushBuffer();
      void forwardExecute( const wstring& command );
    public:
      ConsoleWindow( ConsolePtr console, const string& title, int x, int y, int w, int h );
      void onConsolePrint( Console* console, vec3 color, const string& str ) override;
      void clearCmdline();
      void setCmdline( const string& line );
      void print( COLORREF color, const wstring& line );
      void print( const wstring& line );
      void print( const string& line );
      virtual ~ConsoleWindow();
    };

  }

}

#endif