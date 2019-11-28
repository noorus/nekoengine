#pragma once
#include "neko_types.h"
#include "gfx_types.h"
#include "forwards.h"

namespace neko {

  struct Shader {
  public:
    enum Type {
      Vertex,
      Fragment,
      Geometry
    } type_;
    utf8String filename_;
    GLuint id_;
    Shader( Type type, const utf8String& filename ):
      type_( type ), filename_( filename ), id_( 0 ) {}
  };

  using ShaderVector = vector<Shader>;

  struct ShaderProgram {
    string name;
    GLuint id;
    size_t vp;
    size_t fp;
    map<string, GLuint> uniforms;
    ShaderProgram( const string& name_, size_t vertexProgram, size_t fragmentProgram ):
      name( name_ ), vp( vertexProgram ), fp( fragmentProgram ), id( 0 ) {}
    GLuint getUBO( const char* name );
    template <typename T>
    inline void setUniform( const char* name, T const& value )
    {
      auto& index = uniforms[name];
      if constexpr ( std::is_same_v<T, GLint> )
        glProgramUniform1i( id, index, value );
      else if constexpr ( std::is_same_v<T, GLuint> )
        glProgramUniform1ui( id, index, value );
      else if constexpr ( std::is_same_v<T, bool> )
        glProgramUniform1ui( id, index, value );
      else if constexpr ( std::is_same_v<T, GLfloat> )
        glProgramUniform1f( id, index, value );
      else if constexpr ( std::is_same_v<T, GLdouble> )
        glProgramUniform1d( id, index, value );
      else if constexpr ( std::is_same_v<T, vec2f> )
        glProgramUniform2fv( id, index, 1, glm::value_ptr( value ) );
      else if constexpr ( std::is_same_v<T, vec3f> )
        glProgramUniform3fv( id, index, 1, glm::value_ptr( value ) );
      else if constexpr ( std::is_same_v<T, vec4f> )
        glProgramUniform4fv( id, index, 1, glm::value_ptr( value ) );
      else if constexpr ( std::is_same_v<T, vec2i> )
        glProgramUniform2iv( id, index, 1, glm::value_ptr( value ) );
      else if constexpr ( std::is_same_v<T, vec3i> )
        glProgramUniform3iv( id, index, 1, glm::value_ptr( value ) );
      else if constexpr ( std::is_same_v<T, vec4i> )
        glProgramUniform4iv( id, index, 1, glm::value_ptr( value ) );
      else if constexpr ( std::is_same_v<T, vec2u> )
        glProgramUniform2uiv( id, index, 1, glm::value_ptr( value ) );
      else if constexpr ( std::is_same_v<T, vec3u> )
        glProgramUniform3uiv( id, index, 1, glm::value_ptr( value ) );
      else if constexpr ( std::is_same_v<T, vec4u> )
        glProgramUniform4uiv( id, index, 1, glm::value_ptr( value ) );
      else if constexpr ( std::is_same_v<T, quaternion> )
        glProgramUniform4fv( id, index, 1, glm::value_ptr( value ) );
      else if constexpr ( std::is_same_v<T, mat3> )
        glProgramUniformMatrix3fv( id, index, 1, GL_FALSE, glm::value_ptr( value ) );
      else if constexpr ( std::is_same_v<T, mat4> )
        glProgramUniformMatrix4fv( id, index, 1, GL_FALSE, glm::value_ptr( value ) );
      else
        NEKO_EXCEPT( "Unsupported uniform value type" );
    }
  };

  using ShaderProgramVector = vector<ShaderProgram>;

  class Shaders: public noncopyable {
  private:
    mat4 model_;
    mat4 view_;
    mat4 projection_;
    EnginePtr engine_;
    ShaderVector shaders_;
    ShaderProgramVector programs_;
    void dumpLog( const GLuint &target, const bool isProgram = false );
    void linkProgram( GLuint vertex, GLuint fragment, GLuint& program_out );
    void linkProgram( ShaderProgram& );
    void compileShader( GLenum type, const string_view source, GLuint& out );
    void compileShader( Shader& shader );
  public:
    Shaders( EnginePtr engine );
    void initialize();
    void shutdown();
    ~Shaders();
    void setModelMat( const mat4& model );
    void setViewProjectionMat( const mat4& view, const mat4& projection );
    ShaderProgram& get( size_t program );
    ShaderProgram& use( size_t program );
  };

}