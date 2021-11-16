#include "stdafx.h"
#include "gfx_types.h"

namespace neko {

  using namespace gl;

  #if !defined( GL_SR8 )
#define GL_SR8 (GLenum)0x8FBD // same as GL_SR8_EXT
#endif
#if !defined( GL_SRG8 )
#define GL_SRG8 (GLenum)0x8FBE // same as GL_SRG8_EXT
#endif

#if !defined( GL_ETC1_RGB8_OES )
#define GL_ETC1_RGB8_OES (GLenum)0x8D64
#endif

  //
  // PVRTC
  //

#if !defined( GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG )
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG (GLenum)0x8C01
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG (GLenum)0x8C00
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG (GLenum)0x8C03
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG (GLenum)0x8C02
#endif
#if !defined( GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG )
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG (GLenum)0x9137
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG (GLenum)0x9138
#endif
#if !defined( GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT )
#define GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT (GLenum)0x8A54
#define GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT (GLenum)0x8A55
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT (GLenum)0x8A56
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT (GLenum)0x8A57
#endif
#if !defined( GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG )
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG (GLenum)0x93F0
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG (GLenum)0x93F1
#endif

#if !defined( GL_COMPRESSED_RGBA_ASTC_3x3x3_OES )
#define GL_COMPRESSED_RGBA_ASTC_3x3x3_OES (GLenum)0x93C0
#define GL_COMPRESSED_RGBA_ASTC_4x3x3_OES (GLenum)0x93C1
#define GL_COMPRESSED_RGBA_ASTC_4x4x3_OES (GLenum)0x93C2
#define GL_COMPRESSED_RGBA_ASTC_4x4x4_OES (GLenum)0x93C3
#define GL_COMPRESSED_RGBA_ASTC_5x4x4_OES (GLenum)0x93C4
#define GL_COMPRESSED_RGBA_ASTC_5x5x4_OES (GLenum)0x93C5
#define GL_COMPRESSED_RGBA_ASTC_5x5x5_OES (GLenum)0x93C6
#define GL_COMPRESSED_RGBA_ASTC_6x5x5_OES (GLenum)0x93C7
#define GL_COMPRESSED_RGBA_ASTC_6x6x5_OES (GLenum)0x93C8
#define GL_COMPRESSED_RGBA_ASTC_6x6x6_OES (GLenum)0x93C9
#endif

#if !defined( GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES )
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES (GLenum)0x93E0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES (GLenum)0x93E1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES (GLenum)0x93E2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES (GLenum)0x93E3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES (GLenum)0x93E4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES (GLenum)0x93E5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES (GLenum)0x93E6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES (GLenum)0x93E7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES (GLenum)0x93E8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES (GLenum)0x93E9
#endif

  //
  // ATC
  //

#if !defined( GL_ATC_RGB_AMD )
#define GL_ATC_RGB_AMD (GLenum)0x8C92
#define GL_ATC_RGBA_EXPLICIT_ALPHA_AMD (GLenum)0x8C93
#define GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD (GLenum)0x87EE
#endif

  GLenum glGetFormatFromInternalFormat( const GLenum internalFormat )
  {
    switch ( internalFormat )
    {
      //
      // 8 bits per component
      //
      case GL_R8: return GL_RED; // 1-component, 8-bit unsigned normalized
      case GL_RG8: return GL_RG; // 2-component, 8-bit unsigned normalized
      case GL_RGB8: return GL_RGB; // 3-component, 8-bit unsigned normalized
      case GL_RGBA8: return GL_RGBA; // 4-component, 8-bit unsigned normalized

      case GL_R8_SNORM: return GL_RED; // 1-component, 8-bit signed normalized
      case GL_RG8_SNORM: return GL_RG; // 2-component, 8-bit signed normalized
      case GL_RGB8_SNORM: return GL_RGB; // 3-component, 8-bit signed normalized
      case GL_RGBA8_SNORM: return GL_RGBA; // 4-component, 8-bit signed normalized

      case GL_R8UI: return GL_RED; // 1-component, 8-bit unsigned integer
      case GL_RG8UI: return GL_RG; // 2-component, 8-bit unsigned integer
      case GL_RGB8UI: return GL_RGB; // 3-component, 8-bit unsigned integer
      case GL_RGBA8UI: return GL_RGBA; // 4-component, 8-bit unsigned integer

      case GL_R8I: return GL_RED; // 1-component, 8-bit signed integer
      case GL_RG8I: return GL_RG; // 2-component, 8-bit signed integer
      case GL_RGB8I: return GL_RGB; // 3-component, 8-bit signed integer
      case GL_RGBA8I: return GL_RGBA; // 4-component, 8-bit signed integer

      case GL_SR8: return GL_RED; // 1-component, 8-bit sRGB
      case GL_SRG8: return GL_RG; // 2-component, 8-bit sRGB
      case GL_SRGB8: return GL_RGB; // 3-component, 8-bit sRGB
      case GL_SRGB8_ALPHA8: return GL_RGBA; // 4-component, 8-bit sRGB

      //
      // 16 bits per component
      //
      case GL_R16: return GL_RED; // 1-component, 16-bit unsigned normalized
      case GL_RG16: return GL_RG; // 2-component, 16-bit unsigned normalized
      case GL_RGB16: return GL_RGB; // 3-component, 16-bit unsigned normalized
      case GL_RGBA16: return GL_RGBA; // 4-component, 16-bit unsigned normalized

      case GL_R16_SNORM: return GL_RED; // 1-component, 16-bit signed normalized
      case GL_RG16_SNORM: return GL_RG; // 2-component, 16-bit signed normalized
      case GL_RGB16_SNORM: return GL_RGB; // 3-component, 16-bit signed normalized
      case GL_RGBA16_SNORM: return GL_RGBA; // 4-component, 16-bit signed normalized

      case GL_R16UI: return GL_RED; // 1-component, 16-bit unsigned integer
      case GL_RG16UI: return GL_RG; // 2-component, 16-bit unsigned integer
      case GL_RGB16UI: return GL_RGB; // 3-component, 16-bit unsigned integer
      case GL_RGBA16UI: return GL_RGBA; // 4-component, 16-bit unsigned integer

      case GL_R16I: return GL_RED; // 1-component, 16-bit signed integer
      case GL_RG16I: return GL_RG; // 2-component, 16-bit signed integer
      case GL_RGB16I: return GL_RGB; // 3-component, 16-bit signed integer
      case GL_RGBA16I: return GL_RGBA; // 4-component, 16-bit signed integer

      case GL_R16F: return GL_RED; // 1-component, 16-bit floating-point
      case GL_RG16F: return GL_RG; // 2-component, 16-bit floating-point
      case GL_RGB16F: return GL_RGB; // 3-component, 16-bit floating-point
      case GL_RGBA16F: return GL_RGBA; // 4-component, 16-bit floating-point

      //
      // 32 bits per component
      //
      case GL_R32UI: return GL_RED; // 1-component, 32-bit unsigned integer
      case GL_RG32UI: return GL_RG; // 2-component, 32-bit unsigned integer
      case GL_RGB32UI: return GL_RGB; // 3-component, 32-bit unsigned integer
      case GL_RGBA32UI: return GL_RGBA; // 4-component, 32-bit unsigned integer

      case GL_R32I: return GL_RED; // 1-component, 32-bit signed integer
      case GL_RG32I: return GL_RG; // 2-component, 32-bit signed integer
      case GL_RGB32I: return GL_RGB; // 3-component, 32-bit signed integer
      case GL_RGBA32I: return GL_RGBA; // 4-component, 32-bit signed integer

      case GL_R32F: return GL_RED; // 1-component, 32-bit floating-point
      case GL_RG32F: return GL_RG; // 2-component, 32-bit floating-point
      case GL_RGB32F: return GL_RGB; // 3-component, 32-bit floating-point
      case GL_RGBA32F: return GL_RGBA; // 4-component, 32-bit floating-point

      //
      // Packed
      //
      case GL_R3_G3_B2: return GL_RGB; // 3-component 3:3:2,       unsigned normalized
      case GL_RGB4: return GL_RGB; // 3-component 4:4:4,       unsigned normalized
      case GL_RGB5: return GL_RGB; // 3-component 5:5:5,       unsigned normalized
      case GL_RGB565: return GL_RGB; // 3-component 5:6:5,       unsigned normalized
      case GL_RGB10: return GL_RGB; // 3-component 10:10:10,    unsigned normalized
      case GL_RGB12: return GL_RGB; // 3-component 12:12:12,    unsigned normalized
      case GL_RGBA2: return GL_RGBA; // 4-component 2:2:2:2,     unsigned normalized
      case GL_RGBA4: return GL_RGBA; // 4-component 4:4:4:4,     unsigned normalized
      case GL_RGBA12: return GL_RGBA; // 4-component 12:12:12:12, unsigned normalized
      case GL_RGB5_A1: return GL_RGBA; // 4-component 5:5:5:1,     unsigned normalized
      case GL_RGB10_A2: return GL_RGBA; // 4-component 10:10:10:2,  unsigned normalized
      case GL_RGB10_A2UI: return GL_RGBA; // 4-component 10:10:10:2,  unsigned integer
      case GL_R11F_G11F_B10F: return GL_RGB; // 3-component 11:11:10,    floating-point
      case GL_RGB9_E5: return GL_RGB; // 3-component/exp 9:9:9/5, floating-point

        //
        // S3TC/DXT/BC
        //

      case GL_COMPRESSED_RGB_S3TC_DXT1_EXT: return GL_RGB; // line through 3D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: return GL_RGBA; // line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: return GL_RGBA; // line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: return GL_RGBA; // line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized

      case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT: return GL_RGB; // line through 3D space, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: return GL_RGBA; // line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: return GL_RGBA; // line through 3D space plus line through 1D space, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: return GL_RGBA; // line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB

      case GL_COMPRESSED_LUMINANCE_LATC1_EXT: return GL_RED; // line through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT: return GL_RG; // two lines through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT: return GL_RED; // line through 1D space, 4x4 blocks, signed normalized
      case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT: return GL_RG; // two lines through 1D space, 4x4 blocks, signed normalized

      case GL_COMPRESSED_RED_RGTC1: return GL_RED; // line through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RG_RGTC2: return GL_RG; // two lines through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SIGNED_RED_RGTC1: return GL_RED; // line through 1D space, 4x4 blocks, signed normalized
      case GL_COMPRESSED_SIGNED_RG_RGTC2: return GL_RG; // two lines through 1D space, 4x4 blocks, signed normalized

      case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT: return GL_RGB; // 3-component, 4x4 blocks, unsigned floating-point
      case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT: return GL_RGB; // 3-component, 4x4 blocks, signed floating-point
      case GL_COMPRESSED_RGBA_BPTC_UNORM: return GL_RGBA; // 4-component, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM: return GL_RGBA; // 4-component, 4x4 blocks, sRGB

      //
      // ETC
      //
      case GL_ETC1_RGB8_OES: return GL_RGB; // 3-component ETC1, 4x4 blocks, unsigned normalized

      case GL_COMPRESSED_RGB8_ETC2: return GL_RGB; // 3-component ETC2, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2: return GL_RGBA; // 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA8_ETC2_EAC: return GL_RGBA; // 4-component ETC2, 4x4 blocks, unsigned normalized

      case GL_COMPRESSED_SRGB8_ETC2: return GL_RGB; // 3-component ETC2, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: return GL_RGBA; // 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC: return GL_RGBA; // 4-component ETC2, 4x4 blocks, sRGB

      case GL_COMPRESSED_R11_EAC: return GL_RED; // 1-component ETC, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RG11_EAC: return GL_RG; // 2-component ETC, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SIGNED_R11_EAC: return GL_RED; // 1-component ETC, 4x4 blocks, signed normalized
      case GL_COMPRESSED_SIGNED_RG11_EAC: return GL_RG; // 2-component ETC, 4x4 blocks, signed normalized

      //
      // PVRTC
      //
      case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG: return GL_RGB; // 3-component PVRTC, 16x8 blocks, unsigned normalized
      case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG: return GL_RGB; // 3-component PVRTC,  8x8 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG: return GL_RGBA; // 4-component PVRTC, 16x8 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG: return GL_RGBA; // 4-component PVRTC,  8x8 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG: return GL_RGBA; // 4-component PVRTC,  8x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG: return GL_RGBA; // 4-component PVRTC,  4x4 blocks, unsigned normalized

      case GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT: return GL_RGB; // 3-component PVRTC, 16x8 blocks, sRGB
      case GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT: return GL_RGB; // 3-component PVRTC,  8x8 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT: return GL_RGBA; // 4-component PVRTC, 16x8 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT: return GL_RGBA; // 4-component PVRTC,  8x8 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG: return GL_RGBA; // 4-component PVRTC,  8x4 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG: return GL_RGBA; // 4-component PVRTC,  4x4 blocks, sRGB

      //
      // ASTC
      //
      case GL_COMPRESSED_RGBA_ASTC_4x4_KHR: return GL_RGBA; // 4-component ASTC, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_5x4_KHR: return GL_RGBA; // 4-component ASTC, 5x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_5x5_KHR: return GL_RGBA; // 4-component ASTC, 5x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_6x5_KHR: return GL_RGBA; // 4-component ASTC, 6x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_6x6_KHR: return GL_RGBA; // 4-component ASTC, 6x6 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_8x5_KHR: return GL_RGBA; // 4-component ASTC, 8x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_8x6_KHR: return GL_RGBA; // 4-component ASTC, 8x6 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_8x8_KHR: return GL_RGBA; // 4-component ASTC, 8x8 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_10x5_KHR: return GL_RGBA; // 4-component ASTC, 10x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_10x6_KHR: return GL_RGBA; // 4-component ASTC, 10x6 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_10x8_KHR: return GL_RGBA; // 4-component ASTC, 10x8 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_10x10_KHR: return GL_RGBA; // 4-component ASTC, 10x10 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_12x10_KHR: return GL_RGBA; // 4-component ASTC, 12x10 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_12x12_KHR: return GL_RGBA; // 4-component ASTC, 12x12 blocks, unsigned normalized

      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR: return GL_RGBA; // 4-component ASTC, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR: return GL_RGBA; // 4-component ASTC, 5x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR: return GL_RGBA; // 4-component ASTC, 5x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR: return GL_RGBA; // 4-component ASTC, 6x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR: return GL_RGBA; // 4-component ASTC, 6x6 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR: return GL_RGBA; // 4-component ASTC, 8x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR: return GL_RGBA; // 4-component ASTC, 8x6 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR: return GL_RGBA; // 4-component ASTC, 8x8 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR: return GL_RGBA; // 4-component ASTC, 10x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR: return GL_RGBA; // 4-component ASTC, 10x6 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR: return GL_RGBA; // 4-component ASTC, 10x8 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR: return GL_RGBA; // 4-component ASTC, 10x10 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR: return GL_RGBA; // 4-component ASTC, 12x10 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR: return GL_RGBA; // 4-component ASTC, 12x12 blocks, sRGB

      case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES: return GL_RGBA; // 4-component ASTC, 3x3x3 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES: return GL_RGBA; // 4-component ASTC, 4x3x3 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES: return GL_RGBA; // 4-component ASTC, 4x4x3 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES: return GL_RGBA; // 4-component ASTC, 4x4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES: return GL_RGBA; // 4-component ASTC, 5x4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES: return GL_RGBA; // 4-component ASTC, 5x5x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES: return GL_RGBA; // 4-component ASTC, 5x5x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES: return GL_RGBA; // 4-component ASTC, 6x5x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES: return GL_RGBA; // 4-component ASTC, 6x6x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES: return GL_RGBA; // 4-component ASTC, 6x6x6 blocks, unsigned normalized

      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES: return GL_RGBA; // 4-component ASTC, 3x3x3 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES: return GL_RGBA; // 4-component ASTC, 4x3x3 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES: return GL_RGBA; // 4-component ASTC, 4x4x3 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES: return GL_RGBA; // 4-component ASTC, 4x4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES: return GL_RGBA; // 4-component ASTC, 5x4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES: return GL_RGBA; // 4-component ASTC, 5x5x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES: return GL_RGBA; // 4-component ASTC, 5x5x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES: return GL_RGBA; // 4-component ASTC, 6x5x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES: return GL_RGBA; // 4-component ASTC, 6x6x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES: return GL_RGBA; // 4-component ASTC, 6x6x6 blocks, sRGB

      //
      // ATC
      //
      case GL_ATC_RGB_AMD: return GL_RGB; // 3-component, 4x4 blocks, unsigned normalized
      case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD: return GL_RGBA; // 4-component, 4x4 blocks, unsigned normalized
      case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD: return GL_RGBA; // 4-component, 4x4 blocks, unsigned normalized

      //
      // Palletized
      //
      case GL_PALETTE4_RGB8_OES: return GL_RGB; // 3-component 8:8:8,   4-bit palette, unsigned normalized
      case GL_PALETTE4_RGBA8_OES: return GL_RGBA; // 4-component 8:8:8:8, 4-bit palette, unsigned normalized
      case GL_PALETTE4_R5_G6_B5_OES: return GL_RGB; // 3-component 5:6:5,   4-bit palette, unsigned normalized
      case GL_PALETTE4_RGBA4_OES: return GL_RGBA; // 4-component 4:4:4:4, 4-bit palette, unsigned normalized
      case GL_PALETTE4_RGB5_A1_OES: return GL_RGBA; // 4-component 5:5:5:1, 4-bit palette, unsigned normalized
      case GL_PALETTE8_RGB8_OES: return GL_RGB; // 3-component 8:8:8,   8-bit palette, unsigned normalized
      case GL_PALETTE8_RGBA8_OES: return GL_RGBA; // 4-component 8:8:8:8, 8-bit palette, unsigned normalized
      case GL_PALETTE8_R5_G6_B5_OES: return GL_RGB; // 3-component 5:6:5,   8-bit palette, unsigned normalized
      case GL_PALETTE8_RGBA4_OES: return GL_RGBA; // 4-component 4:4:4:4, 8-bit palette, unsigned normalized
      case GL_PALETTE8_RGB5_A1_OES: return GL_RGBA; // 4-component 5:5:5:1, 8-bit palette, unsigned normalized

      //
      // Depth/stencil
      //
      case GL_DEPTH_COMPONENT16: return GL_DEPTH_COMPONENT;
      case GL_DEPTH_COMPONENT24: return GL_DEPTH_COMPONENT;
      case GL_DEPTH_COMPONENT32: return GL_DEPTH_COMPONENT;
      case GL_DEPTH_COMPONENT32F: return GL_DEPTH_COMPONENT;
      case GL_DEPTH_COMPONENT32F_NV: return GL_DEPTH_COMPONENT;
      case GL_STENCIL_INDEX1: return GL_STENCIL_INDEX;
      case GL_STENCIL_INDEX4: return GL_STENCIL_INDEX;
      case GL_STENCIL_INDEX8: return GL_STENCIL_INDEX;
      case GL_STENCIL_INDEX16: return GL_STENCIL_INDEX;
      case GL_DEPTH24_STENCIL8: return GL_DEPTH_STENCIL;
      case GL_DEPTH32F_STENCIL8: return GL_DEPTH_STENCIL;
      case GL_DEPTH32F_STENCIL8_NV: return GL_DEPTH_STENCIL;

      default: return GL_INVALID_VALUE;
    }
  }

  GLenum glGetTypeFromInternalFormat( const GLenum internalFormat )
  {
    switch ( internalFormat )
    {
      //
      // 8 bits per component
      //
      case GL_R8: return GL_UNSIGNED_BYTE; // 1-component, 8-bit unsigned normalized
      case GL_RG8: return GL_UNSIGNED_BYTE; // 2-component, 8-bit unsigned normalized
      case GL_RGB8: return GL_UNSIGNED_BYTE; // 3-component, 8-bit unsigned normalized
      case GL_RGBA8: return GL_UNSIGNED_BYTE; // 4-component, 8-bit unsigned normalized

      case GL_R8_SNORM: return GL_BYTE; // 1-component, 8-bit signed normalized
      case GL_RG8_SNORM: return GL_BYTE; // 2-component, 8-bit signed normalized
      case GL_RGB8_SNORM: return GL_BYTE; // 3-component, 8-bit signed normalized
      case GL_RGBA8_SNORM: return GL_BYTE; // 4-component, 8-bit signed normalized

      case GL_R8UI: return GL_UNSIGNED_BYTE; // 1-component, 8-bit unsigned integer
      case GL_RG8UI: return GL_UNSIGNED_BYTE; // 2-component, 8-bit unsigned integer
      case GL_RGB8UI: return GL_UNSIGNED_BYTE; // 3-component, 8-bit unsigned integer
      case GL_RGBA8UI: return GL_UNSIGNED_BYTE; // 4-component, 8-bit unsigned integer

      case GL_R8I: return GL_BYTE; // 1-component, 8-bit signed integer
      case GL_RG8I: return GL_BYTE; // 2-component, 8-bit signed integer
      case GL_RGB8I: return GL_BYTE; // 3-component, 8-bit signed integer
      case GL_RGBA8I: return GL_BYTE; // 4-component, 8-bit signed integer

      case GL_SR8: return GL_UNSIGNED_BYTE; // 1-component, 8-bit sRGB
      case GL_SRG8: return GL_UNSIGNED_BYTE; // 2-component, 8-bit sRGB
      case GL_SRGB8: return GL_UNSIGNED_BYTE; // 3-component, 8-bit sRGB
      case GL_SRGB8_ALPHA8: return GL_UNSIGNED_BYTE; // 4-component, 8-bit sRGB

      //
      // 16 bits per component
      //
      case GL_R16: return GL_UNSIGNED_SHORT; // 1-component, 16-bit unsigned normalized
      case GL_RG16: return GL_UNSIGNED_SHORT; // 2-component, 16-bit unsigned normalized
      case GL_RGB16: return GL_UNSIGNED_SHORT; // 3-component, 16-bit unsigned normalized
      case GL_RGBA16: return GL_UNSIGNED_SHORT; // 4-component, 16-bit unsigned normalized

      case GL_R16_SNORM: return GL_SHORT; // 1-component, 16-bit signed normalized
      case GL_RG16_SNORM: return GL_SHORT; // 2-component, 16-bit signed normalized
      case GL_RGB16_SNORM: return GL_SHORT; // 3-component, 16-bit signed normalized
      case GL_RGBA16_SNORM: return GL_SHORT; // 4-component, 16-bit signed normalized

      case GL_R16UI: return GL_UNSIGNED_SHORT; // 1-component, 16-bit unsigned integer
      case GL_RG16UI: return GL_UNSIGNED_SHORT; // 2-component, 16-bit unsigned integer
      case GL_RGB16UI: return GL_UNSIGNED_SHORT; // 3-component, 16-bit unsigned integer
      case GL_RGBA16UI: return GL_UNSIGNED_SHORT; // 4-component, 16-bit unsigned integer

      case GL_R16I: return GL_SHORT; // 1-component, 16-bit signed integer
      case GL_RG16I: return GL_SHORT; // 2-component, 16-bit signed integer
      case GL_RGB16I: return GL_SHORT; // 3-component, 16-bit signed integer
      case GL_RGBA16I: return GL_SHORT; // 4-component, 16-bit signed integer

      case GL_R16F: return GL_HALF_FLOAT; // 1-component, 16-bit floating-point
      case GL_RG16F: return GL_HALF_FLOAT; // 2-component, 16-bit floating-point
      case GL_RGB16F: return GL_HALF_FLOAT; // 3-component, 16-bit floating-point
      case GL_RGBA16F: return GL_HALF_FLOAT; // 4-component, 16-bit floating-point

      //
      // 32 bits per component
      //
      case GL_R32UI: return GL_UNSIGNED_INT; // 1-component, 32-bit unsigned integer
      case GL_RG32UI: return GL_UNSIGNED_INT; // 2-component, 32-bit unsigned integer
      case GL_RGB32UI: return GL_UNSIGNED_INT; // 3-component, 32-bit unsigned integer
      case GL_RGBA32UI: return GL_UNSIGNED_INT; // 4-component, 32-bit unsigned integer

      case GL_R32I: return GL_INT; // 1-component, 32-bit signed integer
      case GL_RG32I: return GL_INT; // 2-component, 32-bit signed integer
      case GL_RGB32I: return GL_INT; // 3-component, 32-bit signed integer
      case GL_RGBA32I: return GL_INT; // 4-component, 32-bit signed integer

      case GL_R32F: return GL_FLOAT; // 1-component, 32-bit floating-point
      case GL_RG32F: return GL_FLOAT; // 2-component, 32-bit floating-point
      case GL_RGB32F: return GL_FLOAT; // 3-component, 32-bit floating-point
      case GL_RGBA32F:
        return GL_FLOAT; // 4-component, 32-bit floating-point

      //
      // Packed
      //
      case GL_R3_G3_B2: return GL_UNSIGNED_BYTE_2_3_3_REV; // 3-component 3:3:2,       unsigned normalized
      case GL_RGB4: return GL_UNSIGNED_SHORT_4_4_4_4; // 3-component 4:4:4,       unsigned normalized
      case GL_RGB5: return GL_UNSIGNED_SHORT_5_5_5_1; // 3-component 5:5:5,       unsigned normalized
      case GL_RGB565: return GL_UNSIGNED_SHORT_5_6_5; // 3-component 5:6:5,       unsigned normalized
      case GL_RGB10: return GL_UNSIGNED_INT_10_10_10_2; // 3-component 10:10:10,    unsigned normalized
      case GL_RGB12: return GL_UNSIGNED_SHORT; // 3-component 12:12:12,    unsigned normalized
      case GL_RGBA2: return GL_UNSIGNED_BYTE; // 4-component 2:2:2:2,     unsigned normalized
      case GL_RGBA4: return GL_UNSIGNED_SHORT_4_4_4_4; // 4-component 4:4:4:4,     unsigned normalized
      case GL_RGBA12: return GL_UNSIGNED_SHORT; // 4-component 12:12:12:12, unsigned normalized
      case GL_RGB5_A1: return GL_UNSIGNED_SHORT_5_5_5_1; // 4-component 5:5:5:1,     unsigned normalized
      case GL_RGB10_A2: return GL_UNSIGNED_INT_2_10_10_10_REV; // 4-component 10:10:10:2,  unsigned normalized
      case GL_RGB10_A2UI: return GL_UNSIGNED_INT_2_10_10_10_REV; // 4-component 10:10:10:2,  unsigned integer
      case GL_R11F_G11F_B10F: return GL_UNSIGNED_INT_10F_11F_11F_REV; // 3-component 11:11:10,    floating-point
      case GL_RGB9_E5:
        return GL_UNSIGNED_INT_5_9_9_9_REV; // 3-component/exp 9:9:9/5, floating-point

        //
        // S3TC/DXT/BC
        //

      case GL_COMPRESSED_RGB_S3TC_DXT1_EXT: return GL_UNSIGNED_BYTE; // line through 3D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: return GL_UNSIGNED_BYTE; // line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: return GL_UNSIGNED_BYTE; // line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: return GL_UNSIGNED_BYTE; // line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized

      case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT: return GL_UNSIGNED_BYTE; // line through 3D space, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: return GL_UNSIGNED_BYTE; // line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: return GL_UNSIGNED_BYTE; // line through 3D space plus line through 1D space, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: return GL_UNSIGNED_BYTE; // line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB

      case GL_COMPRESSED_LUMINANCE_LATC1_EXT: return GL_UNSIGNED_BYTE; // line through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT: return GL_UNSIGNED_BYTE; // two lines through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT: return GL_UNSIGNED_BYTE; // line through 1D space, 4x4 blocks, signed normalized
      case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT: return GL_UNSIGNED_BYTE; // two lines through 1D space, 4x4 blocks, signed normalized

      case GL_COMPRESSED_RED_RGTC1: return GL_UNSIGNED_BYTE; // line through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RG_RGTC2: return GL_UNSIGNED_BYTE; // two lines through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SIGNED_RED_RGTC1: return GL_UNSIGNED_BYTE; // line through 1D space, 4x4 blocks, signed normalized
      case GL_COMPRESSED_SIGNED_RG_RGTC2: return GL_UNSIGNED_BYTE; // two lines through 1D space, 4x4 blocks, signed normalized

      case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT: return GL_FLOAT; // 3-component, 4x4 blocks, unsigned floating-point
      case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT: return GL_FLOAT; // 3-component, 4x4 blocks, signed floating-point
      case GL_COMPRESSED_RGBA_BPTC_UNORM: return GL_UNSIGNED_BYTE; // 4-component, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        return GL_UNSIGNED_BYTE; // 4-component, 4x4 blocks, sRGB

      //
      // ETC
      //
      case GL_ETC1_RGB8_OES: return GL_UNSIGNED_BYTE; // 3-component ETC1, 4x4 blocks, unsigned normalized" ),

      case GL_COMPRESSED_RGB8_ETC2: return GL_UNSIGNED_BYTE; // 3-component ETC2, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2: return GL_UNSIGNED_BYTE; // 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA8_ETC2_EAC: return GL_UNSIGNED_BYTE; // 4-component ETC2, 4x4 blocks, unsigned normalized

      case GL_COMPRESSED_SRGB8_ETC2: return GL_UNSIGNED_BYTE; // 3-component ETC2, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: return GL_UNSIGNED_BYTE; // 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC: return GL_UNSIGNED_BYTE; // 4-component ETC2, 4x4 blocks, sRGB

      case GL_COMPRESSED_R11_EAC: return GL_UNSIGNED_BYTE; // 1-component ETC, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RG11_EAC: return GL_UNSIGNED_BYTE; // 2-component ETC, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SIGNED_R11_EAC: return GL_UNSIGNED_BYTE; // 1-component ETC, 4x4 blocks, signed normalized
      case GL_COMPRESSED_SIGNED_RG11_EAC:
        return GL_UNSIGNED_BYTE; // 2-component ETC, 4x4 blocks, signed normalized

      //
      // PVRTC
      //
      case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG: return GL_UNSIGNED_BYTE; // 3-component PVRTC, 16x8 blocks, unsigned normalized
      case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG: return GL_UNSIGNED_BYTE; // 3-component PVRTC,  8x8 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG: return GL_UNSIGNED_BYTE; // 4-component PVRTC, 16x8 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG: return GL_UNSIGNED_BYTE; // 4-component PVRTC,  8x8 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG: return GL_UNSIGNED_BYTE; // 4-component PVRTC,  8x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG: return GL_UNSIGNED_BYTE; // 4-component PVRTC,  4x4 blocks, unsigned normalized

      case GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT: return GL_UNSIGNED_BYTE; // 3-component PVRTC, 16x8 blocks, sRGB
      case GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT: return GL_UNSIGNED_BYTE; // 3-component PVRTC,  8x8 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT: return GL_UNSIGNED_BYTE; // 4-component PVRTC, 16x8 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT: return GL_UNSIGNED_BYTE; // 4-component PVRTC,  8x8 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG: return GL_UNSIGNED_BYTE; // 4-component PVRTC,  8x4 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG:
        return GL_UNSIGNED_BYTE; // 4-component PVRTC,  4x4 blocks, sRGB

      //
      // ASTC
      //
      case GL_COMPRESSED_RGBA_ASTC_4x4_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_5x4_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 5x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_5x5_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 5x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_6x5_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 6x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_6x6_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 6x6 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_8x5_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 8x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_8x6_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 8x6 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_8x8_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 8x8 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_10x5_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 10x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_10x6_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 10x6 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_10x8_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 10x8 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_10x10_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 10x10 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_12x10_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 12x10 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_12x12_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 12x12 blocks, unsigned normalized

      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 5x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 5x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 6x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 6x6 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 8x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 8x6 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 8x8 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 10x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 10x6 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 10x8 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 10x10 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 12x10 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR: return GL_UNSIGNED_BYTE; // 4-component ASTC, 12x12 blocks, sRGB

      case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 3x3x3 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 4x3x3 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 4x4x3 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 4x4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 5x4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 5x5x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 5x5x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 6x5x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 6x6x5 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 6x6x6 blocks, unsigned normalized

      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 3x3x3 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 4x3x3 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 4x4x3 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 4x4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 5x4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 5x5x4 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 5x5x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 6x5x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES: return GL_UNSIGNED_BYTE; // 4-component ASTC, 6x6x5 blocks, sRGB
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:
        return GL_UNSIGNED_BYTE; // 4-component ASTC, 6x6x6 blocks, sRGB

      //
      // ATC
      //
      case GL_ATC_RGB_AMD: return GL_UNSIGNED_BYTE; // 3-component, 4x4 blocks, unsigned normalized
      case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD: return GL_UNSIGNED_BYTE; // 4-component, 4x4 blocks, unsigned normalized
      case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD:
        return GL_UNSIGNED_BYTE; // 4-component, 4x4 blocks, unsigned normalized

      //
      // Palletized
      //
      case GL_PALETTE4_RGB8_OES: return GL_UNSIGNED_BYTE; // 3-component 8:8:8,   4-bit palette, unsigned normalized
      case GL_PALETTE4_RGBA8_OES: return GL_UNSIGNED_BYTE; // 4-component 8:8:8:8, 4-bit palette, unsigned normalized
      case GL_PALETTE4_R5_G6_B5_OES: return GL_UNSIGNED_SHORT_5_6_5; // 3-component 5:6:5,   4-bit palette, unsigned normalized
      case GL_PALETTE4_RGBA4_OES: return GL_UNSIGNED_SHORT_4_4_4_4; // 4-component 4:4:4:4, 4-bit palette, unsigned normalized
      case GL_PALETTE4_RGB5_A1_OES: return GL_UNSIGNED_SHORT_5_5_5_1; // 4-component 5:5:5:1, 4-bit palette, unsigned normalized
      case GL_PALETTE8_RGB8_OES: return GL_UNSIGNED_BYTE; // 3-component 8:8:8,   8-bit palette, unsigned normalized
      case GL_PALETTE8_RGBA8_OES: return GL_UNSIGNED_BYTE; // 4-component 8:8:8:8, 8-bit palette, unsigned normalized
      case GL_PALETTE8_R5_G6_B5_OES: return GL_UNSIGNED_SHORT_5_6_5; // 3-component 5:6:5,   8-bit palette, unsigned normalized
      case GL_PALETTE8_RGBA4_OES: return GL_UNSIGNED_SHORT_4_4_4_4; // 4-component 4:4:4:4, 8-bit palette, unsigned normalized
      case GL_PALETTE8_RGB5_A1_OES:
        return GL_UNSIGNED_SHORT_5_5_5_1; // 4-component 5:5:5:1, 8-bit palette, unsigned normalized

      //
      // Depth/stencil
      //
      case GL_DEPTH_COMPONENT16: return GL_UNSIGNED_SHORT;
      case GL_DEPTH_COMPONENT24: return GL_UNSIGNED_INT_24_8;
      case GL_DEPTH_COMPONENT32: return GL_UNSIGNED_INT;
      case GL_DEPTH_COMPONENT32F: return GL_FLOAT;
      case GL_DEPTH_COMPONENT32F_NV: return GL_FLOAT;
      case GL_STENCIL_INDEX1: return GL_UNSIGNED_BYTE;
      case GL_STENCIL_INDEX4: return GL_UNSIGNED_BYTE;
      case GL_STENCIL_INDEX8: return GL_UNSIGNED_BYTE;
      case GL_STENCIL_INDEX16: return GL_UNSIGNED_SHORT;
      case GL_DEPTH24_STENCIL8: return GL_UNSIGNED_INT_24_8;
      case GL_DEPTH32F_STENCIL8: return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
      case GL_DEPTH32F_STENCIL8_NV: return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;

      default: return GL_INVALID_VALUE;
    }
  }

  int glGetTypeSizeFromType( GLenum type )
  {
    switch ( type )
    {
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_UNSIGNED_BYTE_3_3_2:
      case GL_UNSIGNED_BYTE_2_3_3_REV:
        return 1;

      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_UNSIGNED_SHORT_5_6_5:
      case GL_UNSIGNED_SHORT_4_4_4_4:
      case GL_UNSIGNED_SHORT_5_5_5_1:
      case GL_UNSIGNED_SHORT_5_6_5_REV:
      case GL_UNSIGNED_SHORT_4_4_4_4_REV:
      case GL_UNSIGNED_SHORT_1_5_5_5_REV:
      case GL_HALF_FLOAT:
        return 2;

      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_UNSIGNED_INT_8_8_8_8:
      case GL_UNSIGNED_INT_8_8_8_8_REV:
      case GL_UNSIGNED_INT_10_10_10_2:
      case GL_UNSIGNED_INT_2_10_10_10_REV:
      case GL_UNSIGNED_INT_24_8:
      case GL_UNSIGNED_INT_10F_11F_11F_REV:
      case GL_UNSIGNED_INT_5_9_9_9_REV:
      case GL_FLOAT:
      case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
        return 4;

      default:
        return 0;
    }
  }

  GLFormatSize glGetFormatSize( const GLenum internalFormat )
  {
    GLFormatSize formatSize = { 0 };

    formatSize.minBlocksX = formatSize.minBlocksY = 1;
    switch ( internalFormat )
    {
      //
      // 8 bits per component
      //
      case GL_R8: // 1-component, 8-bit unsigned normalized
      case GL_R8_SNORM: // 1-component, 8-bit signed normalized
      case GL_R8UI: // 1-component, 8-bit unsigned integer
      case GL_R8I: // 1-component, 8-bit signed integer
      case GL_SR8: // 1-component, 8-bit sRGB
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 1 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RG8: // 2-component, 8-bit unsigned normalized
      case GL_RG8_SNORM: // 2-component, 8-bit signed normalized
      case GL_RG8UI: // 2-component, 8-bit unsigned integer
      case GL_RG8I: // 2-component, 8-bit signed integer
      case GL_SRG8: // 2-component, 8-bit sRGB
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 2 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGB8: // 3-component, 8-bit unsigned normalized
      case GL_RGB8_SNORM: // 3-component, 8-bit signed normalized
      case GL_RGB8UI: // 3-component, 8-bit unsigned integer
      case GL_RGB8I: // 3-component, 8-bit signed integer
      case GL_SRGB8: // 3-component, 8-bit sRGB
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 3 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGBA8: // 4-component, 8-bit unsigned normalized
      case GL_RGBA8_SNORM: // 4-component, 8-bit signed normalized
      case GL_RGBA8UI: // 4-component, 8-bit unsigned integer
      case GL_RGBA8I: // 4-component, 8-bit signed integer
      case GL_SRGB8_ALPHA8: // 4-component, 8-bit sRGB
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 4 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;

      //
      // 16 bits per component
      //
      case GL_R16: // 1-component, 16-bit unsigned normalized
      case GL_R16_SNORM: // 1-component, 16-bit signed normalized
      case GL_R16UI: // 1-component, 16-bit unsigned integer
      case GL_R16I: // 1-component, 16-bit signed integer
      case GL_R16F: // 1-component, 16-bit floating-point
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 2 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RG16: // 2-component, 16-bit unsigned normalized
      case GL_RG16_SNORM: // 2-component, 16-bit signed normalized
      case GL_RG16UI: // 2-component, 16-bit unsigned integer
      case GL_RG16I: // 2-component, 16-bit signed integer
      case GL_RG16F: // 2-component, 16-bit floating-point
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 4 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGB16: // 3-component, 16-bit unsigned normalized
      case GL_RGB16_SNORM: // 3-component, 16-bit signed normalized
      case GL_RGB16UI: // 3-component, 16-bit unsigned integer
      case GL_RGB16I: // 3-component, 16-bit signed integer
      case GL_RGB16F: // 3-component, 16-bit floating-point
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 6 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGBA16: // 4-component, 16-bit unsigned normalized
      case GL_RGBA16_SNORM: // 4-component, 16-bit signed normalized
      case GL_RGBA16UI: // 4-component, 16-bit unsigned integer
      case GL_RGBA16I: // 4-component, 16-bit signed integer
      case GL_RGBA16F: // 4-component, 16-bit floating-point
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 8 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;

      //
      // 32 bits per component
      //
      case GL_R32UI: // 1-component, 32-bit unsigned integer
      case GL_R32I: // 1-component, 32-bit signed integer
      case GL_R32F: // 1-component, 32-bit floating-point
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 4 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RG32UI: // 2-component, 32-bit unsigned integer
      case GL_RG32I: // 2-component, 32-bit signed integer
      case GL_RG32F: // 2-component, 32-bit floating-point
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 8 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGB32UI: // 3-component, 32-bit unsigned integer
      case GL_RGB32I: // 3-component, 32-bit signed integer
      case GL_RGB32F: // 3-component, 32-bit floating-point
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 12 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGBA32UI: // 4-component, 32-bit unsigned integer
      case GL_RGBA32I: // 4-component, 32-bit signed integer
      case GL_RGBA32F: // 4-component, 32-bit floating-point
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 16 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;

      //
      // Packed
      //
      case GL_R3_G3_B2: // 3-component 3:3:2, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGB4: // 3-component 4:4:4, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 12;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGB5: // 3-component 5:5:5, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 16;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGB565: // 3-component 5:6:5, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 16;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGB10: // 3-component 10:10:10, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 32;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGB12: // 3-component 12:12:12, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 36;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGBA2: // 4-component 2:2:2:2, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGBA4: // 4-component 4:4:4:4, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 16;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGBA12: // 4-component 12:12:12:12, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 48;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGB5_A1: // 4-component 5:5:5:1, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 32;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGB10_A2: // 4-component 10:10:10:2, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 32;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_RGB10_A2UI: // 4-component 10:10:10:2, unsigned integer
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 32;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_R11F_G11F_B10F: // 3-component 11:11:10, floating-point
      case GL_RGB9_E5: // 3-component/exp 9:9:9/5, floating-point
        formatSize.flags = GL_FORMAT_SIZE_PACKED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 32;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;

      //
      // S3TC/DXT/BC
      //
      case GL_COMPRESSED_RGB_S3TC_DXT1_EXT: // line through 3D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: // line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT: // line through 3D space, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: // line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 64;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: // line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: // line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: // line through 3D space plus line through 1D space, 4x4 blocks, sRGB
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: // line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;

      case GL_COMPRESSED_LUMINANCE_LATC1_EXT: // line through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT: // line through 1D space, 4x4 blocks, signed normalized
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 64;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT: // two lines through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT: // two lines through 1D space, 4x4 blocks, signed normalized
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;

      case GL_COMPRESSED_RED_RGTC1: // line through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SIGNED_RED_RGTC1: // line through 1D space, 4x4 blocks, signed normalized
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 64;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RG_RGTC2: // two lines through 1D space, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SIGNED_RG_RGTC2: // two lines through 1D space, 4x4 blocks, signed normalized
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;

      case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT: // 3-component, 4x4 blocks, unsigned floating-point
      case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT: // 3-component, 4x4 blocks, signed floating-point
      case GL_COMPRESSED_RGBA_BPTC_UNORM: // 4-component, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM: // 4-component, 4x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;

      //
      // ETC
      //
      case GL_ETC1_RGB8_OES: // 3-component ETC1, 4x4 blocks, unsigned normalized" ),
      case GL_COMPRESSED_RGB8_ETC2: // 3-component ETC2, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ETC2: // 3-component ETC2, 4x4 blocks, sRGB
      case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2: // 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: // 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 64;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA8_ETC2_EAC: // 4-component ETC2, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC: // 4-component ETC2, 4x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;

      case GL_COMPRESSED_R11_EAC: // 1-component ETC, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SIGNED_R11_EAC: // 1-component ETC, 4x4 blocks, signed normalized
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 64;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RG11_EAC: // 2-component ETC, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SIGNED_RG11_EAC: // 2-component ETC, 4x4 blocks, signed normalized
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;

      //
      // PVRTC
      //
      case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG: // 3-component PVRTC, 8x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT: // 3-component PVRTC, 8x4 blocks, sRGB
      case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG: // 4-component PVRTC, 8x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT: // 4-component PVRTC, 8x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 64;
        formatSize.blockWidth = 8;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        formatSize.minBlocksX = 2;
        formatSize.minBlocksY = 2;
        break;
      case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG: // 3-component PVRTC, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT: // 3-component PVRTC, 4x4 blocks, sRGB
      case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG: // 4-component PVRTC, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT: // 4-component PVRTC, 4x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 64;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        formatSize.minBlocksX = 2;
        formatSize.minBlocksY = 2;
        break;
      case GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG: // 4-component PVRTC, 8x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG: // 4-component PVRTC, 8x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 64;
        formatSize.blockWidth = 8;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG: // 4-component PVRTC, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG: // 4-component PVRTC, 4x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 64;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;

      //
      // ASTC
      //
      case GL_COMPRESSED_RGBA_ASTC_4x4_KHR: // 4-component ASTC, 4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR: // 4-component ASTC, 4x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_5x4_KHR: // 4-component ASTC, 5x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR: // 4-component ASTC, 5x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 5;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_5x5_KHR: // 4-component ASTC, 5x5 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR: // 4-component ASTC, 5x5 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 5;
        formatSize.blockHeight = 5;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_6x5_KHR: // 4-component ASTC, 6x5 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR: // 4-component ASTC, 6x5 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 6;
        formatSize.blockHeight = 5;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_6x6_KHR: // 4-component ASTC, 6x6 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR: // 4-component ASTC, 6x6 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 6;
        formatSize.blockHeight = 6;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_8x5_KHR: // 4-component ASTC, 8x5 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR: // 4-component ASTC, 8x5 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 8;
        formatSize.blockHeight = 5;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_8x6_KHR: // 4-component ASTC, 8x6 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR: // 4-component ASTC, 8x6 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 8;
        formatSize.blockHeight = 6;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_8x8_KHR: // 4-component ASTC, 8x8 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR: // 4-component ASTC, 8x8 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 8;
        formatSize.blockHeight = 8;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_10x5_KHR: // 4-component ASTC, 10x5 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR: // 4-component ASTC, 10x5 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 10;
        formatSize.blockHeight = 5;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_10x6_KHR: // 4-component ASTC, 10x6 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR: // 4-component ASTC, 10x6 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 10;
        formatSize.blockHeight = 6;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_10x8_KHR: // 4-component ASTC, 10x8 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR: // 4-component ASTC, 10x8 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 10;
        formatSize.blockHeight = 8;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_10x10_KHR: // 4-component ASTC, 10x10 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR: // 4-component ASTC, 10x10 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 10;
        formatSize.blockHeight = 10;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_12x10_KHR: // 4-component ASTC, 12x10 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR: // 4-component ASTC, 12x10 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 12;
        formatSize.blockHeight = 10;
        formatSize.blockDepth = 1;
        break;
      case GL_COMPRESSED_RGBA_ASTC_12x12_KHR: // 4-component ASTC, 12x12 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR: // 4-component ASTC, 12x12 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 12;
        formatSize.blockHeight = 12;
        formatSize.blockDepth = 1;
        break;

      case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES: // 4-component ASTC, 3x3x3 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES: // 4-component ASTC, 3x3x3 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 3;
        formatSize.blockHeight = 3;
        formatSize.blockDepth = 3;
        break;
      case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES: // 4-component ASTC, 4x3x3 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES: // 4-component ASTC, 4x3x3 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 3;
        formatSize.blockDepth = 3;
        break;
      case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES: // 4-component ASTC, 4x4x3 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES: // 4-component ASTC, 4x4x3 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 3;
        break;
      case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES: // 4-component ASTC, 4x4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES: // 4-component ASTC, 4x4x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 4;
        break;
      case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES: // 4-component ASTC, 5x4x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES: // 4-component ASTC, 5x4x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 5;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 4;
        break;
      case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES: // 4-component ASTC, 5x5x4 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES: // 4-component ASTC, 5x5x4 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 5;
        formatSize.blockHeight = 5;
        formatSize.blockDepth = 4;
        break;
      case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES: // 4-component ASTC, 5x5x5 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES: // 4-component ASTC, 5x5x5 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 5;
        formatSize.blockHeight = 5;
        formatSize.blockDepth = 5;
        break;
      case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES: // 4-component ASTC, 6x5x5 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES: // 4-component ASTC, 6x5x5 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 6;
        formatSize.blockHeight = 5;
        formatSize.blockDepth = 5;
        break;
      case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES: // 4-component ASTC, 6x6x5 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES: // 4-component ASTC, 6x6x5 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 6;
        formatSize.blockHeight = 6;
        formatSize.blockDepth = 5;
        break;
      case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES: // 4-component ASTC, 6x6x6 blocks, unsigned normalized
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES: // 4-component ASTC, 6x6x6 blocks, sRGB
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 6;
        formatSize.blockHeight = 6;
        formatSize.blockDepth = 6;
        break;

      //
      // ATC
      //
      case GL_ATC_RGB_AMD: // 3-component, 4x4 blocks, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 64;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;
      case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD: // 4-component, 4x4 blocks, unsigned normalized
      case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD: // 4-component, 4x4 blocks, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_COMPRESSED_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 128;
        formatSize.blockWidth = 4;
        formatSize.blockHeight = 4;
        formatSize.blockDepth = 1;
        break;

      //
      // Palletized
      //
      case GL_PALETTE4_RGB8_OES: // 3-component 8:8:8,   4-bit palette, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PALETTIZED_BIT;
        formatSize.paletteSizeInBits = 16 * 24;
        formatSize.blockSizeInBits = 4;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_PALETTE4_RGBA8_OES: // 4-component 8:8:8:8, 4-bit palette, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PALETTIZED_BIT;
        formatSize.paletteSizeInBits = 16 * 32;
        formatSize.blockSizeInBits = 4;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_PALETTE4_R5_G6_B5_OES: // 3-component 5:6:5,   4-bit palette, unsigned normalized
      case GL_PALETTE4_RGBA4_OES: // 4-component 4:4:4:4, 4-bit palette, unsigned normalized
      case GL_PALETTE4_RGB5_A1_OES: // 4-component 5:5:5:1, 4-bit palette, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PALETTIZED_BIT;
        formatSize.paletteSizeInBits = 16 * 16;
        formatSize.blockSizeInBits = 4;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_PALETTE8_RGB8_OES: // 3-component 8:8:8,   8-bit palette, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PALETTIZED_BIT;
        formatSize.paletteSizeInBits = 256 * 24;
        formatSize.blockSizeInBits = 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_PALETTE8_RGBA8_OES: // 4-component 8:8:8:8, 8-bit palette, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PALETTIZED_BIT;
        formatSize.paletteSizeInBits = 256 * 32;
        formatSize.blockSizeInBits = 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_PALETTE8_R5_G6_B5_OES: // 3-component 5:6:5,   8-bit palette, unsigned normalized
      case GL_PALETTE8_RGBA4_OES: // 4-component 4:4:4:4, 8-bit palette, unsigned normalized
      case GL_PALETTE8_RGB5_A1_OES: // 4-component 5:5:5:1, 8-bit palette, unsigned normalized
        formatSize.flags = GL_FORMAT_SIZE_PALETTIZED_BIT;
        formatSize.paletteSizeInBits = 256 * 16;
        formatSize.blockSizeInBits = 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;

      //
      // Depth/stencil
      //
      case GL_DEPTH_COMPONENT16:
        formatSize.flags = GL_FORMAT_SIZE_DEPTH_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 16;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_DEPTH_COMPONENT24:
      case GL_DEPTH_COMPONENT32:
      case GL_DEPTH_COMPONENT32F:
      case GL_DEPTH_COMPONENT32F_NV:
        formatSize.flags = GL_FORMAT_SIZE_DEPTH_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 32;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_STENCIL_INDEX1:
        formatSize.flags = GL_FORMAT_SIZE_STENCIL_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 1;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_STENCIL_INDEX4:
        formatSize.flags = GL_FORMAT_SIZE_STENCIL_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 4;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_STENCIL_INDEX8:
        formatSize.flags = GL_FORMAT_SIZE_STENCIL_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_STENCIL_INDEX16:
        formatSize.flags = GL_FORMAT_SIZE_STENCIL_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 16;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_DEPTH24_STENCIL8:
        formatSize.flags = GL_FORMAT_SIZE_DEPTH_BIT | GL_FORMAT_SIZE_STENCIL_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 32;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
      case GL_DEPTH32F_STENCIL8:
      case GL_DEPTH32F_STENCIL8_NV:
        formatSize.flags = GL_FORMAT_SIZE_DEPTH_BIT | GL_FORMAT_SIZE_STENCIL_BIT;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 64;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;

      default:
        formatSize.flags = 0;
        formatSize.paletteSizeInBits = 0;
        formatSize.blockSizeInBits = 0 * 8;
        formatSize.blockWidth = 1;
        formatSize.blockHeight = 1;
        formatSize.blockDepth = 1;
        break;
    }

    return formatSize;
  }

}