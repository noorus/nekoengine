#pragma once

#include "neko_types.h"
#include "neko_exception.h"
#include "neko_platform.h"

namespace neko {

  // This is Windows-specific, should be a slightly different implementation for other platforms

  class filepath {
  private:
    std::filesystem::path internal_;
  public:
    filepath(): internal_() {}
    filepath( const std::filesystem::path& other )
    {
      static_assert( std::is_same_v<std::filesystem::path::string_type, std::wstring> );
      internal_ = other;
    }
    filepath( wstring source )
    {
      static_assert( std::is_same_v<std::filesystem::path::string_type, std::wstring> );
      std::replace( source.begin(), source.end(), L'/', L'\\' );
      internal_ = std::filesystem::path( source );
    }
    filepath( const wchar_t* source ): filepath( wstring( source ) ) {}
    filepath( const utf8String& source ): filepath( platform::utf8ToWide( source ) ) {}
    bool operator<( const filepath& rhs ) const
    {
      return ( internal_ < rhs.internal_ );
    }
    filepath( HANDLE file )
    {
      DWORD ret = MAX_PATH + 1;
      vector<wchar_t> buffer;
      while ( ret > buffer.size() )
      {
        buffer.resize( static_cast<size_t>( ret ), L'\0' );
        ret = GetFinalPathNameByHandleW( file, buffer.data(), static_cast<DWORD>( buffer.size() ), FILE_NAME_NORMALIZED | VOLUME_NAME_DOS );
        if ( !ret )
          NEKO_WINAPI_EXCEPT( "GetFinalPathNameByHandleW" );
      }
      internal_ = std::filesystem::path( buffer.data() );
    }
    static filepath normalizeFrom( const wstring& path_, const wstring& directory_ )
    {
      filepath absolute;
      filepath path( path_ );
      if ( path.isAbsolute() )
        absolute = path;
      else
      {
        if ( !directory_.empty() && directory_[directory_.size() - 1] != L'\\' )
          absolute = filepath( directory_ + L'\\' + path_ );
        else
          absolute = filepath( directory_ + path_ );
      }
      absolute.canonicalize();
      return absolute;
    }
    static filepath normalizeFrom( const filepath& path, const filepath& directory )
    {
      return normalizeFrom( path.wide(), directory.wide() );
    }
    filepath& operator /=( const filepath& rhs )
    {
      internal_ /= rhs.internal_;
      return *this;
    }
    filepath& operator +=( const filepath& rhs )
    {
      internal_ += rhs.internal_;
      return *this;
    }
    filepath operator + ( const filepath& rhs )
    {
      filepath cpy( internal_ );
      cpy += rhs;
      return cpy;
    }
    filepath operator / ( const filepath& rhs )
    {
      filepath cpy( internal_ );
      cpy /= rhs;
      return cpy;
    }
    inline void append( const filepath& rhs ) { internal_ /= rhs.internal_; }
    inline void concat( const filepath& rhs ) { internal_ += rhs.internal_; }
    inline wstring wide() const { return internal_.wstring(); }
    inline utf8String utf8() const { return platform::wideToUtf8( internal_.wstring() ); }
    inline void canonicalize() { internal_ = std::filesystem::weakly_canonical( internal_ ); }
    inline filepath canonicalized() const { return filepath( std::filesystem::weakly_canonical( internal_ ) ); }
    inline bool isRelative() const { return internal_.is_relative(); }
    inline bool isAbsolute() const { return internal_.is_absolute(); }
    inline bool empty() const { return internal_.empty(); }
    inline bool hasFilename() const { return internal_.has_filename(); }
    inline bool hasExtension() const { return internal_.has_extension(); }
    inline filepath filename() const
    {
      if ( !hasFilename() )
        return {};
      return internal_.filename();
    }
    inline bool hasDirectory() const { return internal_.has_parent_path(); }
    inline filepath directory() const
    {
      if ( !internal_.has_parent_path() )
        NEKO_EXCEPT( "Path has no directory" );
      return internal_.parent_path();
    }
  };

}