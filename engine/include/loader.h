#pragma once
#include "utilities.h"
#include "renderer.h"
#include "fontmanager.h"
#include "gfx_types.h"

namespace neko {

  class SceneNode;

  struct LoadTask {
    enum LoadType {
      Load_Texture,
      Load_Fontface,
      Load_Model
    } type_;
    struct TextureLoad {
      MaterialPtr material_;
      vector<utf8String> paths_;
    } textureLoad;
    struct FontfaceLoad {
      FontPtr font_;
      Font::Specs specs_;
      utf8String path_;
    } fontfaceLoad;
    struct ModelLoad {
      utf8String path_;
    } modelLoad;
    LoadTask( MaterialPtr material, vector<utf8String> paths ): type_( Load_Texture )
    {
      textureLoad.material_ = move( material );
      textureLoad.paths_.swap( paths );
    }
    LoadTask( FontPtr font, const utf8String& path, Real pointSize ): type_( Load_Fontface )
    {
      fontfaceLoad.font_ = move( font );
      fontfaceLoad.specs_.atlasSize_ = vec2i( 1024, 1024 );
      fontfaceLoad.specs_.pointSize_ = pointSize;
      fontfaceLoad.path_ = path;
    }
    LoadTask( const utf8String& path ): type_( Load_Model )
    {
      modelLoad.path_ = path;
    }
  };

  using LoadTaskVector = vector<LoadTask>;

  class ThreadedLoader: public enable_shared_from_this<ThreadedLoader> {
  protected:
    platform::Thread thread_;
    platform::Event newTasksEvent_;
    platform::RWLock addTaskLock_;
    platform::Event finishedMaterialsEvent_;
    platform::Event finishedFontsEvent_;
    platform::Event finishedModelsEvent_;
    platform::RWLock finishedTasksLock_;
    FbxManager* fbxmgr_;
    FbxIOSettings* fbxio_;
    LoadTaskVector newTasks_;
    MaterialVector finishedMaterials_;
    vector<SceneNode*> finishedModels_;
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
    void getFinishedModels( vector<SceneNode*>& models );
    void addLoadTask( const LoadTaskVector& resources );
    ~ThreadedLoader();
  };

}