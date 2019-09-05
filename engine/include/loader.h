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
      FontManagerPtr manager_;
      utf8String path_;
      float size_;
    } fontfaceLoad;
    LoadTask( MaterialPtr material, const string& path ): type_( Load_Texture )
    {
      textureLoad.material_ = move( material );
      textureLoad.path_ = path;
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
    void handleNewTasks();
  private:
    static bool threadProc( platform::Event& running, platform::Event& wantStop, void* argument );
  public:
    ThreadedLoader();
    void start();
    void stop();
    void getFinishedMaterials( MaterialVector& materials );
    void addLoadTask( const LoadTaskVector& resources );
    ~ThreadedLoader();
  };

}