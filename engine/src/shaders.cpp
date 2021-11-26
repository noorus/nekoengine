#include "stdafx.h"
#include "shaders.h"
#include "engine.h"
#include "console.h"
#include "locator.h"
#include "memory.h"
#include "neko_exception.h"

namespace neko {

  using namespace gl;

  const wchar_t c_shadersBaseDirectory[] = LR"(\shaders\)";

  namespace shaders {

    struct ShaderMapper
    {
      Type type;
      GLenum glType;
      UseProgramStageMask maskBit;
      string name;
    };

    const ShaderMapper c_shaderTypes[Max_Shader_Type] =
    {
      { Shader_Vertex, GL_VERTEX_SHADER, GL_VERTEX_SHADER_BIT, "vertex" },
      { Shader_Fragment, GL_FRAGMENT_SHADER, GL_FRAGMENT_SHADER_BIT, "fragment" },
      { Shader_Geometry, GL_GEOMETRY_SHADER, GL_GEOMETRY_SHADER_BIT, "geometry" },
      { Shader_TessellationControl, GL_TESS_CONTROL_SHADER, GL_TESS_CONTROL_SHADER_BIT, "tess_ctrl" },
      { Shader_TessellationEvaluation, GL_TESS_EVALUATION_SHADER, GL_TESS_EVALUATION_SHADER_BIT, "tess_eval" },
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
      rootFilePath_.append( c_shadersBaseDirectory );

      world_ = make_unique<PersistentBuffer<neko::uniforms::World>>();
      processing_ = make_unique<PersistentBuffer<neko::uniforms::Processing>>();

      // Includes
      loadInclude( "inc.buffers.glsl" );
      loadInclude( "inc.colorutils.glsl" );
      loadInclude( "inc.math.glsl" );

      // Default pipelines
      createSimplePipeline( "mainframebuf2d", "passthrough2d.vert", "mainframebuf2d.frag", { "hdr", "exposure", "gamma", "texMain", "texGBuffer" } );
      createSimplePipeline( "passthrough2d", "passthrough2d.vert", "passthrough2d.frag", { "texMain", "texSecondary" } );
      createSimplePipeline( "gaussblur2d", "passthrough2d.vert", "gaussblur2d.frag", { "tex_image", "horizontal" } );
      createSimplePipeline( "gui", "gui.vert", "gui.frag", { "yscale", "tex" } );
      createSimplePipeline( "dbg_showvertexnormals", "dbg_showvertexnormals.vert", "dbg_showvertexnormals.geom", "dbg_showvertexnormals.frag", { "model" } );
      createSimplePipeline( "dbg_line", "dbg_line.vert", "dbg_line.frag", { "model" } );
      createSimplePipeline( "dbg_showvertextangents", "dbg_showvertextangents.vert", "dbg_showvertextangents.geom", "dbg_showvertextangents.frag", { "model" } );
      createSimplePipeline( "mat_unlit", "mat_unlitdefault.vert", "mat_unlitdefault.frag", { "model", "gamma", "tex" } );
      createSimplePipeline( "particle_world", "particle_world.vert", "particle_world.geom", "particle.frag", { "model", "tex" } );
      createSimplePipeline( "particle_billboard", "particle_billboard.vert", "particle_billboard.geom", "particle.frag", { "model", "tex" } );
      createSimplePipeline( "mat_skybox", "mat_skybox.vert", "mat_skybox.frag", { "model", "gamma", "exposure", "tex" } );
      createSimplePipeline( "mat_screentone", "mat_screentone.vert", "mat_screentone.frag", { "model" } );
      createSimplePipeline( "text2d", "text_2d.vert", "text.frag", { "model", "tex" } );
      createSimplePipeline( "text3d", "text_3d.vert", "text.frag", { "model", "tex" } );
    }

    // Shader

    Shader::Shader( Type type ): type_( type ), id_( 0 ), compiled_( false )
    {
      id_ = glCreateShader( c_shaderTypes[type].glType );
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
      glUseProgramStages( id_, c_shaderTypes[stage].maskBit, id );
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

    void Shaders::linkSingleProgram( Program& program, Shader& shader, const vector<utf8String>& uniforms )
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

      for ( const auto& uni : uniforms )
        program.uniforms_[uni.c_str()] = glGetUniformLocation( program.id(), uni.c_str() );
    }

    utf8String Shaders::loadSource( const utf8String& filename, int includeDepth )
    {
      if ( includeDepth > 3 )
      {
        NEKO_EXCEPT( "Maximum include depth exceeded; circular dependency?" );
      }

      utf8String source;
      platform::FileReader file( rootFilePath_ + platform::utf8ToWide( filename ) );
      source.resize( file.size() );
      file.read( &source[0], (uint32_t)file.size() );
      stringstream ss( source );
      utf8String line;
      utf8String out;
      int ifdefDepth = 0;
      bool skip = false;
      const std::regex rgx( "^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*" );
      while ( std::getline( ss, line ) )
      {
        if ( line.find( "#ifdef __cplusplus" ) != line.npos )
        {
          ifdefDepth++;
          skip = true;
        }
        else if ( line.find( "#else" ) != line.npos && ifdefDepth > 0 )
        {
          skip = false;
        }
        else if ( line.find( "#endif" ) != line.npos && ifdefDepth > 0 )
        {
          ifdefDepth--;
          skip = false;
        }
        else if ( !skip )
        {
          std::smatch matches;
          if ( std::regex_search( line, matches, rgx ) )
          {
            if ( includes_.find( matches[1].str() ) == includes_.end() )
              NEKO_EXCEPT( "Include not found: " + matches[1].str() )
            out.append( includes_.at( matches[1].str() ) + "\n" );
          }
          else
          {
            out.append( line + "\n" );
          }
        }
      }

      return move( out );
    }

    void Shaders::loadInclude( utf8String filename )
    {
      auto source = loadSource( filename );
      source[source.length() - 1] = '\n';
      includes_[filename] = source;
    }

    void Shaders::buildSeparableProgram( const utf8String& name,
    const utf8String& filename, Type type, ShaderPtr& shader, ProgramPtr& program, const vector<utf8String>& uniforms )
    {
      auto source = loadSource( filename );

      console_->printf( Console::srcGfx, "Compiling %s shader: %s", c_shaderTypes[type].name.c_str(), name.c_str() );

      shader = make_unique<Shader>( type );
      compileShader( *shader, source );
      program = make_shared<Program>();
      linkSingleProgram( *program, *shader, uniforms );
    }

    void Shaders::createSimplePipeline( const utf8String& name, const utf8String& vp_filename, const utf8String& fp_filename, const vector<utf8String>& uniforms )
    {
      assert( pipelines_.find( name ) == pipelines_.end() );

      ShaderPtr vs, fs;
      ProgramPtr vp, fp;

      buildSeparableProgram( name, vp_filename, Type::Shader_Vertex, vs, vp, uniforms );
      buildSeparableProgram( name, fp_filename, Type::Shader_Fragment, fs, fp, uniforms );

      auto pipeline = make_unique<Pipeline>( name );
      glUseProgramStages( pipeline->id(), c_shaderTypes[vs->type()].maskBit, vp->id() );
      glUseProgramStages( pipeline->id(), c_shaderTypes[fs->type()].maskBit, fp->id() );

      pipeline->stages_[Type::Shader_Vertex] = vp;
      pipeline->stages_[Type::Shader_Fragment] = fp;

      shaders_.push_back( move( vs ) );
      shaders_.push_back( move( fs ) );
      programs_.push_back( move( vp ) );
      programs_.push_back( move( fp ) );
      pipelines_[name] = move( pipeline );
    }

    void Shaders::createSimplePipeline( const utf8String& name, const utf8String& vp_filename, const utf8String& gp_filename, const utf8String& fp_filename, const vector<utf8String>& uniforms )
    {
      assert( pipelines_.find( name ) == pipelines_.end() );

      ShaderPtr vs, gs, fs;
      ProgramPtr vp, gp, fp;

      buildSeparableProgram( name, vp_filename, Type::Shader_Vertex, vs, vp, uniforms );
      buildSeparableProgram( name, gp_filename, Type::Shader_Geometry, gs, gp, uniforms );
      buildSeparableProgram( name, fp_filename, Type::Shader_Fragment, fs, fp, uniforms );

      auto pipeline = make_unique<Pipeline>( name );
      glUseProgramStages( pipeline->id(), c_shaderTypes[vs->type()].maskBit, vp->id() );
      glUseProgramStages( pipeline->id(), c_shaderTypes[gs->type()].maskBit, gp->id() );
      glUseProgramStages( pipeline->id(), c_shaderTypes[fs->type()].maskBit, fp->id() );

      pipeline->stages_[Type::Shader_Vertex] = vp;
      pipeline->stages_[Type::Shader_Geometry] = gp;
      pipeline->stages_[Type::Shader_Fragment] = fp;

      shaders_.push_back( move( vs ) );
      shaders_.push_back( move( gs ) );
      shaders_.push_back( move( fs ) );
      programs_.push_back( move( vp ) );
      programs_.push_back( move( gp ) );
      programs_.push_back( move( fp ) );
      pipelines_[name] = move( pipeline );
    }

    Pipeline& Shaders::usePipeline( const utf8String& name )
    {
      assert( pipelines_.find( name ) != pipelines_.end() );
      auto& pipeline = ( *pipelines_[name] );
      glBindProgramPipeline( pipeline.id_ );
      glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, world_->id() );
      glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, processing_->id() );

      return pipeline;
    }

    Shaders::~Shaders()
    {
    }

  }

}