#pragma once
#include "neko_types.h"
#include "neko_exception.h"
//#include "consolelistener.h"
#include <windows.h>

namespace neko {

  namespace platform {

    void initialize();
    void prepareProcess();
    void shutdown();

    extern HINSTANCE g_instance;

    //! \class RWLock
    //! Reader-writer lock class for easy portability.
    class RWLock: noncopyable {
    protected:
      SRWLOCK mLock;
    public:
      RWLock() { InitializeSRWLock( &mLock ); }
      void lock() { AcquireSRWLockExclusive( &mLock ); }
      void unlock() { ReleaseSRWLockExclusive( &mLock ); }
      void lockShared() { AcquireSRWLockShared( &mLock ); }
      void unlockShared() { ReleaseSRWLockShared( &mLock ); }
    };

    class Event {
    private:
      HANDLE handle_;
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
      inline HANDLE get() const { return handle_; }
      ~Event()
      {
        if ( handle_ )
          CloseHandle( handle_ );
      }
    };

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

    class PerformanceTimer {
    private:
      LARGE_INTEGER frequency_;
      LARGE_INTEGER time_;
    public:
      PerformanceTimer()
      {
        QueryPerformanceFrequency( &frequency_ );
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

    class FileReader {
    protected:
      HANDLE file_;
      uint64_t size_;
    public:
      FileReader( const string& filename ): file_( INVALID_HANDLE_VALUE )
      {
        file_ = CreateFileA( filename.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
        if ( file_ == INVALID_HANDLE_VALUE )
          NEKO_EXCEPT( "File open failed" );

        DWORD sizeHigh = 0;
        DWORD sizeLow = GetFileSize( file_, &sizeHigh );

        size_ = ( (uint64_t)sizeHigh << 32 | sizeLow );

        SetFilePointer( file_, 0, NULL, FILE_BEGIN );
      }
      inline const uint64_t size() const { return size_; }
      ~FileReader()
      {
        if ( file_ != INVALID_HANDLE_VALUE )
          CloseHandle( file_ );
      }
      void read( void* out, uint32_t length )
      {
        DWORD read = 0;
        if ( ReadFile( file_, out, length, &read, nullptr ) != TRUE || read < length )
          NEKO_EXCEPT( "File read failed or length mismatch" );
      }
      uint64_t readUint64()
      {
        uint64_t ret;
        DWORD read = 0;
        if ( ReadFile( file_, &ret, sizeof( uint64_t ), &read, nullptr ) != TRUE || read < sizeof( uint64_t ) )
          NEKO_EXCEPT( "File read failed or length mismatch" );
        return ret;
      }
      uint32_t readUint32()
      {
        uint32_t ret;
        DWORD read = 0;
        if ( ReadFile( file_, &ret, sizeof( uint32_t ), &read, nullptr ) != TRUE || read < sizeof( uint32_t ) )
          NEKO_EXCEPT( "File read failed or length mismatch" );
        return ret;
      }
      int readInt()
      {
        int ret;
        DWORD read = 0;
        if ( ReadFile( file_, &ret, sizeof( int ), &read, nullptr ) != TRUE || read < sizeof( int ) )
          NEKO_EXCEPT( "File read failed or length mismatch" );
        return ret;
      }
      bool readBool()
      {
        uint8_t ret;
        DWORD read = 0;
        if ( ReadFile( file_, &ret, sizeof( uint8_t ), &read, nullptr ) != TRUE || read < sizeof( uint8_t ) )
          NEKO_EXCEPT( "File read failed or length mismatch" );
        return ( ret != 0 );
      }
      Real readReal()
      {
        Real ret;
        DWORD read = 0;
        if ( ReadFile( file_, &ret, sizeof( Real ), &read, nullptr ) != TRUE || read < sizeof( Real ) )
          NEKO_EXCEPT( "File read failed or length mismatch" );
        return ret;
      }
      string readFullString()
      {
        string ret;
        auto pos = SetFilePointer( file_, 0, 0, FILE_CURRENT );
        if ( pos == INVALID_SET_FILE_POINTER )
          NEKO_EXCEPT( "SetFilePointer failed" );
        uint32_t length = ( (uint32_t)size_ - pos );
        ret.resize( length );
        DWORD read = 0;
        if ( ReadFile( file_, (LPVOID)ret.data(), length, &read, nullptr ) != TRUE || read != length )
          NEKO_EXCEPT( "File read failed or length mismatch" );
        return ret;
      }
    };

    class FileWriter {
    protected:
      HANDLE file_;
    public:
      FileWriter( const string& filename, const bool append = false ): file_( INVALID_HANDLE_VALUE )
      {
        file_ = CreateFileA( filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
          append ? OPEN_ALWAYS : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );

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

    inline bool fileExists( const string& path )
    {
      DWORD attributes = GetFileAttributesA( path.c_str() );
      return ( attributes != INVALID_FILE_ATTRIBUTES && !( attributes & FILE_ATTRIBUTE_DIRECTORY ) );
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
    inline void outputDebugString( const char* str ) throw()
    {
      OutputDebugStringA( str );
    }

    //! Outputs a global debug string.
    inline void outputDebugString( const string& str ) throw()
    {
      outputDebugString( str.c_str() );
    }

    //! UTF-8 to wide string conversion.
    inline wstring utf8ToWide( const string& in ) throw()
    {
      int length = MultiByteToWideChar( CP_UTF8, 0, in.c_str(), -1, nullptr, 0 );
      if ( length == 0 )
        return wstring();
      vector<wchar_t> conversion( length );
      MultiByteToWideChar( CP_UTF8, 0, in.c_str(), -1, &conversion[0], length );
      return wstring( &conversion[0] );
    }

    //! Wide string to UTF-8 conversion.
    inline string wideToUtf8( const wstring& in ) throw( )
    {
      int length = WideCharToMultiByte( CP_UTF8, 0, in.c_str(), -1, nullptr, 0, 0, FALSE );
      if ( length == 0 )
        return string();
      vector<char> conversion( length );
      WideCharToMultiByte( CP_UTF8, 0, in.c_str(), -1, &conversion[0], length, 0, FALSE );
      return string( &conversion[0] );
    }

    //! Assign a thread name that will be visible in debuggers
    inline void setDebuggerThreadName( DWORD threadID, const std::string& threadName )
    {
#ifdef _DEBUG
#pragma pack( push, 8 )
      struct threadNamingStruct {
        DWORD type;
        LPCSTR name;
        DWORD threadID;
        DWORD flags;
      } nameSignalStruct;
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