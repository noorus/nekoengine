#pragma once
#include "utilities.h"
#include "subsystem.h"
#include "forwards.h"
#include "neko_types.h"

namespace neko {

  template <typename T>
  class LoadedResourceManagerBase;

  template <typename T>
  class LoadedResourceBase: public nocopy {
  public:
    using Ptr = shared_ptr<T>;
  protected:
    const utf8String name_;
    bool loaded_ = false;
  public:
    LoadedResourceBase() = delete;
    LoadedResourceBase( const utf8String& name ):
    name_( name )
    {
      assert( !name_.empty() );
      // OutputDebugStringA( utils::ilprinf( "Resource created: %s\r\n", name_.c_str() ).c_str() );
    }
    ~LoadedResourceBase()
    {
      // OutputDebugStringA( utils::ilprinf( "Resource freed: %s\r\n", name_.c_str() ).c_str() );
    }
    inline const utf8String& name() const { return name_; }
    inline bool loaded() const { return loaded_; }
  };

  template <typename T>
  class LoadedResourceManagerBase: public nocopy {
  public:
    using ResourcePtr = typename T::Ptr;
    using MapType = map<utf8String, ResourcePtr>;
  protected:
    ThreadedLoaderPtr loader_;
    MapType map_;
  public:
    LoadedResourceManagerBase() = delete;
    LoadedResourceManagerBase( ThreadedLoaderPtr loader ):
    loader_( move( loader ) )
    {
      //
    }
    inline const T* get( const utf8String& name ) const
    {
      auto it = map_.find( name );
      if ( it == map_.end() )
        return nullptr;
      return ( ( *it ).second.get() );
    }
    inline ResourcePtr getPtr( const utf8String& name ) const
    {
      auto it = map_.find( name );
      if ( it == map_.end() )
        return {};
      return ( ( *it ).second );
    }
    inline const MapType& items() const
    {
      return map_;
    }
  };

}