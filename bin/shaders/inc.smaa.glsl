/**
 * Copyright (C) 2011 Jorge Jimenez (jorge@iryoku.com)
 * Copyright (C) 2011 Belen Masia (bmasia@unizar.es)
 * Copyright (C) 2011 Jose I. Echevarria (joseignacioechevarria@gmail.com)
 * Copyright (C) 2011 Fernando Navarro (fernandn@microsoft.com)
 * Copyright (C) 2011 Diego Gutierrez (diegog@unizar.es)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution:
 *
 *      "Uses SMAA. Copyright (C) 2011 by Jorge Jimenez, Jose I. Echevarria,
 *       Belen Masia, Fernando Navarro and Diego Gutierrez."
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the copyright holders.
 */

/**
 *                  _______  ___  ___       ___           ___
 *                 /       ||   \/   |     /   \         /   \
 *                |   (---- |  \  /  |    /  ^  \       /  ^  \
 *                 \   \    |  |\/|  |   /  /_\  \     /  /_\  \
 *              ----)   |   |  |  |  |  /  _____  \   /  _____  \
 *             |_______/    |__|  |__| /__/     \__\ /__/     \__\
 *
 *                               E N H A N C E D
 *       S U B P I X E L   M O R P H O L O G I C A L   A N T I A L I A S I N G
 *
 *                         http://www.iryoku.com/smaa/
 *
 * Hi, welcome aboard!
 *
 * Here you'll find instructions to get the shader up and running as fast as
 * possible.
 *
 * IMPORTANTE NOTICE: when updating, remember to update both this file and the
 * precomputed textures! They may change from version to version.
 *
 * The shader has three passes, chained together as follows:
 *
 *                           |input|------------------·
 *                              v                     |
 *                    [ SMAA*EdgeDetection ]          |
 *                              v                     |
 *                          |edgesTex|                |
 *                              v                     |
 *              [ SMAABlendingWeightCalculation ]     |
 *                              v                     |
 *                          |blendTex|                |
 *                              v                     |
 *                [ SMAANeighborhoodBlending ] <------·
 *                              v
 *                           |output|
 *
 * Note that each [pass] has its own vertex and pixel shader.
 *
 * You've three edge detection methods to choose from: luma, color or depth.
 * They represent different quality/performance and anti-aliasing/sharpness
 * tradeoffs, so our recommendation is for you to choose the one that best
 * suits your particular scenario:
 *
 * - Depth edge detection is usually the fastest but it may miss some edges.
 *
 * - Luma edge detection is usually more expensive than depth edge detection,
 *   but catches visible edges that depth edge detection can miss.
 *
 * - Color edge detection is usually the most expensive one but catches
 *   chroma-only edges.
 *
 * For quickstarters: just use luma edge detection.
 *
 * The general advice is to not rush the integration process and ensure each
 * step is done correctly (don't try to integrate SMAA T2x with predicated edge
 * detection from the start!). Ok then, let's go!
 *
 *  1. The first step is to create two RGBA temporal framebuffers for holding
 *     |edgesTex| and |blendTex|.
 *
 *     In DX10, you can use a RG framebuffer for the edges texture, but in our
 *     experience it yields worse performance.
 *
 *     On the Xbox 360, you can use the same framebuffer for resolving both
 *     |edgesTex| and |blendTex|, as they aren't needed simultaneously.
 *
 *  2. Both temporal framebuffers |edgesTex| and |blendTex| must be cleared
 *     each frame. Do not forget to clear the alpha channel!
 *
 *  3. The next step is loading the two supporting precalculated textures,
 *     'areaTex' and 'searchTex'. You'll find them in the 'Textures' folder as
 *     C++ headers, and also as regular DDS files. They'll be needed for the
 *     'SMAABlendingWeightCalculation' pass.
 *
 *     If you use the C++ headers, be sure to load them in the format specified
 *     inside of them.
 *
 *  4. In DX9, all samplers must be set to linear filtering and clamp, with the
 *     exception of 'searchTex', which must be set to point filtering.
 *
 *  5. All texture reads and buffer writes must be non-sRGB, with the exception
 *     of the input read and the output write of input in
 *     'SMAANeighborhoodBlending' (and only in this pass!). If sRGB reads in
 *     this last pass are not possible, the technique will work anyway, but
 *     will perform antialiasing in gamma space.
 *
 *     IMPORTANT: for best results the input read for the color/luma edge
 *     detection should *NOT* be sRGB.
 *
 *  6. Before including SMAA.h you'll have to setup the framebuffer pixel size,
 *     the target and any optional configuration defines. Optionally you can
 *     use a preset.
 *
 *     You have three targets available:
 *         SMAA_HLSL_3
 *         SMAA_HLSL_4
 *         SMAA_HLSL_4_1
 *         SMAA_GLSL_3 *
 *         SMAA_GLSL_4 *
 *
 *         * (See SMAA_ONLY_COMPILE_VS below).
 *
 *     And four presets:
 *         SMAA_PRESET_LOW          (%60 of the quality)
 *         SMAA_PRESET_MEDIUM       (%80 of the quality)
 *         SMAA_PRESET_HIGH         (%95 of the quality)
 *         SMAA_PRESET_ULTRA        (%99 of the quality)
 *
 *     For example:
 *         #define SMAA_PIXEL_SIZE float2(1.0 / 1280.0, 1.0 / 720.0)
 *         #define SMAA_HLSL_4 1
 *         #define SMAA_PRESET_HIGH 1
 *         #include "SMAA.h"
 *
 *  7. Then, you'll have to setup the passes as indicated in the scheme above.
 *     You can take a look into SMAA.fx, to see how we did it for our demo.
 *     Checkout the function wrappers, you may want to copy-paste them!
 *
 *  8. It's recommended to validate the produced |edgesTex| and |blendTex|.
 *     It's advised to not continue with the implementation until both buffers
 *     are verified to produce identical results to our reference demo.
 *
 *  9. After you get the last pass to work, it's time to optimize. You'll have
 *     to initialize a stencil buffer in the first pass (discard is already in
 *     the code), then mask execution by using it the second pass. The last
 *     pass should be executed in all pixels.
 *
 *
 * After this point you can choose to enable predicated thresholding,
 * temporal supersampling and motion blur integration:
 *
 * a) If you want to use predicated thresholding, take a look into
 *    SMAA_PREDICATION; you'll need to pass an extra texture in the edge
 *    detection pass.
 *
 * b) If you want to enable temporal supersampling (SMAA T2x):
 *
 * 1. The first step is to render using subpixel jitters. I won't go into
 *    detail, but it's as simple as moving each vertex position in the
 *    vertex shader, you can check how we do it in our DX10 demo.
 *
 * 2. Then, you must setup the temporal resolve. You may want to take a look
 *    into SMAAResolve for resolving 2x modes. After you get it working, you'll
 *    probably see ghosting everywhere. But fear not, you can enable the
 *    CryENGINE temporal reprojection by setting the SMAA_REPROJECTION macro.
 *
 * 3. The next step is to apply SMAA to each subpixel jittered frame, just as
 *    done for 1x.
 *
 * 4. At this point you should already have something usable, but for best
 *    results the proper area textures must be set depending on current jitter.
 *    For this, the parameter 'subsampleIndices' of
 *    'SMAABlendingWeightCalculationPS' must be set as follows, for our T2x
 *    mode:
 *
 *    @SUBSAMPLE_INDICES
 *
 *    | S# |  Camera Jitter   |  subsampleIndices  |
 *    +----+------------------+--------------------+
 *    |  0 |  ( 0.25, -0.25)  |  int4(1, 1, 1, 0)  |
 *    |  1 |  (-0.25,  0.25)  |  int4(2, 2, 2, 0)  |
 *
 *    These jitter positions assume a bottom-to-top y axis. S# stands for the
 *    sample number.
 *
 * More information about temporal supersampling here:
 *    http://iryoku.com/aacourse/downloads/13-Anti-Aliasing-Methods-in-CryENGINE-3.pdf
 *
 * c) If you want to enable spatial multisampling (SMAA S2x):
 *
 * 1. The scene must be rendered using MSAA 2x. The MSAA 2x buffer must be
 *    created with:
 *      - DX10:     see below (*)
 *      - DX10.1:   D3D10_STANDARD_MULTISAMPLE_PATTERN or
 *      - DX11:     D3D11_STANDARD_MULTISAMPLE_PATTERN
 *
 *    This allows to ensure that the subsample order matches the table in
 *    @SUBSAMPLE_INDICES.
 *
 *    (*) In the case of DX10, we refer the reader to:
 *      - SMAA::detectMSAAOrder and
 *      - SMAA::msaaReorder
 *
 *    These functions allow to match the standard multisample patterns by
 *    detecting the subsample order for a specific GPU, and reordering
 *    them appropriately.
 *
 * 2. A shader must be run to output each subsample into a separate buffer
 *    (DX10 is required). You can use SMAASeparate for this purpose, or just do
 *    it in an existing pass (for example, in the tone mapping pass).
 *
 * 3. The full SMAA 1x pipeline must be run for each separated buffer, storing
 *    the results in the final buffer. The second run should alpha blend with
 *    the existing final buffer using a blending factor of 0.5.
 *    'subsampleIndices' must be adjusted as in the SMAA T2x case (see point
 *    b).
 *
 * d) If you want to enable temporal supersampling on top of SMAA S2x
 *    (which actually is SMAA 4x):
 *
 * 1. SMAA 4x consists on temporally jittering SMAA S2x, so the first step is
 *    to calculate SMAA S2x for current frame. In this case, 'subsampleIndices'
 *    must be set as follows:
 *
 *    | F# | S# |   Camera Jitter    |    Net Jitter     |  subsampleIndices  |
 *    +----+----+--------------------+-------------------+--------------------+
 *    |  0 |  0 |  ( 0.125,  0.125)  |  ( 0.375, -0.125) |  int4(5, 3, 1, 3)  |
 *    |  0 |  1 |  ( 0.125,  0.125)  |  (-0.125,  0.375) |  int4(4, 6, 2, 3)  |
 *    +----+----+--------------------+-------------------+--------------------+
 *    |  1 |  2 |  (-0.125, -0.125)  |  ( 0.125, -0.375) |  int4(3, 5, 1, 4)  |
 *    |  1 |  3 |  (-0.125, -0.125)  |  (-0.375,  0.125) |  int4(6, 4, 2, 4)  |
 *
 *    These jitter positions assume a bottom-to-top y axis. F# stands for the
 *    frame number. S# stands for the sample number.
 *
 * 2. After calculating SMAA S2x for current frame (with the new subsample
 *    indices), previous frame must be reprojected as in SMAA T2x mode (see
 *    point b).
 *
 * e) If motion blur is used, you may want to do the edge detection pass
 *    together with motion blur. This has two advantages:
 *
 * 1. Pixels under heavy motion can be omitted from the edge detection process.
 *    For these pixels we can just store "no edge", as motion blur will take
 *    care of them.
 * 2. The center pixel tap is reused.
 *
 * Note that in this case depth testing should be used instead of stenciling,
 * as we have to write all the pixels in the motion blur pass.
 *
 * That's it!
 */

//-----------------------------------------------------------------------------
// SMAA Presets

/**
 * Note that if you use one of these presets, the corresponding macros below
 * won't be used.
 */

#ifndef SMAA_INCLUDE_GLSL
#define SMAA_INCLUDE_GLSL

#define SMAA_GLSL_4

#define mad( a, b, c ) ( a * b + c )
#define saturate( a ) clamp( a, 0.0, 1.0 )
#define round( v ) floor( v + 0.5 )
#define SMAASampleLevelZeroOffset( tex, coord, offset ) texture2D( tex, coord + offset * SMAA_RT_METRICS.xy )

#define SMAA_PRESET_ULTRA 1

// Presets
#if defined( SMAA_PRESET_LOW )
#define SMAA_THRESHOLD 0.15
#define SMAA_MAX_SEARCH_STEPS 4
#define SMAA_DISABLE_DIAG_DETECTION
#define SMAA_DISABLE_CORNER_DETECTION
#elif defined( SMAA_PRESET_MEDIUM )
#define SMAA_THRESHOLD 0.1
#define SMAA_MAX_SEARCH_STEPS 8
#define SMAA_DISABLE_DIAG_DETECTION
#define SMAA_DISABLE_CORNER_DETECTION
#elif defined( SMAA_PRESET_HIGH )
#define SMAA_THRESHOLD 0.1
#define SMAA_MAX_SEARCH_STEPS 16
#define SMAA_MAX_SEARCH_STEPS_DIAG 8
#define SMAA_CORNER_ROUNDING 25
#elif defined( SMAA_PRESET_ULTRA )
#define SMAA_THRESHOLD 0.05
#define SMAA_MAX_SEARCH_STEPS 32
#define SMAA_MAX_SEARCH_STEPS_DIAG 16
#define SMAA_CORNER_ROUNDING 25
#endif

#ifndef SMAA_THRESHOLD
#define SMAA_THRESHOLD 0.1
#endif

#ifndef SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR
#define SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR 2.0
#endif

// Non-Configurable Defines
#define SMAA_AREATEX_MAX_DISTANCE 16
#define SMAA_AREATEX_MAX_DISTANCE_DIAG 20
#define SMAA_AREATEX_PIXEL_SIZE ( 1.0 / vec2( 160.0, 560.0 ) )
#define SMAA_AREATEX_SUBTEX_SIZE ( 1.0 / 7.0 )
#define SMAA_SEARCHTEX_SIZE vec2( 66.0, 33.0 )
#define SMAA_SEARCHTEX_PACKED_SIZE vec2( 64.0, 16.0 )
#define SMAA_CORNER_ROUNDING_NORM ( float( SMAA_CORNER_ROUNDING ) / 100.0 )

// Texture Access Defines
#ifndef SMAA_AREATEX_SELECT
#define SMAA_AREATEX_SELECT( sample ) sample.rg
#endif

#ifndef SMAA_SEARCHTEX_SELECT
#define SMAA_SEARCHTEX_SELECT( sample ) sample.r
#endif

void SMAAMovc( bvec2 cond, inout vec2 variable, vec2 value )
{
  if ( cond.x )
    variable.x = value.x;
  if ( cond.y )
    variable.y = value.y;
}

void SMAAMovc( bvec4 cond, inout vec4 variable, vec4 value )
{
  SMAAMovc( cond.xy, variable.xy, value.xy );
  SMAAMovc( cond.zw, variable.zw, value.zw );
}

/**
 * Allows to decode two binary values from a bilinear-filtered access.
 */
vec2 SMAADecodeDiagBilinearAccess( vec2 e )
{
  // Bilinear access for fetching 'e' have a 0.25 offset, and we are
  // interested in the R and G edges:
  //
  // +---G---+-------+
  // |   x o R   x   |
  // +-------+-------+
  //
  // Then, if one of these edge is enabled:
  //   Red:   (0.75 * X + 0.25 * 1) => 0.25 or 1.0
  //   Green: (0.75 * 1 + 0.25 * X) => 0.75 or 1.0
  //
  // This function will unpack the values (mad + mul + round):
  // wolframalpha.com: round(x * abs(5 * x - 5 * 0.75)) plot 0 to 1
  e.r = e.r * abs( 5.0 * e.r - 5.0 * 0.75 );
  return round( e );
}

vec4 SMAADecodeDiagBilinearAccess( vec4 e )
{
  e.rb = e.rb * abs( 5.0 * e.rb - 5.0 * 0.75 );
  return round( e );
}

#endif // SMAA_INCLUDE_GLSL
