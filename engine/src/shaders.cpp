#include "stdafx.h"
#include "shaders.h"
#include "engine.h"
#include "console.h"
#include "locator.h"
#include "memory.h"
#include "neko_exception.h"

namespace neko {

  struct ShaderMapper {
    Shader::Type type;
    GLenum glType;
    string name;
  };

  const ShaderMapper cShaderTypes[3] = {
    { Shader::Vertex, GL_VERTEX_SHADER, "vertex" },
    { Shader::Fragment, GL_FRAGMENT_SHADER, "fragment" },
    { Shader::Geometry, GL_GEOMETRY_SHADER, "geometry" }
  };

  // Program

  void ShaderProgram::setUniform2f( const char *uniform, const float &f0, const float &f1 )
  {
    glUniform2f( uniforms[uniform], f0, f1 );
  }

  void ShaderProgram::setUniform4f( const char *uniform, const float &f0, const float &f1, const float &f2, const float &f3 )
  {
    glUniform4f( uniforms[uniform], f0, f1, f2, f3 );
  }

  void ShaderProgram::setUniform4fv( const char *uniform, const GLsizei &count, const float *v )
  {
    glUniform4fv( uniforms[uniform], count, v );
  }

  void ShaderProgram::setUniform1i( const char *uniform, const GLint &i )
  {
    glUniform1i( uniforms[uniform], i );
  }

  void ShaderProgram::setUniform1ui( const char *uniform, const GLuint &i )
  {
    glUniform1ui( uniforms[uniform], i );
  }

  void ShaderProgram::setUniform1f( const char* uniform, const GLfloat& f )
  {
    glUniform1f( uniforms[uniform], f );
  }

  void ShaderProgram::setUniform1d( const char* uniform, const GLdouble& d )
  {
    glUniform1d( uniforms[uniform], d );
  }

  void ShaderProgram::setUniformMatrix4fv( const char *uniform, const GLsizei &count, const GLboolean &transpose, const GLfloat *v )
  {
    glUniformMatrix4fv( uniforms[uniform], count, transpose, v );
  }

  GLuint ShaderProgram::getUBO( const char* name )
  {
    return glGetUniformBlockIndex( id, name );
  }

  // Shaders

  Shaders::Shaders( EnginePtr engine ): engine_( move( engine ) )
  {
  }

  void Shaders::initialize()
  {
    GLboolean hasCompiler;
    glGetBooleanv( GL_SHADER_COMPILER, &hasCompiler );
    engine_->console()->printf( Console::srcGfx, "Shader compiler supported: %s", hasCompiler == GL_TRUE ? "yes" : "no" );
    if ( hasCompiler != GL_TRUE )
      NEKO_EXCEPT( "Shader compiler is not present on this platform" );

    utfString rootDirectory = platform::getCurrentDirectory();
    rootDirectory.append( R"(\data\shaders\)" );

    shaders_.emplace_back( Shader::Vertex, rootDirectory + "default_vp.glsl" );
    shaders_.emplace_back( Shader::Fragment, rootDirectory + "default_fp.glsl" );

    for ( auto& shader : shaders_ )
      compileShader( shader );

    programs_.emplace_back( "default", 0, 1 );

    for ( auto& program : programs_ )
    {
      linkProgram( program );
      program.uniforms["model"] = glGetUniformLocation( program.id, "model" );
      program.uniforms["view"] = glGetUniformLocation( program.id, "view" );
      program.uniforms["projection"] = glGetUniformLocation( program.id, "projection" );
    }
  }

  void Shaders::shutdown()
  {
    for ( auto& shader : shaders_ )
      glDeleteShader( shader.id_ );
    for ( auto& program : programs_ )
      glDeleteProgram( program.id );
  }

  void Shaders::setMatrices( const mat4& model, const mat4& view, const mat4& projection )
  {
    model_ = model;
    view_ = view;
    projection_ = projection;
  }

  ShaderProgram& Shaders::get( size_t program )
  {
    return programs_[program];
  }

  ShaderProgram& Shaders::use( size_t program )
  {
    glUseProgram( programs_[program].id );
    programs_[program].setUniformMatrix4fv( "model", 1, GL_FALSE, glm::value_ptr( model_ ) );
    programs_[program].setUniformMatrix4fv( "view", 1, GL_FALSE, glm::value_ptr( view_ ) );
    programs_[program].setUniformMatrix4fv( "projection", 1, GL_FALSE, glm::value_ptr( projection_ ) );
    return programs_[program];
  }

  void Shaders::dumpLog( const GLuint &target, const bool isProgram )
  {
    GLint blen = 0;
    if ( isProgram )
      glGetProgramiv( target, GL_INFO_LOG_LENGTH, &blen );
    else
      glGetShaderiv( target, GL_INFO_LOG_LENGTH, &blen );
    if ( blen > 1 )
    {
      GLsizei slen = 0;
      auto compiler_log = (GLchar*)Locator::getMemory().alloc( Memory::Sector_Graphics, blen );
      glGetInfoLogARB( target, blen, &slen, compiler_log );
      auto log = string( compiler_log );
      Locator::getMemory().free( Memory::Sector_Graphics, compiler_log );
      engine_->console()->printf( Console::srcGfx, "GL Shader error: %s", log.c_str() );
    }
  }

  void Shaders::compileShader( GLenum type, const string& source, GLuint& shader_out )
  {
    switch ( type )
    {
      case GL_VERTEX_SHADER:
      case GL_FRAGMENT_SHADER:
      case GL_GEOMETRY_SHADER:
        break;
      default:
        NEKO_EXCEPT( "Invalid shader type" );
        break;
    }

    if ( source.empty() )
      NEKO_EXCEPT( "Shader source is empty" );

    shader_out = glCreateShader( type );
    if ( shader_out == 0 )
      NEKO_EXCEPT( "Unable to create shader" );

    auto src = (const GLchar*)source.c_str();
    glShaderSource( shader_out, 1, &src, nullptr );
    glCompileShader( shader_out );

    GLint isCompiled;
    glGetShaderiv( shader_out, GL_COMPILE_STATUS, &isCompiled );
    if ( isCompiled != GL_TRUE )
    {
      dumpLog( shader_out );
      glDeleteShader( shader_out );
      shader_out = 0;
      NEKO_EXCEPT( "GL shader compilation failed" );
    }
  }

  void Shaders::compileShader( Shader& shader )
  {
    engine_->console()->printf( Console::srcGfx,
        "Compiling %s shader %s",
      cShaderTypes[shader.type_].name.c_str(),
        shader.filename_.c_str() );

    string source;
    platform::FileReader file( shader.filename_ );
    source.resize( file.size() + 1 );
    file.read( &source[0], (uint32_t)file.size() );
    source[file.size()] = '\0';
    compileShader( cShaderTypes[shader.type_].glType, source, shader.id_ );
  }

  void Shaders::linkProgram( GLuint vertex, GLuint fragment, GLuint& program_out )
  {
    if ( !glIsShader( vertex ) || !glIsShader( fragment ) )
      NEKO_EXCEPT( "Invalid vertex or fragment shader index" );

    program_out = glCreateProgram();
    if ( program_out == 0 )
      NEKO_EXCEPT( "Failed to create program" );

    glAttachShader( program_out, vertex );
    glAttachShader( program_out, fragment );
    glLinkProgram( program_out );

    GLint linked = GL_FALSE;
    glGetProgramiv( program_out, GL_LINK_STATUS, &linked );
    if ( linked != GL_TRUE )
    {
      dumpLog( program_out, true );
      glDeleteProgram( program_out );
      program_out = 0;
      NEKO_EXCEPT( "GL program link failed" );
    }
  }

  void Shaders::linkProgram( ShaderProgram& program )
  {
    engine_->console()->printf( Console::srcGfx,
        "Linking program: %s", program.name.c_str() );

    linkProgram( shaders_[program.vp].id_, shaders_[program.fp].id_, program.id );
  }

  Shaders::~Shaders()
  {
  }

}