#include "stdafx.h"
#include "discord.h"
#include "neko_exception.h"

namespace tank {

  void translateUser( discord::User const* from, DiscordUser& to )
  {
    to.id_ = from->GetId();
    to.name_ = from->GetUsername();
    to.discriminator_ = from->GetDiscriminator();
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
      host_->logPrint( message );
    });

    core_->UserManager().OnCurrentUserUpdate.Connect( [&]()
    {
      discord::User user;
      core_->UserManager().GetCurrentUser( &user );
      translateUser( &user, state_.localUser_ );
      state_.updated_.bits.self = true;
    });

    //core_->ActivityManager().RegisterCommand( "" );
    core_->ActivityManager().RegisterSteam( steamAppID );

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

    core_->RelationshipManager().OnRelationshipUpdate.Connect( [&]( discord::Relationship const& relationship )
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
    discord::Activity activity{};
    activity.SetDetails( "Testing" );
    activity.SetState( "Partying" );
    activity.SetInstance( false );
    activity.SetType( discord::ActivityType::Playing );
    core_->ActivityManager().UpdateActivity( activity, []( discord::Result result )
    {
    });
  }

  void Discord::shutdown()
  {
    core_->ActivityManager().ClearActivity( []( discord::Result result )
    {
    });
  }

  void Discord::update()
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
        sprintf_s( asd, 1024, "discord: self: %s#%s (%I64u)", state_.localUser_.name_.c_str(), state_.localUser_.discriminator_.c_str(), state_.localUser_.id_ );
        host_->logPrint( asd );
      }
      if ( state_.updated_.bits.friends )
      {
        for ( auto& frnd : state_.friends_ )
        {
          sprintf_s( asd, 1024, "discord: friend: %s#%s (%I64u)", frnd.second.name_.c_str(), frnd.second.discriminator_.c_str(), frnd.second.id_ );
          host_->logPrint( asd );
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