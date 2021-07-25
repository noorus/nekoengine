#ifndef COLORUTILS_INCLUDE_GLSL
#define COLORUTILS_INCLUDE_GLSL

float bt709LumaExtract( vec3 linearColor )
{
  return dot( linearColor, vec3( 0.2126, 0.7152, 0.0722 ) );
}

vec3 gammaCorrect( vec3 color )
{
  return pow( color, vec3( 1.0 / gamma ) );
}

vec3 tonemap_linear( vec3 color )
{
  color = clamp( exposure * color, 0.0, 1.0 );
  color = gammaCorrect( color );
  return color;
}

vec3 tonemap_simpleReinhard( vec3 color )
{
  color *= exposure / ( 1.0 + color / exposure );
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
  color *= exposure;
  color = ( ( color * ( A * color + C * B ) + D * E ) / ( color * ( A * color + B ) + D * F ) ) - E / F;
  float white = ( ( W * ( A * W + C * B ) + D * E ) / ( W * ( A * W + B ) + D * F ) ) - E / F;
  color /= white;
  color = gammaCorrect( color );
  return color;
}

#endif