#pragma once
#ifndef NEKO_NO_SCRIPTING

#include "neko_types.h"
#include "neko_exception.h"
#include "console.h"
#include "mesh_primitives.h"
#include "js_wrapper.h"
#include "locator.h"

namespace neko {

  namespace js {

    class Mesh: public DynamicObjectWrapper<Mesh, JSMesh> {
    private:
      JSMesh local_;
    protected:
      void js_toString( const V8CallbackArgs& args );
    public:
      static void jsConstructor( const V8CallbackArgs& info );
      virtual int32_t jsEstimateSize() const;
      virtual void jsOnDestruct( Isolate* isolate );
      static void registerExport( Isolate* isolate, V8FunctionTemplate& obj );
    public:
      Mesh( const JSMesh& source ): local_( source ) {}
      inline void setFrom( const JSMesh& other )
      {
        Locator::console().printf( neko::Console::srcScripting,
          "js::Mesh(0x%I64X) setFrom JSMesh(0x%I64X)", this, other );
        // TODO better; this is destructive
        local_.vbo_->copy( *other.vbo_ );
        local_.ebo_->copy( *other.ebo_ );
        local_.vao_->reset();
      }
      /*inline void setFromDestructive( JSMesh& other )
      {
        local_.localVBO_ = other.localVBO_;
        local_.localEBO_ = other.localEBO_;
        local_.localVAO_ = other.localVAO_;
      }*/
      inline JSMesh& mesh() { return local_; } //!< Get the internal JSMesh.
    };

    using MeshPtr = shared_ptr<Mesh>;
    using MeshVector = vector<Mesh*>;
    using MeshPtrVector = vector<MeshPtr>;

    MeshPtr extractMeshMember( Isolate* isolate, const utf8String& func, v8::MaybeLocal<v8::Object>& maybeObject, const utf8String& name, bool shouldThrow );

  }

}

#endif