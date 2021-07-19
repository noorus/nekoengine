#version 450 core

uniform sampler2D screenTexture;

//uniform vec3 palette[8];
//uniform int paletteSize;

in vec2 texcoord;

out vec4 out_color;

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

void main()
{
  //vec4 clr = texture( screenTexture, texcoord );
  //frag_colour = vec4( dither( clr.rgb ), clr.a );
  out_color = texture( screenTexture, texcoord );
}
