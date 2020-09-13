#include "stdafx.h"
#include "console.h"
#include "utilities.h"
#include "neko_exception.h"
#include "neko_algorithm.h"
#include "engine.h"

// Used in the log filename and in the log startup banner.
#define NEKO_LOGNAME "nekoengine"
#define NEKO_LOGTITLE "Nekoengine Alpha"

// Don't ask
#pragma warning(disable: 4073)
#pragma init_seg(lib)

namespace neko {

  const char* c_fileLogFormat = "[%0#*.2f][%s] %s\r\n";
  const int c_sprintfBufferSize = 8192;

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

  constexpr vec3 rgbToVec3( uint8_t r, uint8_t g, uint8_t b )
  {
    return vec3( (Real)r / 255.0f, (Real)g / 255.0f, (Real)b / 255.0f );
  }

  Console::Console()
  {
    // Register default sources
    registerSource( "error", rgbToVec3( 222, 37, 56 ) );
    registerSource( "engine", rgbToVec3( 60, 64, 76 ) );
    registerSource( "gfx", rgbToVec3( 79, 115, 44 ) );
    registerSource( "sound", rgbToVec3( 181, 80, 10 ) );
    registerSource( "physics", rgbToVec3( 78, 29, 153 ) );
    registerSource( "scripts", rgbToVec3( 34, 70, 197 ) );
    registerSource( "input", rgbToVec3( 219, 38, 122 ) );
    registerSource( "game", rgbToVec3( 4, 127, 77 ) );
    registerSource( "gui", rgbToVec3( 79, 115, 44 ) );

    // Create core commands
    listCmd_ = make_unique<ConCmd>( "list", "List all cvars.", callbackList );
    helpCmd_ = make_unique<ConCmd>( "help", "Get help on a variable/command.", callbackHelp );
    findCmd_ = make_unique<ConCmd>( "find", "List cvars with name containing given string.", callbackFind );
    execCmd_ = make_unique<ConCmd>( "exec", "Execute a configuration file.", callbackExec );

    for ( ConBase* var : precreated_ )
      registerVariable( var );

    precreated_.clear();

    cvars_.sort( []( ConBase* a, ConBase* b ) -> bool {
      return ( _stricmp( a->name().c_str(), b->name().c_str() ) <= 0 );
    });
  }

  Console::Source Console::registerSource( const string& name, vec3 color )
  {
    ScopedRWLock lock( &lock_ );
    ConsoleSource tmp = { name, color };
    auto index = (Console::Source)sources_.size();
    sources_[index] = tmp;
    return index;
  }

  void Console::unregisterSource( Source source )
  {
    ScopedRWLock lock( &lock_ );
    sources_.erase( source );
  }

  void Console::describe( ConBase* base )
  {
    printf( srcEngine, "%s: (%s) - %s",
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
      console->print( srcEngine, "Format: help <variable/command>" );
      return;
    }

    for ( auto base : console->cvars_ )
    {
      if ( algorithm::iequals( base->name(), arguments[1] ) )
      {
        console->describe( base );
        return;
      }
    }

    console->printf( srcEngine, R"(Error: Unknown command "%s")", arguments[1].c_str() );
  }

  void Console::callbackFind( Console* console, ConCmd* command, StringVector& arguments )
  {
    if ( arguments.size() < 2 )
    {
      console->print( srcEngine, "Format: find <text>" );
      return;
    }
    for ( auto base : console->cvars_ )
    {
      for ( size_t i = 1; i < arguments.size(); i++ )
      {
        if ( base->name().find( arguments[i] ) != utf8String::npos )
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
      console->print( srcEngine, "Format: exec <filename>" );
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


  void Console::setEngine( EnginePtr engine )
  {
    if ( !engine )
      return resetEngine();
    engine_ = move( engine );
    start();
  }

  void Console::resetEngine()
  {
    if ( engine_ )
    {
      stop();
      engine_.reset();
    }
  }

  void Console::start()
  {
    DateTime now;
    platform::getDateTime( now );

    fileOut_.reset();

    char filename[64];
    sprintf_s( filename, 64, "%04d%02d%02d_" NEKO_LOGNAME "-%02d%02d%02d.log",
      now.year, now.month, now.day, now.hour, now.minute, now.second );

    fileOut_ = make_unique<TextFileWriter>( filename );

    writeStartBanner();
  }

  void Console::writeStartBanner()
  {
    DateTime now;
    platform::getDateTime( now );

    printf( srcEngine, NEKO_LOGTITLE " [Debug Log]" );
    printf( srcEngine, "Starting on %04d-%02d-%02d %02d:%02d:%02d",
      now.year, now.month, now.day, now.hour, now.minute, now.second );
  }

  void Console::writeStopBanner()
  {
    DateTime now;
    platform::getDateTime( now );

    printf( srcEngine, "Stopping on %04d-%02d-%02d %02d:%02d:%02d",
      now.year, now.month, now.day, now.hour, now.minute, now.second );
  }

  void Console::addListener( ConsoleListener* listener )
  {
    ScopedRWLock lock( &listenerLock_ );
    listeners_.insert( listener );
  }

  void Console::removeListener( ConsoleListener* listener )
  {
    ScopedRWLock lock( &listenerLock_ );
    listeners_.erase( listener );
  }

  void Console::autoComplete( const string& line, CVarList& matches )
  {
    matches.clear();
    auto trimmed = algorithm::trim_copy( line );
    for ( ConBase* base : cvars_ )
    {
      auto comparison = base->name().substr( 0, trimmed.length() );
      if ( algorithm::iequals( trimmed, comparison ) )
        matches.push_back( base );
    }
  }

  void Console::print( Source source, const char* str )
  {
    auto &src = sources_[source];
    auto time = ( engine_ ? (float)engine_->time() : 0.0f );

    char fullbuf[c_sprintfBufferSize + 128];
    sprintf_s( fullbuf, c_sprintfBufferSize + 128, c_fileLogFormat, 8, time, src.name.c_str(), str );

    if ( fileOut_ )
      fileOut_->write( fullbuf );

    listenerLock_.lockShared();
    for ( auto listener : listeners_ )
      listener->onConsolePrint( this, src.color, fullbuf );
    listenerLock_.unlockShared();

#ifdef _DEBUG
    platform::outputDebugString( fullbuf );
#endif
  }

  void Console::printf( Source source, const char* str, ... )
  {
    va_list va_alist;
    char buffer[c_sprintfBufferSize];
    va_start( va_alist, str );
    _vsnprintf_s( buffer, c_sprintfBufferSize, str, va_alist );
    va_end( va_alist );

    print( source, buffer );
  }

  void Console::errorPrintf( const char* str, ... )
  {
    va_list va_alist;
    char buffer[c_sprintfBufferSize];
    va_start( va_alist, str );
    _vsnprintf_s( buffer, c_sprintfBufferSize, str, va_alist );
    va_end( va_alist );

    utf8String fullStr = "Fatal: ";
    fullStr.append( buffer );
    utf8String tmp;
    tmp.reserve( 1024 );
    for ( size_t i = 0; i <= fullStr.size(); ++i )
    {
      if ( i == fullStr.size() || fullStr[i] == LF )
      {
        if ( !tmp.empty() )
          print( srcError, tmp );
        if ( i == fullStr.size() )
          break;
        tmp.clear();
      }
      else if ( fullStr[i] != CR )
        tmp.push_back( fullStr[i] );
    }
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
      }
      else if ( chr == SPACE )
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
      }
      else if ( chr == QUOTE )
      {
        if ( escaped )
        {
          buffer.append( 1, chr );
          escaped = false;
        }
        else
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
      }
      else
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
    algorithm::trim( commandLine );
    if ( commandLine.empty() )
      return;

    auto arguments = tokenize( commandLine );
    if ( arguments.empty() )
      return;

    if ( echo )
      printf( srcEngine, "> %s", commandLine.c_str() );

    ScopedRWLock lock( &lock_ );

    auto command = arguments[0];
    for ( auto base : cvars_ )
    {
      if ( !base->isRegistered() )
        continue;
      if ( algorithm::iequals( base->name(), command ) )
      {
        if ( base->isCommand() )
        {
          auto cmd = static_cast<ConCmd*>( base );
          lock.unlock();
          cmd->call( this, arguments );
        }
        else
        {
          auto var = static_cast<ConVar*>( base );
          if ( arguments.size() > 1 )
            var->set( arguments[1] );
          else
          {
            lock.unlock();
            printf( srcEngine, R"(%s is "%s")",
              var->name().c_str(),
              var->as_s().c_str() );
          }
        }
        return;
      }
    }

    lock.unlock();

    printf( srcEngine, R"(Unknown command "%s")", command.c_str() );
  }

  void Console::stop()
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
    printf( srcEngine, "Executing %s", filename.c_str() );

    if ( !platform::fileExists( filename ) )
    {
      printf( srcEngine, R"(Error: Cannot execute "%s", file does not exist)", filename.c_str() );
      return;
    }

#ifdef NEKO_NO_ICU
    NEKO_EXCEPT( "Cannot execute a console file because ICU was not compiled in." );
#else

    TextFileReader reader( filename );
    auto str = unicodePiece( reader.readFullAssumeUtf8().c_str() );
    auto content = unicodeString::fromUTF8( str );

    // FIXME This will just crash and burn if there's a > ASCII character in the Unicode string.
    string line;
    line.reserve( 128 );
    for ( int32_t i = 0; i < content.length(); ++i )
    {
      if ( content[i] != 0 && content[i] != LF && content[i] != CR )
      {
        line.append( 1, (const char)content[i] );
      }
      else if ( content[i] == LF )
      {
        if ( !line.empty() )
          execute( line, true );
        line.clear();
      }
      else if ( content[i] == 0 )
        break;
    }

    if ( !line.empty() )
      execute( line, true );

#endif
  }

  ConVar* Console::getVariable( const string_view name )
  {
    for ( auto& base : cvars_ )
    {
      if ( !base->isRegistered() )
        continue;
      if ( algorithm::iequals( name, string_view( base->name() ) ) )
        return ( base->isCommand() ? nullptr : (ConVar*)base );
    }

    return nullptr;
  }

  ConCmd* Console::getCommand( const string_view name )
  {
    for ( auto& base : cvars_ )
    {
      if ( !base->isRegistered() )
        continue;
      if ( algorithm::iequals( name, string_view( base->name() ) ) )
        return ( base->isCommand() ? (ConCmd*)base : nullptr );
    }

    return nullptr;
  }

}