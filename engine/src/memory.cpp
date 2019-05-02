#include "stdafx.h"
#include "memory.h"

namespace neko {

  Memory::Memory()
  {
    //
  }

  Memory::~Memory()
  {
    //
  }

  void* Memory::alloc( const Sector sector, size_t size, size_t alignment /* = 0Ui64 */ )
  {
    return ::HeapAlloc( GetProcessHeap(), NULL, size );
  }

  void* Memory::allocZeroed( const Sector sector, size_t size, size_t alignment /* = 0Ui64 */ )
  {
    return ::HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, size );
  }

  void* Memory::realloc( const Sector sector, void* location, size_t size, size_t alignment /* = 0Ui64 */ )
  {
    return ::HeapReAlloc( GetProcessHeap(), NULL, location, size );
  }

  void Memory::free( const Sector sector, void* location )
  {
    ::HeapFree( GetProcessHeap(), NULL, location );
  }

}