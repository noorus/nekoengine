#pragma once
#include "utilities.h"
#include "renderer.h"
#include "font.h"
#include "gfx_types.h"

#ifndef NEKO_NO_ANIMATION
# include <ozz/animation/offline/fbx/fbx.h>
#endif

namespace neko {

  class SceneNode;

#ifndef NEKO_NO_ANIMATION
  struct OzzAnimationEntry
  {
    unique_ptr<ozz::animation::offline::RawAnimation> animation_;
    map<utf8String, size_t> boneTrackMap_;
  };
#else
  struct OzzAnimationEntry
  {
  };
#endif

  using AnimationEntryPtr = shared_ptr<OzzAnimationEntry>;

  using AnimationVector = vector<AnimationEntryPtr>;

  namespace loaders {

#ifndef NEKO_NO_ANIMATION

    struct UnityYamlNode {
      bool isRoot = false;
      int indent;
      UnityYamlNode* parent;
      utf8String name;
      map<utf8String, utf8String> attribs;
      vector<UnityYamlNode*> children;
      UnityYamlNode( UnityYamlNode* parent_, int indent_ ): parent( parent_ ), indent( indent_ ) {}
      ~UnityYamlNode()
      {
        for ( auto& child : children )
          delete child;
      }
    };

    void dumpUnityYaml( UnityYamlNode& node, size_t level = 0 );
    void loadUnityAnimation( const vector<uint8_t>& data, AnimationEntryPtr& into );

#endif

    void loadGLTFModel( const vector<uint8_t>& input, const utf8String& filename, const utf8String& basedir, SceneNode* out );

#ifndef NEKO_NO_FBX
    class FbxLoader {
    protected:
      fbxsdk::FbxManager* fbxmgr_;
      fbxsdk::FbxIOSettings* fbxio_;
      unique_ptr<ozz::animation::offline::fbx::FbxSystemConverter> converter_;
      void parseFBXNode( fbxsdk::FbxNode* node, SceneNode& out );
    public:
      FbxLoader();
      void loadFBXScene( const vector<uint8_t>& data, const utf8String& filename, SceneNode* rootNode );
      ~FbxLoader();
    };
#endif

  }

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
      Font::Specs specs_;
      utf8String path_;
    } fontfaceLoad;
    struct ModelLoad {
      SceneNode* node_;
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
      fontfaceLoad.specs_.atlasSize_ = vec2i( 1024, 1024 );
      fontfaceLoad.specs_.pointSize_ = pointSize;
      fontfaceLoad.path_ = path;
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
#ifndef NEKO_NO_FBX
    loaders::FbxLoader fbxLoader_;
#endif
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