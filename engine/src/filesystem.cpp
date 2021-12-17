#include "stdafx.h"

#include "neko_platform_windows.h"
#include "neko_exception.h"
#include "messaging.h"
#include "locator.h"
#include "console.h"
#include "filesystem.h"

namespace neko {

  //! \class FileReader
  //! Native generic file reader implementation.
  class LocalFilesystemReader: public FileReader {
  protected:
    HANDLE file_;
    uint64_t size_;

  public:
    LocalFilesystemReader( const wstring& filename ): file_( INVALID_HANDLE_VALUE )
    {
      file_ = CreateFileW( filename.c_str(),
        GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING,
        // Practically all of our file reads are sequential for now,
        // so let Windows know and optimize prefetch for that:
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        0 );

      if ( file_ == INVALID_HANDLE_VALUE )
        NEKO_WINAPI_EXCEPT( "File open failed" );

      DWORD sizeHigh = 0;
      DWORD sizeLow = GetFileSize( file_, &sizeHigh );

      size_ = ( (uint64_t)sizeHigh << 32 | sizeLow );

      SetFilePointer( file_, 0, nullptr, FILE_BEGIN );
    }
    inline const uint64_t size() const { return size_; }
    virtual ~LocalFilesystemReader()
    {
      if ( file_ != INVALID_HANDLE_VALUE )
        CloseHandle( file_ );
    }
    virtual void read( void* out, uint32_t length )
    {
      DWORD read = 0;
      if ( ReadFile( file_, out, length, &read, nullptr ) != TRUE || read < length )
        NEKO_EXCEPT( "File read failed or length mismatch" );
    }
    virtual uint64_t readUint64()
    {
      uint64_t ret = 0;
      DWORD read = 0;
      if ( ReadFile( file_, &ret, sizeof( uint64_t ), &read, nullptr ) != TRUE || read < sizeof( uint64_t ) )
        NEKO_EXCEPT( "File read failed or length mismatch" );
      return ret;
    }
    virtual uint32_t readUint32()
    {
      uint32_t ret = 0;
      DWORD read = 0;
      if ( ReadFile( file_, &ret, sizeof( uint32_t ), &read, nullptr ) != TRUE || read < sizeof( uint32_t ) )
        NEKO_EXCEPT( "File read failed or length mismatch" );
      return ret;
    }
    virtual int readInt()
    {
      int ret = 0;
      DWORD read = 0;
      if ( ReadFile( file_, &ret, sizeof( int ), &read, nullptr ) != TRUE || read < sizeof( int ) )
        NEKO_EXCEPT( "File read failed or length mismatch" );
      return ret;
    }
    virtual bool readBool()
    {
      uint8_t ret = 0;
      DWORD read = 0;
      if ( ReadFile( file_, &ret, sizeof( uint8_t ), &read, nullptr ) != TRUE || read < sizeof( uint8_t ) )
        NEKO_EXCEPT( "File read failed or length mismatch" );
      return ( ret != 0 );
    }
    virtual Real readReal()
    {
      Real ret = 0.0f;
      DWORD read = 0;
      if ( ReadFile( file_, &ret, sizeof( Real ), &read, nullptr ) != TRUE || read < sizeof( Real ) )
        NEKO_EXCEPT( "File read failed or length mismatch" );
      return ret;
    }
    virtual string readFullString()
    {
      string ret;
      auto pos = SetFilePointer( file_, 0, 0, FILE_CURRENT );
      if ( pos == INVALID_SET_FILE_POINTER )
        NEKO_EXCEPT( "SetFilePointer failed" );
      uint32_t length = ( (uint32_t)size_ - pos );
      ret.resize( length );
      DWORD read = 0;
      if ( ReadFile( file_, (LPVOID)ret.data(), (DWORD)length, &read, nullptr ) != TRUE || read != length )
        NEKO_EXCEPT( "File read failed or length mismatch" );
      return move( ret );
    }
    virtual void readFullVector( vector<uint8_t>& out )
    {
      out.resize( size_ );
      DWORD read = 0;
      if ( ReadFile( file_, (LPVOID)out.data(), (DWORD)size_, &read, nullptr ) != TRUE || read != size_ )
        NEKO_EXCEPT( "File read failed or length mismatch" );
    }
    virtual uint32_t seek( FileSeek direction, int32_t distance )
    {
      auto dir = ( direction == FileSeek_Beginning ? FILE_BEGIN
        : direction == FileSeek_Current ? FILE_CURRENT
        : FILE_END );
      auto ret = SetFilePointer( file_, distance, nullptr, dir );
      if ( ret == INVALID_SET_FILE_POINTER )
        NEKO_WINAPI_EXCEPT( "SetFilePointer (seek) failed" );
      return ret;
    }
    virtual uint32_t tell()
    {
      auto ptr = static_cast<uint32_t>( SetFilePointer( file_, 0, nullptr, FILE_CURRENT ) );
      if ( ptr == INVALID_SET_FILE_POINTER )
        NEKO_WINAPI_EXCEPT( "SetFilePointer (tell) failed" );
      return ptr;
    }
  };

  /*


  const char c_fontsBaseDirectory[] = ;
  const char c_texturesBaseDirectory[] = R"(assets\textures\)";
  const char c_meshesBaseDirectory[] = R"(assets\meshes\)";
  const char c_animationsBaseDirectory[] = R"(assets\animations\)";
  */
  /*
      Dir_Fonts = 0,
    Dir_Textures,
    Dir_Meshes,
    Dir_Animations
    */

  FileSystem::FileSystem()
  {
    rootDirs_[Dir_Fonts] = LR"(assets\fonts\)";
    rootDirs_[Dir_Textures] = LR"(assets\textures\)";
    rootDirs_[Dir_Meshes] = LR"(assets\meshes\)";
    rootDirs_[Dir_Animations] = LR"(assets\animations\)";
  }

  uint64_t FileSystem::fileStat( FileDir dir, const wstring& path )
  {
    wstring fixedpath = rootDirs_[dir];
    fixedpath.append( path );
    if ( !platform::fileExists( fixedpath ) )
      return 0;
    struct _stat64i32 fst;
    if ( _wstat( fixedpath.c_str(), &fst ) == 0 )
      return fst.st_size;
    return 0;
  }

  FileReaderPtr FileSystem::openFile( FileDir dir, const wstring& path )
  {
    wstring fixedpath = rootDirs_[dir];
    fixedpath.append( path );
    return make_shared<LocalFilesystemReader>( fixedpath );
  }

  FileReaderPtr FileSystem::openFile( FileDir dir, const utf8String& path )
  {
    return openFile( dir, platform::utf8ToWide( path ) );
  }

  FileReaderPtr FileSystem::openFile( const wstring& path )
  {
    return make_shared<LocalFilesystemReader>( path );
  }

  FileReaderPtr FileSystem::openFile( const utf8String& path )
  {
    return openFile( platform::utf8ToWide( path ) );
  }

}