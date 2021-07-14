#include "stdafx.h"
#include "shaders.h"
#include "engine.h"
#include "console.h"
#include "locator.h"
#include "memory.h"
#include "neko_exception.h"

namespace neko {

  using namespace gl;

  namespace shaders {

    struct ShaderMapper
    {
      Type type;
      GLenum glType;
      UseProgramStageMask maskBit;
      string name;
    };

    const ShaderMapper cShaderTypes[Max_Shader_Type] =
    {
      { Shader_Vertex, GL_VERTEX_SHADER, GL_VERTEX_SHADER_BIT, "vertex" },
      { Shader_Fragment, GL_FRAGMENT_SHADER, GL_FRAGMENT_SHADER_BIT, "fragment" },
      { Shader_Geometry, GL_GEOMETRY_SHADER, GL_GEOMETRY_SHADER_BIT, "geometry" },
      { Shader_TesselationControl, GL_TESS_CONTROL_SHADER, GL_TESS_CONTROL_SHADER_BIT, "tess_ctrl" },
      { Shader_TesselationEvaluation, GL_TESS_EVALUATION_SHADER, GL_TESS_EVALUATION_SHADER_BIT, "tess_eval" },
      { Shader_Compute, GL_COMPUTE_SHADER, GL_COMPUTE_SHADER_BIT, "compute" },
      { Shader_Mesh, GL_MESH_SHADER_NV, GL_MESH_SHADER_BIT_NV, "mesh" },
      { Shader_Task, GL_TASK_SHADER_NV, GL_TASK_SHADER_BIT_NV, "task" }
    };

    Shaders::Shaders( ConsolePtr console ): console_( move( console ) )
    {
    }

    void Shaders::initialize()
    {
      GLboolean hasCompiler;
      glGetBooleanv( GL_SHADER_COMPILER, &hasCompiler );
      console_->printf( Console::srcGfx, "Shader compiler supported: %s", hasCompiler == GL_TRUE ? "yes" : "no" );
      if ( hasCompiler != GL_TRUE )
        NEKO_EXCEPT( "Shader compiler is not present on this platform" );

      rootFilePath_ = platform::getCurrentDirectory();
      rootFilePath_.append( R"(\data\shaders\)" );

      world_ = make_unique<PersistentBuffer<World>>();

      //createSimplePipeline( "default2d", "default2d.vert", "default2d.frag" );
      createSimplePipeline( "default3d", "default3d.vert", "default3d.frag" );
      createSimplePipeline( "mainframebuf2d", "mainframebuf2d.vert", "mainframebuf2d.frag" );
      createSimplePipeline( "text3d", "text3d.vert", "text3d.frag" );
      createSimplePipeline( "mygui3d", "mygui3d.vert", "mygui3d.frag" );
    }

    // Shader

    Shader::Shader( Type type ): type_( type ), id_( 0 ), compiled_( false )
    {
      id_ = glCreateShader( cShaderTypes[type].glType );
      if ( id_ == 0 )
        NEKO_EXCEPT( "Shader creation failed" );
    }

    Shader::~Shader()
    {
      glDeleteShader( id_ );
    }

    // Program

    Program::Program(): id_( 0 ), linked_( false )
    {
      id_ = glCreateProgram();
      if ( id_ == 0 )
        NEKO_EXCEPT( "Program creation failed" );
      glProgramParameteri( id_, GL_PROGRAM_SEPARABLE, GL_TRUE );
    }

    GLuint Program::getUBO( const char* name )
    {
      assert( linked_ );
      return glGetUniformBlockIndex( id_, name );
    }

    Program::~Program()
    {
      glDeleteProgram( id_ );
    }

    // Pipeline

    Pipeline::Pipeline( const utf8String& name ): id_( 0 ), name_( name )
    {
      glCreateProgramPipelines( 1, &id_ );
      if ( id_ == 0 )
        NEKO_EXCEPT( "Pipeline creation failed" );
      glObjectLabel( GL_PROGRAM_PIPELINE, id_, (GLsizei)name.size(), name_.c_str() );
    }

    void Pipeline::setProgramStage( Type stage, GLuint id )
    {
      glUseProgramStages( id_, cShaderTypes[stage].maskBit, id );
    }

    Pipeline::~Pipeline()
    {
      glDeleteProgramPipelines( 1, &id_ );
    }

    // Shaders

    void Shaders::shutdown()
    {
      pipelines_.clear();
      programs_.clear();
      shaders_.clear();
    }

    void Shaders::dumpLog( const GLuint& target, const bool isProgram )
    {
      GLint blen = 0;
      if ( isProgram )
        glGetProgramiv( target, GL_INFO_LOG_LENGTH, &blen );
      else
        glGetShaderiv( target, GL_INFO_LOG_LENGTH, &blen );
      if ( blen > 1 )
      {
        GLsizei slen = 0;
        auto compiler_log = (GLchar*)Locator::memory().alloc( Memory::Sector::Graphics, blen );
        glGetInfoLogARB( target, blen, &slen, compiler_log );
        auto log = string( compiler_log );
        Locator::memory().free( Memory::Sector::Graphics, compiler_log );
        console_->printf( Console::srcGfx, "GL Shader error: %s", log.c_str() );
      }
    }

    void Shaders::compileShader( Shader& shader, const string_view source )
    {
      if ( shader.type() >= Max_Shader_Type )
        NEKO_EXCEPT( "Invalid shader type" );

      if ( source.empty() )
        NEKO_EXCEPT( "Shader source is empty" );

      auto src = (const GLchar*)source.data();
      glShaderSource( shader.id(), 1, &src, nullptr );
      glCompileShader( shader.id() );

      GLint compiled = 0;
      glGetShaderiv( shader.id(), GL_COMPILE_STATUS, &compiled );
      shader.compiled_ = ( compiled ? true : false );
      if ( !compiled )
      {
        dumpLog( shader.id(), false );
        NEKO_EXCEPT( "GL shader compilation failed" );
      }
    }

    void Shaders::linkSingleProgram( Program& program, Shader& shader )
    {
      if ( !shader.ready() )
        NEKO_EXCEPT( "Passed shader is not ready" );

      glAttachShader( program.id(), shader.id() );
      glLinkProgram( program.id() );
      // I think it's fine to detach here?
      // Technically we could even delete the shader after this
      // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glCreateShaderProgram.xhtml
      glDetachShader( program.id(), shader.id() );

      GLint linked = 0;
      glGetProgramiv( program.id(), GL_LINK_STATUS, &linked );
      program.linked_ = ( linked ? true : false );
      if ( !linked )
      {
        dumpLog( program.id(), true );
        NEKO_EXCEPT( "GL program linking failed" );
      }

      program.uniforms_["model"] = glGetUniformLocation( program.id(), "model" );
      program.uniforms_["tex"] = glGetUniformLocation( program.id(), "tex" );
      program.uniforms_["yscale"] = glGetUniformLocation( program.id(), "yscale" );
    }

    utf8String Shaders::loadSource( const utf8String& filename )
    {
      utf8String source;
      platform::FileReader file( rootFilePath_ + filename );
      source.resize( file.size() + 1 );
      file.read( &source[0], (uint32_t)file.size() );
      source[file.size()] = '\0';
      return move( source );
    }

    void Shaders::createSimplePipeline( const utf8String& name, const utf8String& vp_filename, const utf8String& fp_filename )
    {
      assert( pipelines_.find( name ) == pipelines_.end() );

      auto vertexSource = loadSource( vp_filename );

      console_->printf( Console::srcGfx, "Compiling %s shader: %s",
        cShaderTypes[Type::Shader_Vertex].name.c_str(), name.c_str() );

      auto vs = make_unique<Shader>( Type::Shader_Vertex );
      compileShader( *vs, vertexSource );
      auto vp = make_shared<Program>();
      linkSingleProgram( *vp, *vs );

      auto fragmentSource = loadSource( fp_filename );

      console_->printf( Console::srcGfx, "Compiling %s shader: %s",
        cShaderTypes[Type::Shader_Fragment].name.c_str(), name.c_str() );

      auto fs = make_unique<Shader>( Type::Shader_Fragment );
      compileShader( *fs, fragmentSource );
      auto fp = make_shared<Program>();
      linkSingleProgram( *fp, *fs );

      auto pipeline = make_unique<Pipeline>( name );
      glUseProgramStages( pipeline->id(), cShaderTypes[vs->type()].maskBit, vp->id() );
      glUseProgramStages( pipeline->id(), cShaderTypes[fs->type()].maskBit, fp->id() );

      pipeline->stages_[Type::Shader_Vertex] = vp;
      pipeline->stages_[Type::Shader_Fragment] = fp;

      shaders_.push_back( move( vs ) );
      shaders_.push_back( move( fs ) );
      programs_.push_back( move( vp ) );
      programs_.push_back( move( fp ) );
      pipelines_[name] = move( pipeline );
    }

    Pipeline& Shaders::usePipeline( const utf8String& name )
    {
      assert( pipelines_.find( name ) != pipelines_.end() );
      auto& pipeline = ( *pipelines_[name] );
      glBindProgramPipeline( pipeline.id_ );
      glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, world_->id() );

      return pipeline;
    }

    Shaders::~Shaders()
    {
    }

  }

}