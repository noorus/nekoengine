#ifndef COLORUTILS_INCLUDE_GLSL
#define COLORUTILS_INCLUDE_GLSL

const float c_PI = 3.14159265359;

float bt709LumaExtract( vec3 linearColor )
{
  return dot( linearColor, vec3( 0.2126, 0.7152, 0.0722 ) );
}

vec3 gammaCorrect( vec3 color )
{
  return pow( color, vec3( 1.0 / processing.gamma ) );
}

float saturate( float v )
{
  return clamp( v, 0.0, 1.0 );
}

vec3 util_unpackNormalDXT5nm( vec4 packednormal )
{
  vec3 normal;
  normal.xy = packednormal.wy * 2.0 - 1.0;
  normal.z = sqrt( 1.0 - saturate( dot( normal.xy, normal.xy ) ) );
  return normal;
}

vec3 util_unpackNormalRGorAG( vec4 packednormal )
{
  packednormal.x *= packednormal.w;
  vec3 normal;
  normal.xy = packednormal.xy * 2.0 - 1.0;
  normal.z = sqrt( 1.0 - saturate( dot( normal.xy, normal.xy ) ) );
  return normal;
}

vec3 util_unpackUnityNormal( vec4 packednormal )
{
  vec3 unpacked = util_unpackNormalRGorAG( packednormal );
  return vec3( unpacked.x, -unpacked.y, unpacked.z );
}

// this function only compiles in a fragment shader
vec3 util_normalToWorld( vec3 tangentNormal, vec2 texcoord, vec3 worldpos, vec3 vertNormal )
{
  vec3 Q1 = dFdx( worldpos );
  vec3 Q2 = dFdy( worldpos );
  vec2 st1 = dFdx( texcoord );
  vec2 st2 = dFdy( texcoord );

  vec3 N = normalize( vertNormal );
  vec3 T = normalize( Q1 * st2.t - Q2 * st1.t );
  vec3 B = -normalize( cross( N, T ) );
  mat3 TBN = mat3( T, B, N );

  return normalize( TBN * tangentNormal );
}

#ifdef COLORUTILS_TONEMAPPING

vec3 tonemap_linear( vec3 color )
{
  color = clamp( world.camera.exposure * color, 0.0, 1.0 );
  color = gammaCorrect( color );
  return color;
}

vec3 tonemap_simpleReinhard( vec3 color )
{
  color *= world.camera.exposure / ( 1.0 + color / world.camera.exposure );
  color = gammaCorrect( color );
  return color;
}

vec3 tonemap_lumaBasedReinhard( vec3 color )
{
  float luma = bt709LumaExtract( color );
  float toneMappedLuma = luma / ( 1.0 + luma );
  color *= toneMappedLuma / luma;
  color = gammaCorrect( color );
  return color;
}

vec3 tonemap_whitePreserveLumaReinhard( vec3 color )
{
  float white = 2.0;
  float luma = bt709LumaExtract( color );
  float toneMappedLuma = luma * ( 1.0 + luma / ( white * white ) ) / ( 1.0 + luma );
  color *= toneMappedLuma / luma;
  color = gammaCorrect( color );
  return color;
}

vec3 tonemap_ronBinDaHouse( vec3 color )
{
  color = exp( -1.0 / ( 2.72 * color + 0.15 ) );
  color = gammaCorrect( color );
  return color;
}

vec3 tonemap_filmic( vec3 color )
{
  color = max( vec3( 0.0 ), color - vec3( 0.004 ) );
  color = ( color * ( 6.2 * color + 0.5 ) ) / ( color * ( 6.2 * color + 1.7 ) + 0.06 );
  return color;
}

vec3 tonemap_uncharted2( vec3 color )
{
  float A = 0.15;
  float B = 0.50;
  float C = 0.10;
  float D = 0.20;
  float E = 0.02;
  float F = 0.30;
  float W = 11.2;
  color *= world.camera.exposure;
  color = ( ( color * ( A * color + C * B ) + D * E ) / ( color * ( A * color + B ) + D * F ) ) - E / F;
  float white = ( ( W * ( A * W + C * B ) + D * E ) / ( W * ( A * W + B ) + D * F ) ) - E / F;
  color /= white;
  color = gammaCorrect( color );
  return color;
}

#endif

//uniform vec3 palette[8];
//uniform int paletteSize;

/*const float lightnessSteps = 4.0;

float lightnessStep( float l )
{
  return floor( ( 0.5 + l * lightnessSteps ) ) / lightnessSteps;
}

vec3 hsl2rgb( in vec3 color )
{
  vec3 rgb = clamp( abs( mod( color.r * 6.0 + vec3( 0.0, 4.0, 2.0 ), 6.0 ) - 3.0 ) - 1.0, 0.0, 1.0 );
  return color.b + color.g * ( rgb - 0.5 ) * ( 1.0 - abs( 2.0 * color.b - 1.0 ) );
}

vec3 hueShift( in vec3 color, in float shift )
{
  vec3 p = vec3( 0.55735 ) * dot( vec3( 0.55735 ), color );
  vec3 u = color - p;
  vec3 v = cross( vec3( 0.55735 ), u );
  color = u * cos( shift * 6.2832 ) + v * sin( shift * 6.2832 ) + p;
  return vec3( color );
}

float hueDistance( float hue1, float hue2 )
{
  float diff = abs( ( hue1 - hue2 ) );
  return min( abs( ( 1.0 - diff ) ), diff );
}

vec3 rgb2hsl( in vec3 color )
{
  float h = 0.0;
  float s = 0.0;
  float l = 0.0;
  float r = color.r;
  float g = color.g;
  float b = color.b;
  float cMin = min( r, min( g, b ) );
  float cMax = max( r, max( g, b ) );
  l = ( cMax + cMin ) / 2.0;
  if ( cMax > cMin )
  {
    float cDelta = cMax - cMin;
    //s = l < .05 ? cDelta / ( cMax + cMin ) : cDelta / ( 2.0 - ( cMax + cMin ) ); Original
    s = l < .0 ? cDelta / ( cMax + cMin ) : cDelta / ( 2.0 - ( cMax + cMin ) );
    if ( r == cMax )
      h = ( g - b ) / cDelta;
    else if ( g == cMax )
      h = 2.0 + ( b - r ) / cDelta;
    else
      h = 4.0 + ( r - g ) / cDelta;

    if ( h < 0.0 )
      h += 6.0;
    h = h / 6.0;
  }
  return vec3( h, s, l );
}

const int indexMatrix8x8[64] = int[]
(
  0,  32, 8,  40, 2,  34, 10, 42,
  48, 16, 56, 24, 50, 18, 58, 26,
  12, 44, 4,  36, 14, 46, 6,  38,
  60, 28, 52, 20, 62, 30, 54, 22,
  3,  35, 11, 43, 1,  33, 9,  41,
  51, 19, 59, 27, 49, 17, 57, 25,
  15, 47, 7,  39, 13, 45, 5,  37,
  63, 31, 55, 23, 61, 29, 53, 21
);

float indexValue()
{
  int x = int( mod( gl_FragCoord.x, 8 ) );
  int y = int( mod( gl_FragCoord.y, 8 ) );
  return indexMatrix8x8[( x + y * 8 )] / 64.0;
}

vec3[2] closestColors( float hue )
{
  vec3 ret[2];
  vec3 closest = vec3( -2, 0, 0 );
  vec3 secondClosest = vec3( -2, 0, 0 );
  vec3 temp;
  for ( int i = 0; i < paletteSize; ++i )
  {
    temp = palette[i];
    float tempDistance = hueDistance( temp.x, hue );
    if ( tempDistance < hueDistance( closest.x, hue ) )
    {
      secondClosest = closest;
      closest = temp;
    }
    else
    {
      if ( tempDistance < hueDistance( secondClosest.x, hue ) )
        secondClosest = temp;
    }
  }
  ret[0] = closest;
  ret[1] = secondClosest;
  return ret;
}

vec3 dither( vec3 color )
{
  vec3 hsl = rgb2hsl( color );

  vec3 cs[2] = closestColors( hsl.x );
  vec3 c1 = cs[0];
  vec3 c2 = cs[1];
  float d = indexValue();
  float hueDiff = hueDistance( hsl.x, c1.x ) / hueDistance( c2.x, c1.x );

  float l1 = lightnessStep( max( ( hsl.z - 0.125 ), 0.0 ) );
  float l2 = lightnessStep( min( ( hsl.z + 0.124 ), 1.0 ) );
  float lightnessDiff = ( hsl.z - l1 ) / ( l2 - l1 );

  vec3 resultColor = ( hueDiff < d ) ? c1 : c2;
  resultColor.z = ( lightnessDiff < d ) ? l1 : l2;
  return hsl2rgb( resultColor );
}*/

#endif