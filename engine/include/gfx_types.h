#pragma once
#include <glbinding/gl45core/gl.h>
#include <glbinding/glbinding.h>

namespace neko {

  using gl::GLuint;
  using gl::GLsizei;
  using gl::GLfloat;
  using gl::GLboolean;
  using gl::GLenum;
  using gl::GLchar;
  using gl::GLsizei;
  using gl::GLdouble;
  using gl::GLint;

  //! Some forwards for hinting at usage despite same underlying datatype.
  using GLGraphicsFormat = gl::GLenum;
  using GLWrapMode = gl::GLenum;

#pragma pack( push, 1 )

  struct Vertex2D
  {
    static const size_t element_count = 4;
    float x, y; //!< Vertex coordinates
    float s, t; //!< Texture coordinates
    Vertex2D(): x( 0.0f ), y( 0.0f ), s( 0.0f ), t( 0.0f ) {}
    Vertex2D( float x_, float y_, float s_, float t_ ): x( x_ ), y( y_ ), s( s_ ), t( t_ ) {}
  };

  struct Vertex3D
  {
    static const size_t element_count = 19;
    vec3 position; //!< 0: Vertex coordinates
    vec3 normal; //!< 1: Vertex normal
    vec2 texcoord; //!< 2: UV coordinates
    vec4 color; //!< 3: Vertex color
    vec4 tangent; //!< 4: Tangent
    vec3 bitangent; //!< 5: Bitangent
    Vertex3D(): position{ 0.0f }, normal{ 0.0f }, texcoord{ 0.0f }, tangent{ 0.0f }, bitangent{ 0.0f }, color{ 0.0f } {}
    Vertex3D( float x_, float y_, float z_, float nx_, float ny_, float nz_, float s_, float t_, float r_, float g_, float b_, float a_ ):
      position( x_, y_, z_ ), normal( nx_, ny_, nz_ ), texcoord( s_, t_ ), color( r_, g_, b_, a_ ), tangent{ 0.0f }, bitangent{ 0.0f } {}
    Vertex3D( vec3 position_, vec3 normal_, vec2 texcoord_, vec4 color_ ):
      position( position_ ), normal( normal_ ), texcoord( texcoord_ ), color( color_ ), tangent{ 0.0f }, bitangent{ 0.0f } {}
  };

  struct VertexPointParticle
  {
    vec3 pos;
    glm::f32quat orient;
    vec3 size;
    vec4 color;
  };

  struct VertexLine
  {
    vec3 pos;
    vec4 color;
    VertexLine( const vec3& p, const vec4& c )
        : pos( p ), color( c ) {}
  };

  struct PixelRGBA
  {
    uint8_t r, g, b, a;
  };

#pragma pack( pop )

  enum PixelFormat
  {
    PixFmtColorRGB8,
    PixFmtColorRGBA8,
    PixFmtColorRGBA16f,
    PixFmtColorRGBA32f,
    PixFmtDepth32f,
    PixFmtDepth24,
    PixFmtDepth16,
    PixFmtDepth24Stencil8,
    PixFmtColorR8,
    PixFmtColorRG8,
    PixFmtColorR16f
  };

  class Pixmap : public nocopy {
  private:
    int width_ = 0;
    int height_ = 0;
    PixelFormat format_;
    vector<uint8_t> data_;
  public:
    Pixmap( PixelFormat fmt );
    Pixmap( int width, int height, PixelFormat fmt, const uint8_t* data );
    Pixmap( const Pixmap& from, int x, int y, int width, int height );
    Pixmap( Pixmap&& rhs ) noexcept
    {
      width_ = rhs.width_;
      height_ = rhs.height_;
      format_ = rhs.format_;
      data_.swap( rhs.data_ );
    }
    static Pixmap from( const Pixmap& rhs );
    static Pixmap fromPNG( const vector<uint8_t>& input );
    static Pixmap fromEXR( const vector<uint8_t>& input );
    static Pixmap fromTIFF( const utf8String& filename );
    inline bool empty() const noexcept { return data_.empty(); }
    inline int width() const noexcept { return width_; }
    inline int height() const noexcept { return height_; }
    inline PixelFormat format() const noexcept { return format_; }
    inline const vector<uint8_t>& data() const noexcept { return data_; }
    void writePNG( const utf8String& filename ) const;
    void reset();
    void flipVertical();
  };

  using PixmapPtr = shared_ptr<Pixmap>;

  enum GLFormatSizeFlagBits
  {
    GL_FORMAT_SIZE_PACKED_BIT = 0x00000001,
    GL_FORMAT_SIZE_COMPRESSED_BIT = 0x00000002,
    GL_FORMAT_SIZE_PALETTIZED_BIT = 0x00000004,
    GL_FORMAT_SIZE_DEPTH_BIT = 0x00000008,
    GL_FORMAT_SIZE_STENCIL_BIT = 0x00000010,
  };

  struct GLFormatSize
  {
    uint32_t flags;
    unsigned int paletteSizeInBits; // For KTX1.
    unsigned int blockSizeInBits;
    unsigned int blockWidth; // in texels
    unsigned int blockHeight; // in texels
    unsigned int blockDepth; // in texels
    unsigned int minBlocksX; // Minimum required number of blocks
    unsigned int minBlocksY;
  };

  gl::GLenum glGetFormatFromInternalFormat( const gl::GLenum internalFormat );
  gl::GLenum glGetTypeFromInternalFormat( const gl::GLenum internalFormat );
  int glGetTypeSizeFromType( gl::GLenum type );
  GLFormatSize glGetFormatSize( const gl::GLenum internalFormat );

}