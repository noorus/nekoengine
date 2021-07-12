#pragma once

#include "tankengine.h"
#include "discord/cpp/discord.h"

namespace tank {

  struct DiscordUser {
    uint64_t id_;
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
      } bits;
      uint64_t field;
    } updated_;
    map<uint64_t, DiscordUser> friends_;
  };

  class Discord {
  private:
    TankHost* host_;
    unique_ptr<discord::Core> core_;
    int64_t clientID_;
    int64_t appID_;
    DiscordState state_;
    GameInstallationState installation_;
  public:
    Discord( int64_t clientID, int64_t applicationID, uint32_t steamAppID, TankHost* host );
    void initialize();
    inline const GameInstallationState& installation() throw() { return installation_; }
    void update();
    void shutdown();
    ~Discord();
  };

}