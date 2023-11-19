#pragma once
#include "utilities.h"
#include "renderer.h"
#include "font.h"
#include "gfx_types.h"
#include "spriteanim.h"

namespace neko {

  struct FontLoadSpec
  {
    Real size;
    FontRendering rendering;
    Real thickness;
  };

  struct LoadTask
  {
    enum LoadType {
      Load_Texture,
      Load_Fontface,
      Load_Spritesheet
    } type_;
    struct TextureLoad {
      MaterialPtr material_;
      vector<utf8String> paths_;
    } textureLoad;
    struct FontfaceLoad {
      FontPtr font_;
      utf8String path_;
      vector<FontLoadSpec> specs_;
    } fontfaceLoad;
    struct SpritesheetLoad
    {
      SpriteAnimationSetDefinitionPtr def_;
    } spriteLoad;
    LoadTask( MaterialPtr material, vector<utf8String> paths ): type_( Load_Texture )
    {
      textureLoad.material_ = move( material );
      textureLoad.paths_.swap( paths );
    }
    LoadTask( FontPtr font, const utf8String& path, vector<FontLoadSpec>& specs ): type_( Load_Fontface )
    {
      fontfaceLoad.font_ = move( font );
      fontfaceLoad.path_ = path;
      fontfaceLoad.specs_ = specs;
    }
    LoadTask( SpriteAnimationSetDefinitionPtr ptr ): type_( Load_Spritesheet )
    {
      spriteLoad.def_ = move( ptr );
    }
  };

  using LoadTaskVector = vector<LoadTask>;

  class ThreadedLoader: public enable_shared_from_this<ThreadedLoader>, public nocopy {
  protected:
    platform::Thread thread_;
    platform::Event newTasksEvent_;
    platform::RWLock addTaskLock_;
    platform::Event finishedMaterialsEvent_;
    platform::Event finishedFontsEvent_;
    platform::Event finishedModelsEvent_;
    platform::Event finishedAnimationsEvent_;
    platform::Event finishedSpritesheetsEvent_;
    platform::RWLock finishedTasksLock_;
    LoadTaskVector newTasks_;
    MaterialVector finishedMaterials_;
    FontVector finishedFonts_;
    SpriteAnimationSetDefinitionVector finishedSpritesheets_;
    void loadFontFace( LoadTask::FontfaceLoad& task );
    Pixmap loadTexture( const utf8String& path );
    void loadMaterial( LoadTask::TextureLoad& task );
    void loadSpritesheet( LoadTask::SpritesheetLoad& task );
    void handleNewTasks();
  private:
    static bool threadProc( platform::Event& running, platform::Event& wantStop, void* argument );
  public:
    ThreadedLoader();
    void start();
    void stop();
    void getFinishedMaterials( MaterialVector& materials );
    void getFinishedFonts( FontVector& fonts );
    void getFinishedSpritesheets( SpriteAnimationSetDefinitionVector& sheets );
    void addLoadTask( const LoadTaskVector& resources );
    void clear();
    ~ThreadedLoader();
  };

}