#pragma once
#include "neko_types.h"
#include "gfx_types.h"
#include "forwards.h"
#include "neko_exception.h"

namespace neko {

  namespace shaders {

    template <typename T>
    class PersistentBuffer
    {
    protected:
      GLuint id_;
      size_t size_;
      span<T> buffer_;

    public:
      PersistentBuffer( size_t count = 1 ): id_( 0 )
      {
        assert( count > 0 );
        size_ = ( count * sizeof( T ) );
        gl::glCreateBuffers( 1, &id_ );
        auto mapflags = ( gl::GL_MAP_WRITE_BIT | gl::GL_MAP_READ_BIT | gl::GL_MAP_PERSISTENT_BIT | gl::GL_MAP_COHERENT_BIT );
        auto storeflags = ( mapflags | gl::GL_DYNAMIC_STORAGE_BIT );
        gl::glNamedBufferStorage( id_, size_, nullptr, storeflags );
        auto buf = reinterpret_cast<T*>( gl::glMapNamedBufferRange( id_, 0, size_, mapflags ) );
        buffer_ = span<T>( buf, size_ );
      }
      inline GLuint id() const { return id_; }
      inline span<T>& buffer() { return buffer_; }
      ~PersistentBuffer()
      {
        gl::glUnmapNamedBuffer( id_ );
        gl::glDeleteBuffers( 1, &id_ );
      }
    };

    // FIXME This alignment isn't guaranteed to work.
    // Should query the requirement from the driver, it could be as bad as 256
    __declspec(align(16)) struct World
    {
      mat4 projection;
      mat4 view;
    };

    enum Type {
      Shader_Vertex,
      Shader_Fragment,
      Shader_Geometry,
      Shader_TesselationControl,
      Shader_TesselationEvaluation,
      Shader_Compute,
      Shader_Mesh,
      Shader_Task,
      Max_Shader_Type
    };

    class Shaders;

    struct Shader {
      friend class Shaders;
    protected:
      Type type_;
      GLuint id_;
      bool compiled_;
    public:
      Shader( Type type );
      inline Type type() const { return type_; }
      inline GLuint id() const { return id_; }
      inline bool compiled() const { return compiled_; }
      inline bool ready() const { return ( gl::glIsShader( id_ ) && compiled_ ); }
      ~Shader();
    };

    using ShaderPtr = unique_ptr<Shader>;
    using ShaderVector = vector<ShaderPtr>;

    struct Program {
      friend class Shaders;
    protected:
      GLuint id_;
      bool linked_;
      map<string, GLint> uniforms_;
    public:
      Program();
      inline GLuint id() const { return id_; }
      inline bool linked() const { return linked_; }
      inline bool ready() const { return ( gl::glIsProgram( id_ ) && linked_ ); }
      GLuint getUBO( const char* name );
      template <typename T>
      inline void setUniform( GLint index, T const& value )
      {
        if ( index == -1 )
          return;
        if constexpr ( std::is_same_v<T, GLint> )
          gl::glProgramUniform1i( id_, index, value );
        else if constexpr ( std::is_same_v<T, GLuint> )
          gl::glProgramUniform1ui( id_, index, value );
        else if constexpr ( std::is_same_v<T, bool> )
          gl::glProgramUniform1ui( id_, index, value );
        else if constexpr ( std::is_same_v<T, GLfloat> )
          gl::glProgramUniform1f( id_, index, value );
        else if constexpr ( std::is_same_v<T, GLdouble> )
          gl::glProgramUniform1d( id_, index, value );
        else if constexpr ( std::is_same_v<T, vec2f> )
          gl::glProgramUniform2fv( id_, index, 1, glm::value_ptr( value ) );
        else if constexpr ( std::is_same_v<T, vec3f> )
          gl::glProgramUniform3fv( id_, index, 1, glm::value_ptr( value ) );
        else if constexpr ( std::is_same_v<T, vec4f> )
          gl::glProgramUniform4fv( id_, index, 1, glm::value_ptr( value ) );
        else if constexpr ( std::is_same_v<T, vec2i> )
          gl::glProgramUniform2iv( id_, index, 1, glm::value_ptr( value ) );
        else if constexpr ( std::is_same_v<T, vec3i> )
          gl::glProgramUniform3iv( id_, index, 1, glm::value_ptr( value ) );
        else if constexpr ( std::is_same_v<T, vec4i> )
          gl::glProgramUniform4iv( id_, index, 1, glm::value_ptr( value ) );
        else if constexpr ( std::is_same_v<T, vec2u> )
          gl::glProgramUniform2uiv( id_, index, 1, glm::value_ptr( value ) );
        else if constexpr ( std::is_same_v<T, vec3u> )
          gl::glProgramUniform3uiv( id_, index, 1, glm::value_ptr( value ) );
        else if constexpr ( std::is_same_v<T, vec4u> )
          gl::glProgramUniform4uiv( id_, index, 1, glm::value_ptr( value ) );
        else if constexpr ( std::is_same_v<T, quaternion> )
          gl::glProgramUniform4fv( id_, index, 1, glm::value_ptr( value ) );
        else if constexpr ( std::is_same_v<T, mat3> )
          gl::glProgramUniformMatrix3fv( id_, index, 1, gl::GL_FALSE, glm::value_ptr( value ) );
        else if constexpr ( std::is_same_v<T, mat4> )
          gl::glProgramUniformMatrix4fv( id_, index, 1, gl::GL_FALSE, glm::value_ptr( value ) );
        else
          NEKO_EXCEPT( "Unsupported uniform value type" );
      }
      inline GLint getUniformLocation( const char* name )
      {
        auto ret = gl::glGetUniformLocation( id_, name );
        uniforms_[name] = ret;
        return ret;
      }
      template <typename T>
      inline void setUniform( const char* name, T const& value )
      {
        if ( uniforms_.find( name ) == uniforms_.end() )
        {
          return setUniform<T>( getUniformLocation( name ), value );
        }
        return setUniform<T>( uniforms_[name], value );
      }
      ~Program();
    };

    using ProgramPtr = shared_ptr<Program>;
    using ProgramVector = vector<ProgramPtr>;

    struct Pipeline {
      friend class Shaders;
    private:
      Pipeline() = delete;
    protected:
      GLuint id_;
      utf8String name_;
      map<Type, ProgramPtr> stages_;
    public:
      Pipeline( const utf8String& name );
      inline GLuint id() const { return id_; }
      void setProgramStage( Type stage, GLuint id );
      inline bool ready() { return ( gl::glIsProgramPipeline( id_ ) && !stages_.empty() ); }
      template <typename T>
      inline void setUniform( const char* name, T const& value )
      {
        for ( auto& stage : stages_ )
        {
          stage.second->setUniform( name, value );
        }
      }
      ~Pipeline();
    };

    using PipelinePtr = unique_ptr<Pipeline>;
    using PipelineVector = vector<PipelinePtr>;
    using PipelineMap = map<utf8String, PipelinePtr>;

    class Shaders {
    protected:
      ConsolePtr console_;
      utf8String rootFilePath_;
      ShaderVector shaders_;
      ProgramVector programs_;
      PipelineMap pipelines_;
      unique_ptr<PersistentBuffer<World>> world_;
      void dumpLog( const GLuint& target, const bool isProgram );
      void compileShader( Shader& shader, const string_view source );
      void linkSingleProgram( Program& program, Shader& shader );
      utf8String loadSource( const utf8String& filename );
    public:
      Shaders( ConsolePtr console );
      inline World* world() { return world_->buffer().data(); }
      void initialize();
      void createSimplePipeline( const utf8String& name, const utf8String& vp_filename, const utf8String& fp_filename );
      Pipeline& usePipeline( const utf8String& name );
      void shutdown();
      ~Shaders();
    };

  }

}