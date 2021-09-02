#pragma once
#include "neko_types.h"
#include "neko_platform.h"
#include "utilities.h"
#include "consolelistener.h"
#include "forwards.h"

namespace neko {

  //! \def NEKO_DECLARE_CONCMD(name,desc,cb)
  //! Declares a console command with a callback.
# define NEKO_DECLARE_CONCMD(name,desc,cb)\
  neko::ConCmd g_CVar_##name( #name, desc, cb )
  //! \def NEKO_DECLARE_CONVAR(name,desc,defval)
  //! Declares a console variable with a default value.
# define NEKO_DECLARE_CONVAR(name,desc,defval)\
  neko::ConVar g_CVar_##name( #name, desc, defval )
  //! \def NEKO_DECLARE_CONVAR_WITH_CB(name,desc,defval,cb)
  //! Declares a console variable with a default value and a callback function.
# define NEKO_DECLARE_CONVAR_WITH_CB(name,desc,defval,cb)\
  neko::ConVar g_CVar_##name( #name, desc, defval, cb )
  //! \def NEKO_EXTERN_CONCMD(name)
  //! Externs the declaration of a console command.
# define NEKO_EXTERN_CONCMD(name)\
  extern neko::ConCmd g_CVar_##name
  //! \def NEKO_EXTERN_CONVAR(name)
  //! Externs the declaration of a console variable.
# define NEKO_EXTERN_CONVAR(name)\
  extern neko::ConVar g_CVar_##name

  //! \class ConBase
  //! Base class for console commands & variables.
  //! \sa Console
  class ConBase {
    friend class Console;
  protected:
    string name_;        //!< Name of the command/variable
    string description_; //!< Short description for the command/variable
    bool registered_;    //!< Is the command/variable registered
    ConBase( const string& name, const string& description );
    //! Called when this object is registered in a Console.
    virtual void onRegister();
  public:
    virtual bool isCommand() = 0;
    virtual const string& name() { return name_; }
    virtual const string& description() { return description_; }
    virtual const bool isRegistered() { return registered_; }
  };

  //! A list of console variables and commands.
  using CVarList = list<ConBase*>;

  //! \class ConCmd
  //! A console command.
  class ConCmd: public ConBase {
  public:
    using Callback = void(*)( Console* console, ConCmd* command, StringVector& arguments );
  protected:
    Callback callback_; //!< Callback function on execution
  public:
    ConCmd( const string& name, const string& description, Callback callback );
    virtual void call( Console* console, StringVector& arguments );
    bool isCommand() override { return true; }
  };

  //! \class ConVar
  //! A console variable.
  class ConVar: public ConBase {
  public:
    struct Value {
      int i;
      float f;
      string str;
    };
    using Callback = bool(*)( ConVar* variable, Value oldValue );
  protected:
    Value value_; //!< Current value
    Value default_; //!< Default value
    Callback callback_; //!< Callback function on value change
  public:
    ConVar( const string& name, const string& description,
      int defaultValue, Callback callback = nullptr );
    ConVar( const string& name, const string& description,
      float defaultValue, Callback callback = nullptr );
    ConVar( const string& name, const string& description,
      const string& defaultValue, Callback callback = nullptr );
    bool isCommand() override { return false; }
    virtual int as_i() const { return value_.i; } //!< Value as integer
    virtual float as_f() const { return value_.f; } //!< Value as float
    virtual const string& as_s() const { return value_.str; } //!< Value as string
    virtual bool as_b() const { return ( as_i() > 0 ); } //!< Value as boolean
    virtual void set( int value );
    virtual void set( float value );
    virtual void set( const string& value );
    virtual void forceSet( int value );
    virtual void forceSet( float value );
    virtual void forceSet( const string& value );
    virtual void toggle();
  };

  using ConCmdPtr = unique_ptr<ConCmd>;

  //! A console message source.
  struct ConsoleSource {
    string name;
    vec3 color;
  };

  class Console: public enable_shared_from_this<Console> {
    friend class ConBase;
  public:
    //! Message source types.
    enum Source: unsigned long {
      srcError = 0,   //!< Error message
      srcEngine,      //!< Message from the engine
      srcGfx,         //!< Message from the graphics subsystem
      srcSound,       //!< Message from the sound subsystem
      srcLoader,      //!< Message from the loader subsystem
      srcScripting,   //!< Message from the scripting subsystem
      srcInput,       //!< Message from the input subsystem
      srcGame,        //!< Message from the game logic
      srcGUI          //!< Message from the gui subsystem
    };
  private:
    unique_ptr<TextFileWriter> fileOut_;
    CVarList cvars_; //!< Registered commands & variables
    static CVarList precreated_; //!< Pre-created commands & variables
    platform::RWLock lock_; //!< Execution lock
    platform::RWLock listenerLock_; //!< Listener add/remove/callback lock
    platform::RWLock bufferLock_; //!< Command buffer lock
    set<ConsoleListener*> listeners_;
    StringVector bufferedCommands_;
    ConCmdPtr listCmd_;
    ConCmdPtr helpCmd_;
    ConCmdPtr findCmd_;
    ConCmdPtr execCmd_;
    EnginePtr engine_;
    map<Source, ConsoleSource> sources_;
    void writeStartBanner( const EngineInfo& info );
    void writeStopBanner();
    //! Registers a console variable or command.
    void registerVariable( ConBase* var );
    static StringVector tokenize( const string& commandLine );
    static void callbackList( Console* console, ConCmd* command, StringVector& arguments );
    static void callbackHelp( Console* console, ConCmd* command, StringVector& arguments );
    static void callbackFind( Console* console, ConCmd* command, StringVector& arguments );
    static void callbackExec( Console* console, ConCmd* command, StringVector& arguments );
  public:
    Console();
    void setEngine( EnginePtr engine );
    void resetEngine();
    //! Describes the given console command or variable.
    void describe( ConBase* base );
    //! Adds a listener.
    void addListener( ConsoleListener* listener );
    //! Removes a listener.
    void removeListener( ConsoleListener* listener );
    //! Registers a message source.
    Source registerSource( const string& name, vec3 color );
    //! Unregisters a message source.
    void unregisterSource( Source source );
    //! Automatic completion search for given command line.
    void autoComplete( const string& line, CVarList& matches );
    void start( const EngineInfo& info, const Environment& env );
    void stop();
    void queueCommand( const string& commandLine );
    //! Queues a command for execution on next update call.
    void executeBuffered();
    //! Prints a message.
    void print( Source source, const char* str );
    //! Prints a message.
    inline void print( Source source, const utf8String& str ) { print( source, str.c_str() ); }
    //! Prints a C-style formatted message.
    void printf( Source source, const char* str, ... );
    //! Prints an error message.
    void errorPrintf( const char* str, ... );
    //! Executes a command line.
    void execute( string commandLine, const bool echo = true );
    //! Executes a file.
    void executeFile( const string& filename );
    //! Gets a variable or command.
    ConVar* getVariable( const string_view name );
    ConCmd* getCommand( const string_view name );
  };

}