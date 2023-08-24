#include "pch.h"
#include "console.h"
#include "utilities.h"
#include "neko_exception.h"
#include "neko_algorithm.h"
#include "engine.h"
#include "filesystem.h"

// Don't ask
// Don't touch
#pragma warning( disable : 4073 )
#pragma init_seg( lib )

namespace neko {

  const char* c_fileLogFormat = "[%0#*.2f][%s] %s\r\n";
  const int c_consolePrintfBufferSize = 16384;

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

  ConCmd::ConCmd( const string& name, const string& description, ConCmd::Callback callback ):
    ConBase( name, description ), callback_( callback )
  {
    assert( callback_ );
  }

  void ConCmd::call( Console* console, StringVector& arguments )
  {
    callback_( console, this, arguments );
  }

  // ConVar

  ConVar::ConVar( const string& name, const string& description, int defaultValue, ConVar::Callback callback ):
    ConBase( name, description ), callback_( callback )
  {
    set( defaultValue );
    default_ = value_;
  }

  ConVar::ConVar( const string& name, const string& description, float defaultValue, ConVar::Callback callback ):
    ConBase( name, description ), callback_( callback )
  {
    set( defaultValue );
    default_ = value_;
  }

  ConVar::ConVar( const string& name, const string& description, const string& defaultValue, ConVar::Callback callback ):
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

  void ConVar::toggle()
  {
    set( ( value_.i > 0 ) ? 0 : 1 );
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
    return { (Real)r / 255.0f, (Real)g / 255.0f, (Real)b / 255.0f };
  }

  Console::Console()
  {
    // Register default sources
    registerSource( "error", rgbToVec3( 222, 37, 56 ) );
    registerSource( "engine", rgbToVec3( 60, 64, 76 ) );
    registerSource( "gfx", rgbToVec3( 79, 115, 44 ) );
    registerSource( "sound", rgbToVec3( 181, 80, 10 ) );
    registerSource( "loader", rgbToVec3( 78, 29, 153 ) );
    registerSource( "scripts", rgbToVec3( 34, 70, 197 ) );
    registerSource( "input", rgbToVec3( 219, 38, 122 ) );
    registerSource( "game", rgbToVec3( 4, 127, 77 ) );
    registerSource( "gui", rgbToVec3( 79, 115, 44 ) );
    registerSource( "messaging", rgbToVec3( 79, 115, 44 ) );
    registerSource( "steam", rgbToVec3( 79, 115, 44 ) );

    // Create core commands
    listCmd_ = make_unique<ConCmd>( "list", "List all cvars.", callbackList );
    helpCmd_ = make_unique<ConCmd>( "help", "Get help on a variable/command.", callbackHelp );
    findCmd_ = make_unique<ConCmd>( "find", "List cvars with name containing given string.", callbackFind );
    execCmd_ = make_unique<ConCmd>( "exec", "Execute a configuration file.", callbackExec );

    for ( ConBase* var : precreated_ )
      registerVariable( var );

    precreated_.clear();

    cvars_.sort(
      []( ConBase* a, ConBase* b ) -> bool { return ( _stricmp( a->name().c_str(), b->name().c_str() ) <= 0 ); } );
  }

  LogSource Console::registerSource( const string& name, vec3 color )
  {
    ScopedRWLock lock( &lock_ );
    ConsoleSource tmp = { name, color };
    auto index = static_cast<LogSource>( sources_.size() );
    sources_[index] = tmp;
    return index;
  }

  void Console::unregisterSource( LogSource source )
  {
    ScopedRWLock lock( &lock_ );
    sources_.erase( source );
  }

  void Console::describe( ConBase* base )
  {
    printf( srcEngine, "%s: (%s) - %s", base->name().c_str(), base->isCommand() ? "command" : "variable",
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
    start( engine_->info(), engine_->env() );
  }

  void Console::resetEngine()
  {
    if ( engine_ )
    {
      stop();
      engine_.reset();
    }
  }

  void Console::start( const EngineInfo& info, const Environment& env )
  {
    DateTime now;
    platform::getDateTime( now );

    fileOut_.reset();

    wchar_t filename[64];
    swprintf_s( filename, 64, L"%04d%02d%02d_%S-%02d%02d%02d.log", now.year, now.month, now.day, info.logName.c_str(),
      now.hour, now.minute, now.second );

    fileOut_ = make_unique<TextFileWriter>( filepath::normalizeFrom( filename, env.documentsPath_ ).wide() );

    writeStartBanner( info );
  }

  void Console::writeStartBanner( const EngineInfo& info )
  {
    DateTime now;
    platform::getDateTime( now );

    printf( srcEngine, "%s v%d.%d.%d [%s]", info.engineName.c_str(), info.major, info.minor, info.build, info.profile.c_str() );
    printf( srcEngine, "Build: %s (%s)", info.compiled.c_str(), info.compiler.c_str() );
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

  thread_local char tls_consolePrintfBuffer[c_consolePrintfBufferSize];
  thread_local char tls_consolePrintBuffer[c_consolePrintfBufferSize + 32];

  void Console::print( LogSource source, const char* str )
  {
    auto& src = sources_[source];
    auto time = ( engine_ ? (float)engine_->time() : 0.0f );

    sprintf_s( tls_consolePrintBuffer, c_consolePrintfBufferSize + 32, c_fileLogFormat, 8, time, src.name.c_str(), str );

    if ( fileOut_ )
      fileOut_->write( tls_consolePrintBuffer );

    listenerLock_.lockShared();
    for ( auto listener : listeners_ )
      listener->onConsolePrint( this, src.color, tls_consolePrintBuffer );
    listenerLock_.unlockShared();

#ifdef _DEBUG
    platform::outputDebugString( tls_consolePrintBuffer );
#endif
  }

  void Console::printf( LogSource source, const char* str, ... )
  {
    va_list va_alist;
    va_start( va_alist, str );
    _vsnprintf_s( tls_consolePrintfBuffer, c_consolePrintfBufferSize, str, va_alist );
    va_end( va_alist );

    print( source, tls_consolePrintfBuffer );
  }

  void Console::errorPrintf( const char* str, ... )
  {
    va_list va_alist;
    va_start( va_alist, str );
    _vsnprintf_s( tls_consolePrintfBuffer, c_consolePrintfBufferSize, str, va_alist );
    va_end( va_alist );

    utf8String fullStr = "Fatal: ";
    fullStr.append( tls_consolePrintfBuffer );
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
        }
        else
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
            printf( srcEngine, R"(%s is "%s")", var->name().c_str(), var->as_s().c_str() );
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

    TextFileReader reader( Locator::fileSystem().openFile( Dir_User, filename ) );
    const auto source = reader.readFullAssumeUtf8();
    stringstream ss( source );

    utf8String line;
    while ( std::getline( ss, line ) )
      if ( !line.empty() )
        execute( line, true );
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