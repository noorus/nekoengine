#pragma once
#include "subsystem.h"
#include "forwards.h"
#include "gfx_types.h"
#include "texture.h"
#include "resources.h"
#include "materials.h"

namespace neko {

  class SpriteManager;

  struct SpriteAnimationDefinition {
    friend SpriteManager; 
  private:
    utf8String name_;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    uint32_t frameCount_ = 0;
  public:
    inline const utf8String& name() const noexcept { return name_; }
    inline uint32_t width() const noexcept { return width_; }
    inline uint32_t height() const noexcept { return height_; }
    inline uint32_t frameCount() const noexcept { return frameCount_; }
  };

  using SpriteAnimationDefinitionMap = map<utf8String, SpriteAnimationDefinition>;

  struct SpriteAnimationSetDefinitionEntry
  {
    utf8String name_;
    utf8String defName_;
    vec2 sheetPos_ { 0.0f, 0.0f };
    vector<int> flipFramesX_ {};
  };

  using SpriteAnimationSetDefinitionMap = map<utf8String, map<utf8String, SpriteAnimationSetDefinitionEntry>>;

  class Sprite {
  private:
    MaterialPtr material_;
    vec2 frameDimensions_ { 0.0f, 0.0f };
    uint32_t frameCount_ = 0;
  public:
    vec2 uv() const;
  };

  using SpritePtr = shared_ptr<Sprite>;

  class SpriteManager {
  private:
    Renderer* renderer_ = nullptr;
    SpriteAnimationDefinitionMap animdefs_;
    SpriteAnimationSetDefinitionMap setdefs_;
  public:
    SpriteManager( Renderer* renderer );
    void initialize();
    void shutdown();
    void loadAnimdefJSONRaw( const nlohmann::json& arr );
    void loadAnimdefJSON( const utf8String& input );
    void loadAnimdefFile( const utf8String& filename );
    void loadAnimsetJSONRaw( const nlohmann::json& arr );
    void loadAnimsetJSON( const utf8String& input );
    void loadAnimsetFile( const utf8String& filename );
    ~SpriteManager();
  };

}