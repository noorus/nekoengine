#pragma once
#include "neko_types.h"
#include "neko_exception.h"
//#include "consolelistener.h"
#include <windows.h>
#include <shlobj.h>


namespace neko {

  enum FileSeek {
    FileSeek_Beginning,
    FileSeek_Current,
    FileSeek_End
  };

  class FileReader {
  public:
    FileReader() {}
    virtual ~FileReader()
    {
    }
    virtual const uint64_t size() const = 0;
    virtual void read( void* out, uint32_t length ) = 0;
    virtual uint64_t readUint64() = 0;
    virtual uint32_t readUint32() = 0;
    virtual int readInt() = 0;
    virtual bool readBool() = 0;
    virtual Real readReal() = 0;
    virtual string readFullString() = 0;
    virtual void readFullVector( vector<uint8_t>& out ) = 0;
    virtual uint32_t seek( FileSeek direction, int32_t distance ) = 0;
    virtual uint32_t tell() = 0;
  };

  using FileReaderPtr = shared_ptr<FileReader>;

  enum FileDir {
    Dir_Fonts = 0,
    Dir_Textures,
    Dir_Meshes,
    Dir_Animations
  };

  class FileSystem {
  private:
    map<FileDir, wstring> rootDirs_;
  public:
    FileSystem();
    uint64_t fileStat( FileDir dir, const wstring& path );
    FileReaderPtr openFile( FileDir dir, const wstring& path );
    FileReaderPtr openFile( FileDir dir, const utf8String& path );
    FileReaderPtr openFile( const wstring& path );
    FileReaderPtr openFile( const utf8String& path );
  };

}