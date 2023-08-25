#pragma once
#include "neko_types.h"
#include "gfx_types.h"
#include "forwards.h"
#include "neko_exception.h"
#include "mesh_primitives.h"
#include "inc.buffers.glsl"

namespace neko {

  namespace shaders {

    enum Type {
      Shader_Vertex,
      Shader_Fragment,
      Shader_Geometry,
      Shader_TessellationControl,
      Shader_TessellationEvaluation,
      Shader_Compute,
      Shader_Mesh,
      Shader_Task,
      Max_Shader_Type
    };

    class Shaders;

    struct Shader: public nocopy {
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

    struct Program: public nocopy {
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
      inline void setUniformInt( GLint index, int value )
      {
        if ( index >= 0 )
          gl::glProgramUniform1i( id_, index, value );
      }
      inline void setUniformUint( GLint index, unsigned int value )
      {
        if ( index >= 0 )
          gl::glProgramUniform1ui( id_, index, value );
      }
      inline void setUniformBool( GLint index, bool value )
      {
        if ( index >= 0 )
          gl::glProgramUniform1ui( id_, index, value );
      }
      inline void setUniformFloat( GLint index, float value )
      {
        if ( index >= 0 )
          gl::glProgramUniform1f( id_, index, value );
      }
      inline void setUniformDouble( GLint index, double value )
      {
        if ( index >= 0 )
          gl::glProgramUniform1d( id_, index, value );
      }
      inline void setUniformVec2f( GLint index, const vec2f& value )
      {
        if ( index >= 0 )
          gl::glProgramUniform2fv( id_, index, 1, glm::value_ptr( value ) );
      }
      inline void setUniformVec3f( GLint index, const vec3f& value )
      {
        if ( index >= 0 )
          gl::glProgramUniform3fv( id_, index, 1, glm::value_ptr( value ) );
      }
      inline void setUniformVec4f( GLint index, const vec4f& value )
      {
        if ( index >= 0 )
          gl::glProgramUniform4fv( id_, index, 1, glm::value_ptr( value ) );
      }
      inline void setUniformVec2i( GLint index, const vec2i& value )
      {
        if ( index >= 0 )
          gl::glProgramUniform2iv( id_, index, 1, glm::value_ptr( value ) );
      }
      inline void setUniformVec3i( GLint index, const vec3i& value )
      {
        if ( index >= 0 )
          gl::glProgramUniform3iv( id_, index, 1, glm::value_ptr( value ) );
      }
      inline void setUniformVec4i( GLint index, const vec4i& value )
      {
        if ( index >= 0 )
          gl::glProgramUniform4iv( id_, index, 1, glm::value_ptr( value ) );
      }
      inline void setUniformVec2u( GLint index, const vec2u& value )
      {
        if ( index >= 0 )
          gl::glProgramUniform2uiv( id_, index, 1, glm::value_ptr( value ) );
      }
      inline void setUniformVec3u( GLint index, const vec3u& value )
      {
        if ( index >= 0 )
          gl::glProgramUniform3uiv( id_, index, 1, glm::value_ptr( value ) );
      }
      inline void setUniformVec4u( GLint index, const vec4u& value )
      {
        if ( index >= 0 )
          gl::glProgramUniform4uiv( id_, index, 1, glm::value_ptr( value ) );
      }
      inline void setUniformQuat( GLint index, const quat& value )
      {
        if ( index >= 0 )
          gl::glProgramUniform4fv( id_, index, 1, glm::value_ptr( value ) );
      }
      inline void setUniformMat3( GLint index, const mat3& value )
      {
        if ( index >= 0 )
          gl::glProgramUniformMatrix3fv( id_, index, 1, gl::GL_FALSE, glm::value_ptr( value ) );
      }
      inline void setUniformMat4( GLint index, const mat4& value )
      {
        if ( index >= 0 )
          gl::glProgramUniformMatrix4fv( id_, index, 1, gl::GL_FALSE, glm::value_ptr( value ) );
      }
      template <typename T>
      inline void setUniform( GLint index, T const& value )
      {
        if ( index == -1 )
          return;
        if constexpr ( std::is_same_v<T, GLint> )
          setUniformInt( index, value );
        else if constexpr ( std::is_same_v<T, GLuint> )
          setUniformUint( index, value );
        else if constexpr ( std::is_same_v<T, bool> )
          setUniformBool( index, value );
        else if constexpr ( std::is_same_v<T, GLfloat> )
          setUniformFloat( index, value );
        else if constexpr ( std::is_same_v<T, GLdouble> )
          setUniformDouble( index, value );
        else if constexpr ( std::is_same_v<T, vec2f> )
          setUniformVec2f( index, value );
        else if constexpr ( std::is_same_v<T, vec3f> )
          setUniformVec3f( index, value );
        else if constexpr ( std::is_same_v<T, vec4f> )
          setUniformVec4f( index, value );
        else if constexpr ( std::is_same_v<T, vec2i> )
          setUniformVec2i( index, value );
        else if constexpr ( std::is_same_v<T, vec3i> )
          setUniformVec3i( index, value );
        else if constexpr ( std::is_same_v<T, vec4i> )
          setUniformVec4i( index, value );
        else if constexpr ( std::is_same_v<T, vec2u> )
          setUniformVec2u( index, value );
        else if constexpr ( std::is_same_v<T, vec3u> )
          setUniformVec3u( index, value );
        else if constexpr ( std::is_same_v<T, vec4u> )
          setUniformVec4u( index, value );
        else if constexpr ( std::is_same_v<T, quaternion> )
          setUniformQuat( index, value );
        else if constexpr ( std::is_same_v<T, mat3> )
          setUniformMat3( index, value );
        else if constexpr ( std::is_same_v<T, mat4> )
          setUniformMat4( index, value );
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

    struct Pipeline: public nocopy {
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
      inline GLuint getProgramStage( Type stage )
      {
        assert( stages_[stage] );
        return stages_[stage]->id();
      }
      inline ProgramPtr getProgram( Type stage )
      {
        if ( !stages_.contains( stage ) )
          return {};
        return stages_[stage];
      }
      inline bool ready() { return ( gl::glIsProgramPipeline( id_ ) && !stages_.empty() ); }
      template <typename T>
      inline Pipeline& setUniform( const char* name, T const& value )
      {
        for ( auto& stage : stages_ )
        {
          stage.second->setUniform( name, value );
        }
        return ( *this );
      }
      ~Pipeline();
    };

    using PipelinePtr = unique_ptr<Pipeline>;
    using PipelineVector = vector<PipelinePtr>;
    using PipelineMap = map<utf8String, PipelinePtr>;

    class Shaders: public nocopy {
    protected:
      ConsolePtr console_;
      ShaderVector shaders_;
      ProgramVector programs_;
      PipelineMap pipelines_;
      map<utf8String, utf8String> includes_;
      unique_ptr<MappedGLBuffer<neko::uniforms::World>> world_;
      unique_ptr<MappedGLBuffer<neko::uniforms::Processing>> processing_;
      void dumpLog( const GLuint& target, const bool isProgram );
      void readIncludeFile( utf8String filename );
      void compileShader( Shader& shader, const string_view source );
      void linkSingleProgram( Program& program, Shader& shader, const vector<utf8String>& uniforms );
      utf8String readSource( const utf8String& filename, int includeDepth = 0 );
      void buildSeparableProgram( const utf8String& name, const utf8String& filename, Type type, ShaderPtr& shader, ProgramPtr& program, const vector<utf8String>& uniforms );
    public:
      Shaders( ConsolePtr console );
      inline unique_ptr<MappedGLBuffer<neko::uniforms::World>>& world() { return world_; }
      inline unique_ptr<MappedGLBuffer<neko::uniforms::Processing>>& processing() { return processing_; }
      void initialize();
      Pipeline& usePipeline( const utf8String& name );
      void loadIncludeJSONRaw( const nlohmann::json& arr );
      void loadIncludeJSON( const utf8String& input );
      void loadIncludeFile( const utf8String& filename );
      void loadPipelineJSONRaw( const nlohmann::json& arr );
      void loadPipelineJSON( const utf8String& input );
      void loadPipelineFile( const utf8String& filename );
      void shutdown();
      ~Shaders();
    };

  }

}