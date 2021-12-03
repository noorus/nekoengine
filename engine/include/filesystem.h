#pragma once
#include "neko_types.h"
#include "neko_exception.h"
//#include "consolelistener.h"
#include <windows.h>
#include <shlobj.h>


namespace neko {

  class FileReader
  {
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
  };

  using FileReaderPtr = shared_ptr<FileReader>;

  class FileSystem
  {
  public:
    FileSystem() {}
    FileReaderPtr openFile( const wstring& path );
    FileReaderPtr openFile( const utf8String& path );
  };

}