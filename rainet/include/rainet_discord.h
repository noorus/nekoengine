#pragma once

#include "rainet.h"
#include "discord/cpp/discord.h"

namespace rainet {

  struct DiscordUser {
    DcSnowflake id_ = 0;
    utf8String name_;
    utf8String discriminator_;
  };

  class DiscordState {
  public:
    DiscordUser localUser_;
    union Updated {
      struct Bits
      {
        uint8_t self : 1;
        uint8_t friends : 1;
        uint8_t images : 1;
      } bits;
      uint64_t field = 0;
    } updated_;
    map<DcSnowflake, DiscordUser> friends_;
    map<DcSnowflake, Image> userImages_;
  };

  class Discord {
  private:
    Host* host_;
    unique_ptr<discord::Core> core_;
    int64_t clientID_;
    int64_t appID_;
    DiscordState state_;
    GameInstallationState installation_;
  protected:
    void fetchImage( DiscordUser& user );
  public:
    Discord( int64_t clientID, int64_t applicationID, uint32_t steamAppID, Host* host );
    void initialize();
    void setActivityAlphaDevelop();
    inline const GameInstallationState& installation() throw() { return installation_; }
    void update( double gameTime, double delta );
    void shutdown();
    ~Discord();
  };

}