#pragma once
#include "forwards.h"

namespace neko {

  class Memory {
  public:
    enum class Sector {
      Generic,
      Graphics,
      Audio,
      Scripting
    };
  public:
    Memory();
    ~Memory();
    void* alloc( const Sector sector, size_t size, size_t alignment = 0Ui64 );
    void* allocZeroed( const Sector sector, size_t size, size_t alignment = 0Ui64 );
    void* realloc( const Sector sector, void* location, size_t size, size_t alignment = 0Ui64 );
    void free( const Sector sector, void* location );
  };

}