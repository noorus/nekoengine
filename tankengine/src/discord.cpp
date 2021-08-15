#include "stdafx.h"
#include "discord.h"
#include "neko_exception.h"

namespace tank {

  struct Snowflake {
  private:
    Snowflake() {}
  public:
    union
    {
      struct SnowflakeComponents
      {
        uint64_t timestamp : 42;
        uint8_t worker : 5;
        uint8_t process : 5;
        uint16_t increment : 12;
      } m_components;
      uint64_t m_full;
    };
    explicit Snowflake( uint64_t val ): m_full( val ) {}
    explicit Snowflake( uint64_t stamp, uint8_t worker_, uint8_t proc_, uint16_t incr )
    {
      m_components.timestamp = stamp;
      m_components.worker = worker_;
      m_components.process = proc_;
      m_components.increment = incr;
    }
    inline utf8String toString()
    {
      char buf[24];
      sprintf_s( buf, 24, "%020llu", m_full );
      return utf8String( buf );
    }
  };

  inline Snowflake makeSnowflake()
  {
    Snowflake f( neko::platform::unixTimestamp(), 123, 456, 7 );
    return move( f );
  }

  void translateUser( discord::User const* from, DiscordUser& to )
  {
    to.id_ = from->GetId();
    to.name_ = from->GetUsername();
    to.discriminator_ = from->GetDiscriminator();
  }

  void Discord::fetchImage( DiscordUser& user )
  {
    discord::ImageHandle request{};
    request.SetId( user.id_ );
    request.SetType( discord::ImageType::User );
    request.SetSize( 256 );
    core_->ImageManager().Fetch( request, true,
    [&]( discord::Result result, discord::ImageHandle handle )
    {
      discord::ImageDimensions dims{};
      core_->ImageManager().GetDimensions( handle, &dims );
      DiscordImage img;
      img.width_ = dims.GetWidth();
      img.height_ = dims.GetHeight();
      img.buffer_.reserve( img.width_ * img.height_ * 4 );
      core_->ImageManager().GetData( handle, img.buffer_.data(), img.buffer_.size() );
      state_.userImages_[user.id_] = move( img );
      state_.updated_.bits.images = true;
      host_->onDiscordUserImage( user.id_, dims.GetWidth(), dims.GetHeight(), state_.userImages_[user.id_].buffer_.data() );
    });
  }

  Discord::Discord( int64_t clientID, int64_t applicationID, uint32_t steamAppID, TankHost* host ):
  clientID_( clientID ), appID_( applicationID ), host_( host )
  {
    SetEnvironmentVariableW( L"DISCORD_INSTANCE_ID", L"0" );

    discord::Core* core = nullptr;
    auto result = discord::Core::Create( clientID_, DiscordCreateFlags_Default, &core );
    if ( result != discord::Result::Ok || !core )
      NEKO_EXCEPT( "Discord core create failed" );

    core_.reset( core );

    core_->SetLogHook( discord::LogLevel::Debug,
    [=]( discord::LogLevel level, const char* message )
    {
      host_->onDiscordDebugPrint( message );
    } );

    //core_->ActivityManager().RegisterCommand( "" );
    if ( steamAppID )
      core_->ActivityManager().RegisterSteam( steamAppID );

    // Current user update callback handler
    core_->UserManager().OnCurrentUserUpdate.Connect( [&]()
    {
      discord::User user;
      core_->UserManager().GetCurrentUser( &user );
      translateUser( &user, state_.localUser_ );
      state_.updated_.bits.self = true;
      fetchImage( state_.localUser_ );
    });

    // Refresh callback handler
    core_->RelationshipManager().OnRefresh.Connect( [&]()
    {
      core_->RelationshipManager().Filter( [&]( discord::Relationship const& relationship ) -> bool
      {
        return ( relationship.GetType() == discord::RelationshipType::Friend
          && !relationship.GetUser().GetBot()
          && relationship.GetUser().GetId() != state_.localUser_.id_ );
      });
      state_.friends_.clear();
      int32_t count = 0;
      core_->RelationshipManager().Count( &count );
      for ( auto i = 0; i < count; ++i )
      {
        discord::Relationship relation;
        core_->RelationshipManager().GetAt( i, &relation );
        DiscordUser user;
        translateUser( &relation.GetUser(), user );
        state_.friends_[user.id_] = move( user );
      }
      state_.updated_.bits.friends = true;
    });

    // Relationship update callback handler
    core_->RelationshipManager().OnRelationshipUpdate.Connect(
    [&]( discord::Relationship const& relationship )
    {
      if ( relationship.GetType() != discord::RelationshipType::Friend
      && state_.friends_.find( relationship.GetUser().GetId() ) != state_.friends_.end() )
      {
        state_.friends_.erase( relationship.GetUser().GetId() );
        return;
      }
      DiscordUser user;
      translateUser( &relationship.GetUser(), user );
      state_.friends_[user.id_] = move( user );
      state_.updated_.bits.friends = true;
    });
  }

  void Discord::initialize()
  {
  }

  void Discord::shutdown()
  {
    core_->ActivityManager().ClearActivity( []( discord::Result result )
    {
    });
  }

  void Discord::setActivityAlphaDevelop()
  {
    auto now = neko::platform::unixTimestamp();

    discord::Activity activity{};
    activity.SetType( discord::ActivityType::Playing );
    activity.SetDetails( "Alpha Test" );
    activity.GetTimestamps().SetStart( now );
    activity.SetState( "Debugging" );

    auto partyFlake = makeSnowflake();

    auto& party = activity.GetParty();
    party.SetId( partyFlake.toString().c_str() );
    party.GetSize().SetCurrentSize( 1 );
    party.GetSize().SetMaxSize( 1 );

    activity.SetInstance( false );

    core_->ActivityManager().UpdateActivity( activity, []( discord::Result result )
    {
    } );
  }

  void Discord::update( double gameTime, double delta )
  {
    auto result = core_->RunCallbacks();
    if ( result == discord::Result::NotRunning )
      return;

    if ( result != discord::Result::Ok )
      NEKO_EXCEPT( "Discord callbacks run failed" );

    char asd[1024];

    if ( state_.updated_.field )
    {
      if ( state_.updated_.bits.self )
      {
        sprintf_s( asd, 1024, "self: %s#%s (%I64u)", state_.localUser_.name_.c_str(), state_.localUser_.discriminator_.c_str(), state_.localUser_.id_ );
        host_->onDiscordDebugPrint( asd );
      }
      if ( state_.updated_.bits.friends )
      {
        for ( auto& frnd : state_.friends_ )
        {
          sprintf_s( asd, 1024, "friend: %s#%s (%I64u)", frnd.second.name_.c_str(), frnd.second.discriminator_.c_str(), frnd.second.id_ );
          host_->onDiscordDebugPrint( asd );
        }
      }
      state_.updated_.field = 0;
    }
  }

  Discord::~Discord()
  {
    core_.reset();
  }

}