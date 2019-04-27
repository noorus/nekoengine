#include "stdafx.h"
#include "console.h"
#include "utilities.h"
#include "neko_exception.h"

// don't ask
#pragma warning(disable: 4073)
#pragma init_seg(lib)

namespace neko {

  const char* c_fileLogFormat = "%06d [%02d:%02d] %s\r\n";
  const int c_sprintfBufferSize = 1024;

  CVarList Console::precreated_;

  // ConBase

  ConBase::ConBase( const string& name, const string& description ):
    name_( name ), description_( description ), registered_( false )
  {
    assert( !name_.empty() && !description_.empty() );

    Console::precreated_.push_back( this );
  }

  void ConBase::onRegister()
  {
    registered_ = true;
  }

  // ConCmd

  ConCmd::ConCmd( const string& name, const string& description,
    ConCmd::Callback callback ):
    ConBase( name, description ), callback_( callback )
  {
    assert( callback_ );
  }

  void ConCmd::call( Console* console, StringVector& arguments )
  {
    callback_( console, this, arguments );
  }

  // ConVar

  ConVar::ConVar( const string& name, const string& description,
    int defaultValue, ConVar::Callback callback ):
    ConBase( name, description ), callback_( callback )
  {
    set( defaultValue );
    default_ = value_;
  }

  ConVar::ConVar( const string& name, const string& description,
    float defaultValue, ConVar::Callback callback ):
    ConBase( name, description ), callback_( callback )
  {
    set( defaultValue );
    default_ = value_;
  }

  ConVar::ConVar( const string& name, const string& description,
    const string& defaultValue, ConVar::Callback callback ):
    ConBase( name, description ), callback_( callback )
  {
    set( defaultValue );
    default_ = value_;
  }

  void ConVar::set( int value )
  {
    Value oldValue = value_;
    value_.i = value;
    value_.f = (float)value;
    char strtmp[32];
    sprintf_s( strtmp, 32, "%i", value );
    value_.str = strtmp;
    if ( !registered_ || !callback_ || callback_( this, oldValue ) )
      return;
    value_ = oldValue;
  }

  void ConVar::set( float value )
  {
    Value oldValue = value_;
    value_.i = (int)value;
    value_.f = value;
    char strtmp[32];
    sprintf_s( strtmp, 32, "%f", value );
    value_.str = strtmp;
    if ( !registered_ || !callback_ || callback_( this, oldValue ) )
      return;
    value_ = oldValue;
  }

  void ConVar::set( const string& value )
  {
    Value oldValue = value_;
    value_.i = atoi( value.c_str() );
    value_.f = (float)atof( value.c_str() );
    value_.str = value;
    if ( !registered_ || !callback_ || callback_( this, oldValue ) )
      return;
    value_ = oldValue;
  }

  void ConVar::forceSet( int value )
  {
    value_.i = value;
    value_.f = (float)value;
    char strtmp[32];
    sprintf_s( strtmp, 32, "%i", value );
    value_.str = strtmp;
  }

  void ConVar::forceSet( float value )
  {
    value_.i = (int)value;
    value_.f = value;
    char strtmp[32];
    sprintf_s( strtmp, 32, "%f", value );
    value_.str = strtmp;
  }

  void ConVar::forceSet( const string& value )
  {
    value_.i = atoi( value.c_str() );
    value_.f = (float)atof( value.c_str() );
    value_.str = value;
  }

  // TextFile

  struct DumbDate {
    uint16_t year;    //!< Year
    uint16_t month;   //!< Month
    uint16_t day;     //!< Day
    uint16_t hour;    //!< Hour
    uint16_t minute;  //!< Minute
    uint16_t second;  //!< Second
  };

  void getDateTime( DumbDate& out )
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

  Console::Console()
  {
    listCmd_ = std::make_unique<ConCmd>( "list", "List all cvars.", callbackList );
    helpCmd_ = std::make_unique<ConCmd>( "help", "Get help on a variable/command.", callbackHelp );
    findCmd_ = std::make_unique<ConCmd>( "find", "List cvars with name containing given string.", callbackFind );
    execCmd_ = std::make_unique<ConCmd>( "exec", "Execute a configuration file.", callbackExec );

    for ( ConBase* var : precreated_ )
      registerVariable( var );

    precreated_.clear();

    cvars_.sort( []( ConBase* a, ConBase* b ) -> bool {
      return ( _stricmp( a->name().c_str(), b->name().c_str() ) <= 0 );
    } );
  }

  void Console::describe( ConBase* base )
  {
    printf( "%s: (%s) - %s",
      base->name().c_str(),
      base->isCommand() ? "command" : "variable",
      base->description().c_str() );
  }

  void Console::callbackList( Console* console, ConCmd* command, StringVector& arguments )
  {
    for ( auto base : console->cvars_ )
      console->describe( base );
  }

  void Console::callbackHelp( Console* console, ConCmd* command, StringVector& arguments )
  {
    if ( arguments.size() < 2 )
    {
      console->print( "Format: help <variable/command>" );
      return;
    }

    for ( auto base : console->cvars_ )
    {
      if ( boost::iequals( base->name(), arguments[1] ) )
      {
        console->describe( base );
        return;
      }
    }

    console->printf( R"(Error: Unknown command "%s")", arguments[1].c_str() );
  }

  void Console::callbackFind( Console* console, ConCmd* command, StringVector& arguments )
  {
    if ( arguments.size() < 2 )
    {
      console->print( "Format: find <text>" );
      return;
    }
    using StringRange = const boost::iterator_range<string::const_iterator>;
    for ( auto base : console->cvars_ )
    {
      for ( size_t i = 1; i < arguments.size(); i++ )
      {
        if ( boost::ifind_first(
          StringRange( base->name().begin(), base->name().end() ),
          StringRange( arguments[i].begin(), arguments[i].end() ) ) )
        {
          console->describe( base );
          break;
        }
      }
    }
  }

  void Console::callbackExec( Console* console, ConCmd* command, StringVector& arguments )
  {
    if ( arguments.size() < 2 )
    {
      console->print( "Format: exec <filename>" );
      return;
    }

    console->executeFile( arguments[1] );
  }

  void Console::registerVariable( ConBase* var )
  {
    ScopedRWLock lock( &lock_ );

    auto it = std::find( cvars_.begin(), cvars_.end(), var );
    if ( it != cvars_.end() || var->isRegistered() )
      NEKO_EXCEPT( "CVAR has already been registered" );

    var->onRegister();
    cvars_.push_back( var );
  }

  void Console::gameBegin()
  {
    DumbDate now;
    getDateTime( now );

    fileOut_.reset();

    char filename[64];
    sprintf_s( filename, 64, "%04d%02d%02d_hivemind_game-%02d%02d%02d.log",
      now.year, now.month, now.day, now.hour, now.minute, now.second );

    fileOut_ = make_unique<TextFileWriter>( filename );

    writeStartBanner();
  }

  void Console::writeStartBanner()
  {
    DumbDate now;
    getDateTime( now );

    printf( "HIVEMIND StarCraft II AI [Debug Log]" );
    printf( "Game starting on %04d-%02d-%02d %02d:%02d:%02d",
      now.year, now.month, now.day, now.hour, now.minute, now.second );
  }

  void Console::writeStopBanner()
  {
    DumbDate now;
    getDateTime( now );

    printf( "Game ending on %04d-%02d-%02d %02d:%02d:%02d",
      now.year, now.month, now.day, now.hour, now.minute, now.second );
  }

  void Console::addListener( ConsoleListenerPtr listener )
  {
    ScopedRWLock lock( &listenerLock_ );
    listeners_.insert( listener );
  }

  void Console::removeListener( ConsoleListenerPtr listener )
  {
    ScopedRWLock lock( &listenerLock_ );
    listeners_.erase( listener );
  }

  void Console::autoComplete( const string& line, CVarList& matches )
  {
    matches.clear();
    auto trimmed = boost::trim_copy( line );
    for ( ConBase* base : cvars_ )
    {
      auto comparison = base->name().substr( 0, trimmed.length() );
      if ( boost::iequals( trimmed, comparison ) )
        matches.push_back( base );
    }
  }

  void Console::print( const char* str )
  {
    auto ticks = 0;
    auto realTime = 0; // utils::ticksToTime( ticks );
    unsigned int minutes = ( realTime / 60 );
    unsigned int seconds = ( realTime % 60 );

    char fullbuf[c_sprintfBufferSize + 128];
    sprintf_s( fullbuf, c_sprintfBufferSize + 128, c_fileLogFormat, ticks, minutes, seconds, str );

    if ( fileOut_ )
      fileOut_->write( fullbuf );

    listenerLock_.lockShared();
    for ( auto listener : listeners_ )
    {
      listener->onConsolePrint( this, fullbuf );
    }
    listenerLock_.unlockShared();

#ifdef _DEBUG
    OutputDebugStringA( fullbuf );
#endif
  }

  void Console::printf( const char* str, ... )
  {
    va_list va_alist;
    char buffer[c_sprintfBufferSize];
    va_start( va_alist, str );
    _vsnprintf_s( buffer, c_sprintfBufferSize, str, va_alist );
    va_end( va_alist );

    print( buffer );
  }

  StringVector Console::tokenize( const string& str )
  {
    // this implementation is naïve, but hardly critical

    bool quoted = false;
    bool escaped = false;

    string buffer;
    StringVector v;

    for ( char chr : str )
    {
      if ( chr == BACKSLASH )
      {
        if ( escaped )
          buffer.append( 1, chr );
        escaped = !escaped;
      } else if ( chr == SPACE )
      {
        if ( !quoted )
        {
          if ( !buffer.empty() )
          {
            v.push_back( buffer );
            buffer.clear();
          }
        } else
          buffer.append( 1, chr );
        escaped = false;
      } else if ( chr == QUOTE )
      {
        if ( escaped )
        {
          buffer.append( 1, chr );
          escaped = false;
        } else
        {
          if ( quoted )
          {
            if ( !buffer.empty() )
            {
              v.push_back( buffer );
              buffer.clear();
            }
          }
          quoted = !quoted;
        }
      } else
      {
        buffer.append( 1, chr );
        escaped = false;
      }
    }

    if ( !buffer.empty() )
      v.push_back( buffer );

    return v;
  }

  void Console::execute( string commandLine, const bool echo )
  {
    boost::trim( commandLine );
    if ( commandLine.empty() )
      return;

    auto arguments = tokenize( commandLine );
    if ( arguments.empty() )
      return;

    if ( echo )
      printf( "> %s", commandLine.c_str() );

    ScopedRWLock lock( &lock_ );

    auto command = arguments[0];
    for ( auto base : cvars_ )
    {
      if ( !base->isRegistered() )
        continue;
      if ( boost::iequals( base->name(), command ) )
      {
        if ( base->isCommand() )
        {
          auto cmd = static_cast<ConCmd*>( base );
          lock.unlock();
          cmd->call( this, arguments );
        } else
        {
          auto var = static_cast<ConVar*>( base );
          if ( arguments.size() > 1 )
            var->set( arguments[1] );
          else
          {
            lock.unlock();
            printf( R"(%s is "%s")",
              var->name().c_str(),
              var->as_s().c_str() );
          }
        }
        return;
      }
    }

    lock.unlock();

    printf( R"(Unknown command "%s")", command.c_str() );
  }

  void Console::gameEnd()
  {
    writeStopBanner();

    fileOut_.reset();
  }

  void Console::queueCommand( const string& commandLine )
  {
    ScopedRWLock lock( &bufferLock_ );
    bufferedCommands_.push_back( commandLine );
  }

  void Console::executeBuffered()
  {
    bufferLock_.lockShared();
    for ( auto& cmd : bufferedCommands_ )
      execute( cmd, true );
    bufferLock_.unlockShared();
    bufferLock_.lock();
    bufferedCommands_.clear();
    bufferLock_.unlock();
  }

  void Console::executeFile( const string& filename )
  {
    printf( "Executing %s", filename.c_str() );

    if ( !platform::fileExists( filename ) )
    {
      printf( R"(Error: Cannot execute "%s", file does not exist)", filename.c_str() );
      return;
    }

    platform::FileReader reader( filename );
    auto str = reader.readFullString();
    const char* content = str.c_str();

    size_t i = 0;

    const BYTE utf8BOM[3] = { 0xEF, 0xBB, 0xBF };
    if ( !memcmp( content, utf8BOM, 3 ) )
      i = 3;

    // naive
    string line;
    line.reserve( 128 );
    for ( i; i < str.length(); ++i )
    {
      if ( content[i] != 0 && content[i] != LF && content[i] != CR )
      {
        line.append( 1, content[i] );
      } else if ( content[i] == LF )
      {
        if ( !line.empty() )
          execute( line, true );
        line.clear();
      } else if ( content[i] == 0 )
        break;
    }

    if ( !line.empty() )
      execute( line, true );
  }

}