#pragma once
#include "neko_types.h"
#include "neko_exception.h"
//#include "consolelistener.h"
#include <windows.h>
#include <shlobj.h>

#ifndef NEKO_SIDELIBRARY_BUILD
// fucking ICU steals the priority of resolving resource.h directly
# include "../../engine/resource.h"
#endif

namespace neko {

  namespace platform {

#ifndef NEKO_SIDELIBRARY_BUILD

    namespace api {

      using fnGetDpiForSystem = UINT( WINAPI* )( VOID );
      using fnSetThreadDpiAwarenessContext = DPI_AWARENESS_CONTEXT( WINAPI* )( DPI_AWARENESS_CONTEXT );
      using fnSetThreadDescription = HRESULT( WINAPI* )( HANDLE hThread, PCWSTR lpThreadDescription );

      struct WinapiCalls
      {
        fnGetDpiForSystem pfnGetDpiForSystem = nullptr;
        fnSetThreadDpiAwarenessContext pfnSetThreadDpiAwarenessContext = nullptr;
        fnSetThreadDescription pfnSetThreadDescription = nullptr;
      };

      extern WinapiCalls g_calls;

      void initialize();

    }

    void initialize();
    void prepareProcess();
    void shutdown();

    // Performance stuff
    void performanceInitializeProcess();
    void performanceTeardownProcess();
    void performanceInitializeGameThread();
    void performanceInitializeRenderThread();
    void performanceInitializeLoaderThread();
    void performanceTeardownCurrentThread();

    extern HINSTANCE g_instance;

#endif

#pragma warning( push )
#pragma warning( disable : 26110 )

    //! \class RWLock
    //! Reader-writer lock class for easy portability.
    class RWLock {
    protected:
      SRWLOCK lock_;
    public:
      RWLock() { InitializeSRWLock( &lock_ ); }
      inline void lock() { AcquireSRWLockExclusive( &lock_ ); }
      inline void unlock() { ReleaseSRWLockExclusive( &lock_ ); }
      inline void lockShared() { AcquireSRWLockShared( &lock_ ); }
      inline void unlockShared() { ReleaseSRWLockShared( &lock_ ); }
    };

#pragma warning( pop )

    //! \class Event
    //! Native event type abstraction for the current platform.
    class Event {
    public:
      using NativeType = HANDLE;
    private:
      NativeType handle_;
    public:
      Event( bool initialState = false ): handle_( 0 )
      {
        handle_ = CreateEventW( nullptr, TRUE, initialState ? TRUE : FALSE, nullptr );
        if ( !handle_ )
          NEKO_EXCEPT( "Event creation failed" );
      }
      inline void set() { SetEvent( handle_ ); }
      inline void reset() { ResetEvent( handle_ ); }
      inline bool wait( uint32_t milliseconds = INFINITE ) const
      {
        return ( WaitForSingleObject( handle_, milliseconds ) == WAIT_OBJECT_0 );
      }
      inline bool check() const
      {
        return ( WaitForSingleObject( handle_, 0 ) == WAIT_OBJECT_0 );
      }
      inline NativeType get() const { return handle_; }
      ~Event()
      {
        if ( handle_ )
          CloseHandle( handle_ );
      }
    };

    using EventVector = vector<Event::NativeType>;

    //! Synchronously wait for one or all of native events to fire, or timeout.
    inline size_t waitForEvents( const EventVector& events, uint32_t milliseconds, bool waitAll = false, size_t timeoutValue = WAIT_TIMEOUT )
    {
      auto ret = static_cast<size_t>( WaitForMultipleObjects( (DWORD)events.size(), events.data(), waitAll, milliseconds > 0 ? milliseconds : INFINITE ) );
      if ( ret >= WAIT_OBJECT_0 && ret < ( WAIT_OBJECT_0 + events.size() ) )
        return ( ret - WAIT_OBJECT_0 );
      else if ( ret == WAIT_TIMEOUT )
        return timeoutValue;
      else
        NEKO_WINAPI_EXCEPT( "WaitForMultipleObjects failed" );
    }

    //! \class Thread
    //! Native platform-dependent threading implementation for dedicated worker usage.
    class Thread {
    public:
      using Callback = bool(*)( Event& running, Event& wantStop, void* argument );
    protected:
      HANDLE thread_;
      DWORD id_;
      string name_;
      Event run_;
      Event stop_;
      void* argument_;
      Callback callback_;
      static DWORD WINAPI threadProc( void* argument );
    public:
      Thread( const string& name, Callback callback, void* argument );
      virtual bool start();
      virtual void stop();
      //! Returns true if thread is alive within given time
      virtual bool waitFor( uint32_t milliseconds = INFINITE ) const;
      //! Returns true if thread is alive
      inline bool check() const { return waitFor( 0 ); }
      ~Thread();
    };

    //! \class SnapshotPainter
    //! \brief Stores an arbitrary bitmap image and can paint it resized onto another DC later.
    //!        Used to crudely scale last render of window contents when drag-sizing the render window.
    class SnapshotPainter {
    private:
      HDC dc_;
      HBITMAP hbitmap_;
      uint8_t* buffer_;
      size2i size_;
    public:
      SnapshotPainter(): dc_( nullptr ), hbitmap_( nullptr ), buffer_( nullptr ), size_() {}
      void reset()
      {
        if ( hbitmap_ )
        {
          DeleteObject( hbitmap_ );
          hbitmap_ = nullptr;
          buffer_ = nullptr;
        }
        if ( dc_ )
        {
          DeleteDC( dc_ );
          dc_ = nullptr;
        }
      }
      void store( HDC windowDC, size2i size, const vector<uint8_t>& data )
      {
        size_ = size;
        dc_ = CreateCompatibleDC( windowDC );
        BITMAPINFOHEADER header = { 0 };
        header.biSize = sizeof( BITMAPINFOHEADER );
        header.biWidth = (LONG)size_.w;
        header.biHeight = (LONG)size_.h;
        header.biPlanes = 1;
        header.biBitCount = 32;
        header.biCompression = BI_RGB;
        hbitmap_ = CreateDIBSection( windowDC, (BITMAPINFO*)( &header ), DIB_RGB_COLORS, (void**)&buffer_, nullptr, 0 );
        if ( !hbitmap_ || !buffer_ )
          NEKO_WINAPI_EXCEPT( "CreateDIBSection failed" );
        memcpy( buffer_, data.data(), data.size() );
      }
      void paint( HDC dc, size2i size )
      {
        if ( !hbitmap_ )
          return;
        auto oldbmp = SelectObject( dc_, hbitmap_ );
        StretchBlt(
          dc, 0, 0, (int)size.w, (int)size.h,
          dc_, 0, 0, (int)size_.w, (int)size_.h,
          SRCCOPY );
        SelectObject( dc_, oldbmp );
      }
      ~SnapshotPainter()
      {
        reset();
      }
    };

    class RenderWindowEventRecipient {
    public:
      virtual const Image& renderWindowReadPixels() = 0;
    };

    class RenderWindowHandler {
    private:
      size2i resolution_;
      float vaspect_;
      float haspect_;
      size2i borders_;
      HWND window_;
      WNDPROC originalProc_;
      void getWindowBordersSize( HWND window, size2i& size, bool withClient );
      void wmSizing( WPARAM wparam, LPARAM lparam );
      void wmEnterSizeMove();
      void wmExitSizeMove();
      bool wmPaint();
      bool resizing_;
      size2i lastSize_;
      void checkStartSizing();
      RenderWindowEventRecipient* target_;
      SnapshotPainter snapshotPainter_;
    public:
      RenderWindowHandler();
      inline HWND getWindow() const noexcept { return window_; }
      void changeTargetResolution( const size2i targetResolution );
      void setWindow( RenderWindowEventRecipient* callback, HWND window );
    private:
      static LRESULT wndProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );
      static RenderWindowHandler* instance_;
    public:
      static RenderWindowHandler& get()
      {
        if ( !instance_ )
          instance_ = new RenderWindowHandler();
        return *instance_;
      }
      static void free()
      {
        if ( instance_ )
        {
          delete instance_;
          instance_ = nullptr;
        }
      }
    };

    //! \class PerformanceTimer
    //! Native performance timer implementation for high-precision timing of tasks.
    class PerformanceTimer {
    private:
      LARGE_INTEGER frequency_;
      LARGE_INTEGER time_;
    public:
      PerformanceTimer(): time_({ 0, 0 }), frequency_({ 0, 0 })
      {
        if ( !QueryPerformanceFrequency( &frequency_ ) )
          NEKO_EXCEPT( "Couldn't query HPC frequency" );
      }
      void start()
      {
        QueryPerformanceCounter( &time_ );
      }
      double stop()
      {
        LARGE_INTEGER newTime;
        QueryPerformanceCounter( &newTime );
        auto delta = ( newTime.QuadPart - time_.QuadPart ) * 1000000;
        delta /= frequency_.QuadPart;
        double ms = (double)delta / 1000.0;
        return ms;
      }
    };

    //! \class PerformanceClock
    //! Native performance clock implementation for high-precision gameloop timing.
    class PerformanceClock {
    private:
      LARGE_INTEGER current_;
      GameTime frequency_;
      double inverse_;
    public:
      PerformanceClock(): current_({ 0, 0 })
      {
        LARGE_INTEGER frequency;
        if ( !QueryPerformanceFrequency( &frequency ) )
          NEKO_EXCEPT( "Couldn't query HPC frequency" );
        frequency_ = (GameTime)frequency.QuadPart;
        inverse_ = ( 1000000.0 / frequency.QuadPart );
      }
      inline void init()
      {
        QueryPerformanceCounter( &current_ );
      }
      inline GameTime update()
      {
        LARGE_INTEGER new_;
        QueryPerformanceCounter( &new_ );
        LONGLONG delta = ( new_.QuadPart - current_.QuadPart );
        current_ = new_;
        return ( (GameTime)delta / frequency_ );
      }
      inline uint64_t peekMicroseconds()
      {
        LARGE_INTEGER new_;
        QueryPerformanceCounter( &new_ );
        auto delta = ( new_.QuadPart - current_.QuadPart );
        return static_cast<uint64_t>( delta * inverse_ );
      }
    };

#ifndef NEKO_SIDELIBRARY_BUILD

    enum class KnownIcon
    {
      MainIcon,
      ConsoleIcon
    };

    inline HICON getIcon( KnownIcon icon )
    {
      HICON ld = nullptr;
      if ( icon == KnownIcon::MainIcon )
        ld = LoadIconW( g_instance, MAKEINTRESOURCEW( IDI_MAINICON ) );
      else if ( icon == KnownIcon::ConsoleIcon )
        ld = LoadIconW( g_instance, MAKEINTRESOURCEW( IDI_CONSOLEICON ) );
      return ld;
    }

    inline void setWindowIcon( HWND window, KnownIcon icon )
    {
      HICON ld = getIcon( icon );
      if ( ld )
      {
        SendMessageW( window, WM_SETICON, ICON_BIG, (LPARAM)ld );
        SendMessageW( window, WM_SETICON, ICON_SMALL, (LPARAM)ld );
      }
    }

#endif

    inline void createDirectory( const wstring& path )
    {
      wstring unicp = L"\\\\?\\" + path;
      if ( !CreateDirectoryW( unicp.c_str(), nullptr ) )
        NEKO_WINAPI_EXCEPT( "CreateDirectoryW failed" );
    }

    inline void ensureDirectory( const wstring& path )
    {
      auto attribs = ::GetFileAttributesW( path.c_str() );
      if ( attribs == INVALID_FILE_ATTRIBUTES || !( attribs & FILE_ATTRIBUTE_DIRECTORY ) )
      {
        createDirectory( path );
      }
    }

#ifdef _DEBUG
    const wchar_t c_gameName[] = L"nekoengineDebug";
#else
    const wchar_t c_gameName[] = L"nekoengineRetail";
#endif

    inline wstring getGameDocumentsPath()
    {
      PWSTR folder;
      if ( ::SHGetKnownFolderPath( FOLDERID_Documents, 0, nullptr, &folder ) != S_OK )
        NEKO_EXCEPT( "SHGetKnownFolderPath failed" );
      wchar_t fullpath[MAX_PATH];
      swprintf_s( fullpath, MAX_PATH, L"%s\\%s\\", folder, c_gameName );
      CoTaskMemFree( folder );
      return fullpath;
    }

    inline void setCurrentDirectory( const wstring& path )
    {
      ::SetCurrentDirectoryW( path.c_str() );
    }

    inline void sleep( uint32_t milliseconds )
    {
      ::SleepEx( milliseconds, TRUE );
    }

    inline bool getCursorPosition( const HWND window, POINT& position )
    {
      if ( !GetCursorPos( &position ) )
        return false;
      if ( !ScreenToClient( window, &position ) )
        return false;
      return true;
    }

    inline int64_t unixTimestamp()
    {
      constexpr int64_t c_unixTimeStart = 0x019DB1DED53E8000;
      constexpr int64_t c_ticksPerSecond = 10000000;

      FILETIME ft;
      GetSystemTimeAsFileTime( &ft );

      LARGE_INTEGER li;
      li.LowPart = ft.dwLowDateTime;
      li.HighPart = ft.dwHighDateTime;

      return ( li.QuadPart - c_unixTimeStart ) / c_ticksPerSecond;
    }

    //! UTF-8 to wide string conversion.
    inline wstring utf8ToWide( const utf8String& in ) noexcept
    {
      int length = MultiByteToWideChar( CP_UTF8, 0, in.c_str(), -1, nullptr, 0 );
      if ( length == 0 )
        return wstring();
      vector<wchar_t> conversion( length );
      MultiByteToWideChar( CP_UTF8, 0, in.c_str(), -1, &conversion[0], length );
      return wstring( &conversion[0] );
    }

    //! Wide string to UTF-8 conversion.
    inline utf8String wideToUtf8( const wstring& in ) noexcept
    {
      int length = WideCharToMultiByte( CP_UTF8, 0, in.c_str(), -1, nullptr, 0, 0, FALSE );
      if ( length == 0 )
        return utf8String();
      vector<char> conversion( length );
      WideCharToMultiByte( CP_UTF8, 0, in.c_str(), -1, &conversion[0], length, 0, FALSE );
      return utf8String( &conversion[0] );
    }

    //! Show a modal error dialog.
    inline void errorBox( string_view content, string_view title )
    {
      MessageBoxA( 0, content.data(), title.data(), MB_ICONERROR | MB_OK | MB_SYSTEMMODAL );
    }

    //! \class FileWriter
    //! Native generic file writer implementation.
    class FileWriter {
    protected:
      HANDLE file_;
    public:
      FileWriter( const utf8String& filename, const bool append = false ): FileWriter( utf8ToWide( filename ) ) {}
      FileWriter( const wstring& filename, const bool append = false ): file_( INVALID_HANDLE_VALUE )
      {
        file_ = CreateFileW( filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, append ? OPEN_ALWAYS : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );

        if ( file_ == INVALID_HANDLE_VALUE )
          NEKO_EXCEPT( "File creation failed" );

        if ( append )
          SetFilePointer( file_, 0, nullptr, FILE_END );
      }
      ~FileWriter()
      {
        if ( file_ != INVALID_HANDLE_VALUE )
          CloseHandle( file_ );
      }
      void writeInt( int value )
      {
        DWORD written = 0;
        auto ret = WriteFile( file_, &value, sizeof( int ), &written, nullptr );
        if ( !ret || written != sizeof( int ) )
          NEKO_EXCEPT( "Failed to write data" );
      }
      void writeUint64( uint64_t value )
      {
        DWORD written = 0;
        auto ret = WriteFile( file_, &value, sizeof( uint64_t ), &written, nullptr );
        if ( !ret || written != sizeof( uint64_t ) )
          NEKO_EXCEPT( "Failed to write data" );
      }
      void writeUint32( uint32_t value )
      {
        DWORD written = 0;
        auto ret = WriteFile( file_, &value, sizeof( uint32_t ), &written, nullptr );
        if ( !ret || written != sizeof( uint32_t ) )
          NEKO_EXCEPT( "Failed to write data" );
      }
      void writeReal( Real value )
      {
        DWORD written = 0;
        auto ret = WriteFile( file_, &value, sizeof( Real ), &written, nullptr );
        if ( !ret || written != sizeof( Real ) )
          NEKO_EXCEPT( "Failed to write data" );
      }
      void writeBlob( const void* buffer, uint32_t length )
      {
        DWORD written = 0;
        auto ret = WriteFile( file_, buffer, length, &written, nullptr );
        if ( !ret || written != length )
          NEKO_EXCEPT( "Failed to write data" );
      }
      void writeBool( bool value )
      {
        uint8_t val = ( value ? 1 : 0 );
        DWORD written = 0;
        auto ret = WriteFile( file_, &val, sizeof( uint8_t ), &written, nullptr );
        if ( !ret || written != sizeof( uint8_t ) )
          NEKO_EXCEPT( "Failed to write data" );
      }
    };

    //! Check whether a file exists at the specified path.
    //! A directory is not seen as a file.
    inline bool fileExists( const wstring& path )
    {
      DWORD attributes = GetFileAttributesW( path.c_str() );
      return ( attributes != INVALID_FILE_ATTRIBUTES && !( attributes & FILE_ATTRIBUTE_DIRECTORY ) );
    }

    //! Check whether a file exists at the specified path.
    //! A directory is not seen as a file.
    inline bool fileExists( const utf8String& path )
    {
      return fileExists( utf8ToWide( path ) );
    }

    //! Get a full path to the current directory.
    inline wstring getCurrentDirectory()
    {
      wchar_t currentDirectory[MAX_PATH];
      if ( !GetCurrentDirectoryW( MAX_PATH, currentDirectory ) )
        NEKO_EXCEPT( "Current directory fetch failed" );
      return currentDirectory;
    }

    //! Get current date and time.
    inline void getDateTime( DateTime& out )
    {
      SYSTEMTIME time;
      GetLocalTime( &time );
      out.year = time.wYear;
      out.month = time.wMonth;
      out.day = time.wDay;
      out.hour = time.wHour;
      out.minute = time.wMinute;
      out.second = time.wSecond;
    }

    //! Outputs a global debug string.
    inline void outputDebugString( const char* str ) noexcept
    {
      OutputDebugStringA( str );
    }

    //! Outputs a global debug string.
    inline void outputDebugString( const utf8String& str ) noexcept
    {
      OutputDebugStringW( utf8ToWide( str ).c_str() );
    }

    //! Assign a thread name that will be visible in debuggers.
    inline void setDebuggerThreadName( HANDLE thread, const utf8String& threadName )
    {
      // Prefer SetThreadDescription, but it is only available from Windows 10 version 1607 onwards.
      /*if ( api::g_calls.pfnSetThreadDescription )
      {
        api::g_calls.pfnSetThreadDescription( thread, utf8ToWide( threadName ).c_str() );
        return;
      }*/
#ifdef _DEBUG
      // Otherwise, back to the ancient trickery.
      auto threadID = GetThreadId( thread );
#pragma pack( push, 8 )
      struct threadNamingStruct {
        DWORD type;
        LPCSTR name;
        DWORD threadID;
        DWORD flags;
      } nameSignalStruct = { 0 };
#pragma pack( pop )
      nameSignalStruct.type = 0x1000;
      nameSignalStruct.name = threadName.c_str();
      nameSignalStruct.threadID = threadID;
      nameSignalStruct.flags = 0;
      __try
      {
        RaiseException( 0x406D1388, 0,
          sizeof( nameSignalStruct ) / sizeof( ULONG_PTR ),
          (const ULONG_PTR*)&nameSignalStruct );
      } __except ( EXCEPTION_EXECUTE_HANDLER ) {}
#endif
    }

  }

}