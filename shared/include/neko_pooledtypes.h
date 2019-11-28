#pragma once

#include "neko_config.h"

#include <exception>
#include <memory>
#include <vector>

namespace neko {

  class PooledVectorObject {
  public:
    bool used_; //!< Whether this pooled object is in use.
    virtual void release() = 0;
  };

  template <class T>
  class PooledVector: public vector<T> {
  public:
    enum {
      PoolNone = numeric_limits<size_t>::max()
    };
    static const size_t pool_leadsCount = 10;
  private:
    size_t pool_recentLeads_[pool_leadsCount];
  public:
    PooledVector(): vector<T>()
    {
      pool_init();
    }
    inline void pool_init()
    {
      for ( size_t i = 0; i < pool_leadsCount; ++i )
        pool_recentLeads_[i] = PoolNone;
    }
    inline size_t pool_acquire()
    {
      for ( size_t i = 0; i < pool_leadsCount; ++i )
      {
        if ( pool_recentLeads_[i] != PoolNone )
        {
          auto rval = pool_recentLeads_[i]; // sorry, double usage
          pool_recentLeads_[i] = PoolNone;
          if ( !(*this)[rval].used_ )
          {
            (*this)[rval].used_ = true;
            return rval;
          }
        }
      }
      auto obj = T();
      obj.used_ = true;
      push_back( move( obj ) );
      return ( this->size() - 1 );
    }
    inline void pool_release( size_t index )
    {
      (*this)[index].release();
      (*this)[index].used_ = false;
      for ( size_t i = 0; i < pool_leadsCount; ++i )
        if ( pool_recentLeads_[i] == PoolNone )
        {
          pool_recentLeads_[i] = index;
          return;
        }
    }
  };

}