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
    gl::GLuint id_;
    Shader( Type type, const utf8String& filename ):
      type_( type ), filename_( filename ), id_( 0 ) {}
  };

  using ShaderVector = vector<Shader>;

  struct ShaderProgram {
    string name;
    gl::GLuint id;
    size_t vp;
    size_t fp;
    map<string, GLuint> uniforms;
    ShaderProgram( const string& name_, size_t vertexProgram, size_t fragmentProgram ):
      name( name_ ), vp( vertexProgram ), fp( fragmentProgram ), id( 0 ) {}
    void setUniform2f( const char *uniform, const float &f0, const float &f1 );
    void setUniform4f( const char *uniform, const float &f0, const float &f1, const float &f2, const float &f3 );
    void setUniform4fv( const char *uniform, const gl::GLsizei &count, const float *v );
    void setUniformMatrix4fv( const char *uniform, const gl::GLsizei &count, const gl::GLboolean &transpose, const gl::GLfloat *v );
    void setUniform1i( const char *uniform, const gl::GLint &i );
    void setUniform1ui( const char *uniform, const gl::GLuint &i );
    void setUniform1f( const char* uniform, const gl::GLfloat& f );
    void setUniform1d( const char* uniform, const gl::GLdouble& d );
    gl::GLuint getUBO( const char* name );
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
    void dumpLog( const gl::GLuint &target, const bool isProgram = false );
    void linkProgram( gl::GLuint vertex, gl::GLuint fragment, gl::GLuint& program_out );
    void linkProgram( ShaderProgram& );
    void compileShader( gl::GLenum type, const std::string& source, gl::GLuint& out );
    void compileShader( Shader& shader );
  public:
    Shaders( EnginePtr engine );
    void initialize();
    void shutdown();
    ~Shaders();
    void setMatrices( const mat4& model, const mat4& view, const mat4& projection );
    ShaderProgram& get( size_t program );
    ShaderProgram& use( size_t program );
  };

}