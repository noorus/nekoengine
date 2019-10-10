#pragma once
#include "utilities.h"
#include "renderer.h"
#include "fontmanager.h"

namespace neko {

  struct LoadTask {
    enum LoadType {
      Load_Texture,
      Load_Fontface
    } type_;
    struct TextureLoad {
      MaterialPtr material_;
      utf8String path_;
    } textureLoad;
    struct FontfaceLoad {
      FontPtr font_;
      Font::Specs specs_;
      utf8String path_;
    } fontfaceLoad;
    LoadTask( MaterialPtr material, const string& path ): type_( Load_Texture )
    {
      textureLoad.material_ = move( material );
      textureLoad.path_ = path;
    }
    LoadTask( FontPtr font, const utf8String& path, Real pointSize ): type_( Load_Fontface )
    {
      fontfaceLoad.font_ = move( font );
      fontfaceLoad.specs_.atlasSize_ = vec2i( 8192, 8192 );
      fontfaceLoad.specs_.pointSize_ = pointSize;
      fontfaceLoad.path_ = path;
    }
  };

  using LoadTaskVector = vector<LoadTask>;

  class ThreadedLoader: public enable_shared_from_this<ThreadedLoader> {
  protected:
    platform::Thread thread_;
    platform::Event newTasksEvent_;
    platform::RWLock addTaskLock_;
    platform::Event finishedTasksEvent_;
    platform::RWLock finishedTasksLock_;
    LoadTaskVector newTasks_;
    MaterialVector finishedMaterials_;
    FontVector finishedFonts_;
    void handleNewTasks();
  private:
    static bool threadProc( platform::Event& running, platform::Event& wantStop, void* argument );
  public:
    ThreadedLoader();
    void start();
    void stop();
    void getFinishedMaterials( MaterialVector& materials );
    void getFinishedFonts( FontVector& fonts );
    void addLoadTask( const LoadTaskVector& resources );
    ~ThreadedLoader();
  };

}