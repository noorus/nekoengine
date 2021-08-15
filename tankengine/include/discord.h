#pragma once

#include "tankengine.h"
#include "discord/cpp/discord.h"

namespace tank {

  struct DiscordImage {
    size_t width_;
    size_t height_;
    std::vector<uint8_t> buffer_;
  };

  struct DiscordUser {
    uint64_t id_;
    utf8String name_;
    utf8String discriminator_;
  };

  using DcSnowflake = uint64_t;

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
      uint64_t field;
    } updated_;
    map<DcSnowflake, DiscordUser> friends_;
    map<DcSnowflake, DiscordImage> userImages_;
  };

  class Discord {
  private:
    TankHost* host_;
    unique_ptr<discord::Core> core_;
    int64_t clientID_;
    int64_t appID_;
    DiscordState state_;
    GameInstallationState installation_;
  protected:
    void fetchImage( DiscordUser& user );
  public:
    Discord( int64_t clientID, int64_t applicationID, uint32_t steamAppID, TankHost* host );
    void initialize();
    void setActivityAlphaDevelop();
    inline const GameInstallationState& installation() throw() { return installation_; }
    void update( double gameTime, double delta );
    void shutdown();
    ~Discord();
  };

}