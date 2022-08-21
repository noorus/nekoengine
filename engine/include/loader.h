#pragma once
#include "utilities.h"
#include "renderer.h"
#include "font.h"
#include "gfx_types.h"

namespace neko {

  class SceneNode;

  // Unused atm
  struct AnimationEntry
  {
  };

  using AnimationEntryPtr = shared_ptr<AnimationEntry>;
  using AnimationVector = vector<AnimationEntryPtr>;

  namespace loaders {

    void loadGLTFModel( const vector<uint8_t>& input, const utf8String& filename, const utf8String& basedir, SceneNode* out );

  }

  struct FontLoadStyleSpec {
    newtype::FontRendering rendering;
    Real thickness;
  };

  struct LoadTask
  {
    enum LoadType {
      Load_Texture,
      Load_Fontface,
      Load_Model,
      Load_Animation
    } type_;
    struct TextureLoad {
      MaterialPtr material_;
      vector<utf8String> paths_;
    } textureLoad;
    struct FontfaceLoad {
      FontPtr font_;
      utf8String path_;
      Real size_ = 0.0f;
      vector<FontLoadStyleSpec> styles_;
    } fontfaceLoad;
    struct ModelLoad {
      SceneNode* node_ = nullptr;
      utf8String path_;
    } modelLoad;
    struct AnimationLoad {
      AnimationEntryPtr animation_;
      utf8String path_;
    } animationLoad;
    LoadTask( MaterialPtr material, vector<utf8String> paths ): type_( Load_Texture )
    {
      textureLoad.material_ = move( material );
      textureLoad.paths_.swap( paths );
    }
    LoadTask( FontPtr font, const utf8String& path, Real pointSize ): type_( Load_Fontface )
    {
      fontfaceLoad.font_ = move( font );
      fontfaceLoad.path_ = path;
      fontfaceLoad.size_ = pointSize;
      fontfaceLoad.styles_ = { { newtype::FontRender_Normal, 0.0 } };
    }
    LoadTask( SceneNode* modelRootNode, const utf8String& path ): type_( Load_Model )
    {
      modelLoad.node_ = modelRootNode;
      modelLoad.path_ = path;
    }
    LoadTask( AnimationEntryPtr animation, const utf8String& path ): type_( Load_Animation )
    {
      animationLoad.animation_ = move( animation );
      animationLoad.path_ = path;
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
    platform::RWLock finishedTasksLock_;
    LoadTaskVector newTasks_;
    MaterialVector finishedMaterials_;
    vector<SceneNode*> finishedModels_;
    FontVector finishedFonts_;
    AnimationVector finishedAnimations_;
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
    void getFinishedAnimations( AnimationVector& animations );
    void addLoadTask( const LoadTaskVector& resources );
    ~ThreadedLoader();
  };

}