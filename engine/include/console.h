#pragma once
#include "neko_types.h"
#include "neko_platform.h"
#include "utilities.h"
#include "consolelistener.h"

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
  };

  using ConCmdPtr = unique_ptr<ConCmd>;

  class Console {
    friend class ConBase;
  private:
    unique_ptr<TextFileWriter> fileOut_;
    CVarList cvars_; //!< Registered commands & variables
    static CVarList precreated_; //!< Pre-created commands & variables
    platform::RWLock lock_; //!< Execution lock
    platform::RWLock listenerLock_; //!< Listener add/remove/callback lock
    platform::RWLock bufferLock_; //!< Command buffer lock
    set<ConsoleListenerPtr> listeners_;
    StringVector bufferedCommands_;
    ConCmdPtr listCmd_;
    ConCmdPtr helpCmd_;
    ConCmdPtr findCmd_;
    ConCmdPtr execCmd_;
    void writeStartBanner();
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
    void describe( ConBase* base );
    void addListener( ConsoleListenerPtr listener );
    void removeListener( ConsoleListenerPtr listener );
    void autoComplete( const string& line, CVarList& matches );
    void gameBegin();
    void gameEnd();
    void queueCommand( const string& commandLine );
    void executeBuffered();
    void print( const char* str );
    inline void print( const string& str ) { print( str.c_str() ); }
    void printf( const char* str, ... );
    void execute( string commandLine, const bool echo = true );
    void executeFile( const string& filename );
  };

}