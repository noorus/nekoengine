#include "pch.h"

#ifndef NEKO_NO_GUI

#include "MyGUI/MyGUI_DataFileStream.h"
#include "FileSystemInfo.h"

#include "utilities.h"
#include "neko_platform.h"
#include "MyGUI_NekoPlatform.h"

namespace MyGUI {

  IDataStream* NekoDataManager::getData( const utf8String& name ) const
  {
    utf8String filepath = dataPath_ + "\\" + name;

    std::ifstream* stream = new std::ifstream();
    stream->open( filepath.c_str(), std::ios_base::binary );

    if ( !stream->is_open() )
      NEKO_EXCEPT( "Couldn't open file" );

    auto data = new DataFileStream( stream );

    return data;
  }

  void NekoDataManager::freeData( IDataStream* data )
  {
    delete data;
  }

  bool NekoDataManager::isDataExist( const utf8String& name ) const
  {
    const VectorString& files = getDataListNames( name );
    return !files.empty();
  }

  const VectorString& NekoDataManager::getDataListNames( const utf8String& pattern ) const
  {
    static VectorString result;
    common::VectorWString wresult;
    result.clear();

    common::scanFolder( wresult, neko::platform::utf8ToWide( dataPath_ ), false, MyGUI::UString( pattern ).asWStr(), false );

    for ( common::VectorWString::const_iterator item = wresult.begin(); item != wresult.end(); ++item )
    {
      result.push_back( MyGUI::UString( *item ).asUTF8() );
    }

    return result;
  }

  void NekoDataManager::setDataPath( const utf8String& path )
  {
    dataPath_ = path;
  }

  const std::string& NekoDataManager::getDataPath( const utf8String& name ) const
  {
    NEKO_EXCEPT( "getDataPath called" );
    static const std::string empty = "";
    return empty;
  }

}

#endif