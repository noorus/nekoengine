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
  public:
    enum Direction
    {
      Horizontal = 0,
      Vertical
    };
  private:
    utf8String name_;
    int width_ = 0;
    int height_ = 0;
    int frameCount_ = 0;
    Direction direction_ = Horizontal;
  public:
    inline const utf8String& name() const noexcept { return name_; }
    inline int width() const noexcept { return width_; }
    inline int height() const noexcept { return height_; }
    inline int frameCount() const noexcept { return frameCount_; }
    inline Direction direction() const noexcept { return direction_; }
  };

  using SpriteAnimationDefinitionPtr = shared_ptr<SpriteAnimationDefinition>;
  using SpriteAnimationDefinitionMap = map<utf8String, SpriteAnimationDefinitionPtr>;

  struct SpriteAnimationSetDefinitionEntry
  {
    const utf8String name_;
    const utf8String sheetName_;
    const utf8String defName_;
    const vec2i sheetPos_ { 0, 0 };
    const vector<int> flipFramesX_ {};
    SpriteAnimationDefinitionPtr definition_;
    MaterialPtr material_;
  public:
    SpriteAnimationSetDefinitionEntry( const utf8String& name, const utf8String& sheetname, const utf8String& defname,
      const vec2i& sheetpos, const vector<int>& flipframesx );
  };

  using SpriteAnimationSetDefinitionEntryPtr = shared_ptr<SpriteAnimationSetDefinitionEntry>;

  struct SpriteAnimationSetDefinition
  {
    utf8String name_;
    map<utf8String, SpriteAnimationSetDefinitionEntryPtr> entries_;
  };

  using SpriteAnimationSetDefinitionPtr = shared_ptr<SpriteAnimationSetDefinition>;
  using SpriteAnimationSetDefinitionVector = vector<SpriteAnimationSetDefinitionPtr>;

  class Sprite {
  public:
  };

  using SpritePtr = shared_ptr<Sprite>;

  class SpriteManager {
  private:
    Renderer* renderer_ = nullptr;
    SpriteAnimationDefinitionMap animdefs_;
    //SpriteAnimationSetDefinitionMap setdefs_;
  public:
    SpriteManager( Renderer* renderer );
    void initialize();
    void shutdown();
    void draw( GameTime time );
    void loadAnimdefJSONRaw( const nlohmann::json& arr );
    void loadAnimdefJSON( const utf8String& input );
    void loadAnimdefFile( const utf8String& filename );
    void loadAnimsetJSONRaw( const nlohmann::json& arr );
    void loadAnimsetJSON( const utf8String& input );
    void loadAnimsetFile( const utf8String& filename );
    ~SpriteManager();
  };

}