#pragma once
#include "neko_types.h"
#include <algorithm>
#include "nekosimd.h"

#ifdef NEKO_MATH_DOUBLE
# define NEKO_MATH_FUNC_ROUND ::round
# define NEKO_MATH_FUNC_SQRT ::sqrt
# define NEKO_MATH_FUNC_FLOOR ::floor
# define NEKO_MATH_FUNC_CEIL ::ceil
# define NEKO_MATH_FUNC_SIN ::sin
# define NEKO_MATH_FUNC_COS ::cos
# define NEKO_MATH_FUNC_TAN ::tan
# define NEKO_MATH_FUNC_ASIN ::asin
# define NEKO_MATH_FUNC_ACOS ::acos
# define NEKO_MATH_FUNC_ATAN ::atan
# define NEKO_MATH_FUNC_ABS ::abs
#else
# define NEKO_MATH_FUNC_ROUND ::roundf
# define NEKO_MATH_FUNC_SQRT ::sqrtf
# define NEKO_MATH_FUNC_FLOOR ::floorf
# define NEKO_MATH_FUNC_CEIL ::ceilf
# define NEKO_MATH_FUNC_SIN ::sinf
# define NEKO_MATH_FUNC_COS ::cosf
# define NEKO_MATH_FUNC_TAN ::tanf
# define NEKO_MATH_FUNC_ASIN ::asinf
# define NEKO_MATH_FUNC_ACOS ::acosf
# define NEKO_MATH_FUNC_ATAN ::atanf
# define NEKO_MATH_FUNC_ABS ::abs
#endif

// Unfortunately the names "near" and "far" cannot be used within
// functions in this header because it interferes with some Windows internals
// (even though we undo the damage in most other parts of the engine)

namespace neko {

  constexpr Radians radians( Degrees r )
  {
    return glm::radians( r );
  }

  constexpr Degrees degrees( Radians r )
  {
    return glm::degrees( r );
  }

  namespace math {

    using glm::angleAxis;
    using glm::cross;
    using glm::degrees;
    using glm::dot;
    using glm::inverse;
    using glm::length;
    using glm::length2;
    using glm::normalize;
    using glm::radians;
    using glm::rotate;
    using glm::toMat3;
    using glm::toMat4;
    using glm::translate;
    using glm::scale;
    using glm::transpose;

    using std::atan2;

    using std::max;
    using std::min;

    static std::random_device rd;
    static std::mt19937_64 reng( rd() );
    static std::uniform_real_distribution<Real> rdist( 0.0f, 1.0f );

    inline Real rand()
    {
      return rdist( reng );
    }

    inline bool epsilonCompare( const vec3& a, const vec3& b )
    {
      return glm::all( glm::epsilonEqual( a, b, vec3( 0.01f ) ) );
    }

    //! \fn inline Real round( Real value )
    //! \brief Rounds to the nearest non-decimal value.
    //! \param value The value.
    //! \returns Given value rounded to nearest non-decimal.
    inline Real round( Real value )
    {
      return NEKO_MATH_FUNC_ROUND( value );
    }

    inline vec2 round( const vec2& v )
    {
      return { round( v.x ), round( v.y ) };
    }

    //! \fn inline int iround( Real value )
    //! \brief Rounds to the nearest non-decimal value, returning an integer.
    //! \param value The value.
    //! \returns Given value rounded to nearest integer.
    inline int iround( Real value )
    {
      return static_cast<int>( NEKO_MATH_FUNC_ROUND( value ) );
    }

    //! \fn inline Real sqrt( Real value )
    //! \brief Square root of given floating point value.
    //! \param value A value.
    //! \return The square root of value.
    inline Real sqrt( Real value )
    {
      return NEKO_MATH_FUNC_SQRT( value );
    }

    //! \fn inline int ifloor( Real value )
    //! \brief Floors the given floating point value and returns an integer,
    //!   due to this being the most common use case.
    //! \param value The value.
    //! \return Given value as floored integer.
    inline int ifloor( Real value )
    {
      return (int)NEKO_MATH_FUNC_FLOOR( value );
    }

#ifndef NEKO_MATH_DOUBLE
    inline int ifloor( double value )
    {
      return static_cast<int>( ::floor( value ) );
    }
#endif

    //! \fn inline Real floor( Real value )
    //! \brief Floors the given value.
    //! \param value The value.
    //! \returns Given value rounded down.
    inline Real floor( Real value )
    {
      return NEKO_MATH_FUNC_FLOOR( value );
    }

    inline vec2 floor( const vec2& v )
    {
      return { floor( v.x ), floor( v.y ) };
    }

    //! \fn inline Real ceil( Real value )
    //! \brief Ceilings the given value.
    //! \param value The value.
    //! \returns Given value rounded up.
    inline Real ceil( Real value )
    {
      return NEKO_MATH_FUNC_CEIL( value );
    }

    inline vec2 ceil( const vec2& v )
    {
      return { ceil( v.x ), ceil( v.y ) };
    }

    //! \fn inline int iceil( Real value )
    //! \brief Ceilings the given value.
    //! \param value The value.
    //! \returns Given value rounded up.
    inline int iceil( Real value )
    {
      return static_cast<int>( NEKO_MATH_FUNC_CEIL( value ) );
    }

    //! \fn inline Real sin( Real value )
    //! \brief Sine.
    //! \param value Value.
    //! \return Sine of value.
    inline Real sin( Real value )
    {
      return NEKO_MATH_FUNC_SIN( value );
    }

    //! \fn inline Real cos( Real value )
    //! \brief Cosine.
    //! \param value Value.
    //! \return Cosine of value.
    inline Real cos( Real value )
    {
      return NEKO_MATH_FUNC_COS( value );
    }

    //! \fn inline Real tan( Real value )
    //! \brief Tangent.
    //! \param value Value.
    //! \return Tangent of value.
    inline Real tan( Real value )
    {
      return NEKO_MATH_FUNC_TAN( value );
    }

    //! \fn inline Real asin( Real value )
    //! \brief Arcsine.
    //! \param value Value.
    //! \return Arcsine of value.
    inline Real asin( Real value )
    {
      return NEKO_MATH_FUNC_ASIN( value );
    }

    //! \fn inline Real cos( Real value )
    //! \brief Arccosine.
    //! \param value Value.
    //! \return Arccosine of value.
    inline Real acos( Real value )
    {
      return NEKO_MATH_FUNC_ACOS( value );
    }

    //! \fn inline Real tan( Real value )
    //! \brief Arctangent.
    //! \param value Value.
    //! \return Arctangent of value.
    inline Real atan( Real value )
    {
      return NEKO_MATH_FUNC_ATAN( value );
    }

    //! \fn abs
    //! \brief Absolute value of float, double, long etc.
    //! \param value A value.
    //! \return Absolute positive of the value.
    template <typename T>
    inline T abs( T value )
    {
      return NEKO_MATH_FUNC_ABS( value );
    }

    //! \fn inline Real clamp( Real value, Real low, Real high )
    //! \brief Clamps the given value between a high bound and a low bound.
    //! \param value The value.
    //! \param low   The low.
    //! \param high  The high.
    //! \return Clamped value.
    constexpr Real clamp( Real value, Real low, Real high )
    {
      return ( value < low ? low : ( value > high ? high : value ) );
    }

    //! \fn inline int clamp( int value, int low, int high )
    //! \brief Clamps the given value between a high bound and a low bound.
    //! \param value The value.
    //! \param low   The low.
    //! \param high  The high.
    //! \return Clamped value.
    constexpr int clamp( int value, int low, int high )
    {
      return ( value < low ? low : ( value > high ? high : value ) );
    }

    //! \fn inline T manhattan( T x1, T y1, T x2, T y2 )
    //! \brief 2D Manhattan distance with coordinates of any type supported by abs().
    //! \param x1 The first x value.
    //! \param y1 The first y value.
    //! \param x2 The second x value.
    //! \param y2 The second y value.
    //! \return Manhattan distance between x1,y1 and x2,y2.
    template <typename T>
    constexpr T manhattan( T x1, T y1, T x2, T y2 )
    {
      return ( abs( x2 - x1 ) + abs( y2 - y1 ) );
    }

    inline vec3 perpendicular( const vec3 v )
    {
      const auto sqz = static_cast<Real>( 1e-06 * 1e-06 );
      auto perp = cross( v, vec3( 1.0f, 0.0f, 0.0f ) );
      if ( length2( perp ) < sqz )
        perp = cross( v, vec3( 0.0f, 1.0f, 0.0f ) );
      return normalize( perp );
    }

    /* this is broken, not sure how
    * but currently we don't need anything that glm::perspective wouldn't provide
    inline mat4 buildPerspective( Real left, Real right, Real bottom, Real top, Real znear, Real zfar )
    {
      // assert( znear > 0.0f && zfar > 0.0f );
      auto invw = ( numbers::one / ( right - left ) );
      auto invh = ( numbers::one / ( top - bottom ) );
      auto invd = ( numbers::one / ( zfar - znear ) );
      auto a = ( numbers::two * znear * invw );
      auto b = ( numbers::two * znear * invh );
      auto c = ( ( right + left ) * invw );
      auto d = ( ( top + bottom ) * invh );
      auto q = -( zfar + znear ) * invd;
      auto qn = -numbers::two * ( zfar * znear ) * invd;
      auto ret = mat4::zero();
      ret[0][0] = a;
      ret[0][2] = c;
      ret[1][1] = b;
      ret[1][2] = d;
      ret[2][2] = q;
      ret[2][3] = qn;
      ret[3][2] = static_cast<Real>( -1.0 );
      return ret;
    }*/

    inline mat4 buildOrthographic( Real left, Real right, Real bottom, Real top, Real znear, Real zfar )
    {
      return glm::ortho( left, right, bottom, top, znear, zfar );
      /*// assert( znear > 0.0f && zfar > 0.0f );
      auto invw = ( numbers::one / ( right - left ) );
      auto invh = ( numbers::one / ( top - bottom ) );
      auto invd = ( numbers::one / ( zfar - znear ) );
      auto a = ( numbers::two * invw );
      auto b = ( numbers::two * invh );
      auto c = - ( right + left ) * invw;
      auto d = - ( top + bottom ) * invh;
      auto q = - numbers::two * invd;
      auto qn = - ( zfar + znear ) * invd;
      auto ret = mat4::zero();
      ret[0][0] = a;
      ret[0][3] = c;
      ret[1][1] = b;
      ret[1][3] = d;
      ret[2][2] = q;
      ret[2][3] = qn;
      ret[3][3] = static_cast<Real>( 1.0 );
      return ret;*/
    }

    //! \fn inline T projection( T v, T normal )
    //! \brief Vector projection.
    //! \param a The vector.
    //! \param b The normal.
    //! \return Vector projection.
    template <typename T>
    constexpr T projection( T v, T normal )
    {
      return ( glm::proj( v, normal ) );
    }

    //! \fn inline T rejection( T v, T normal )
    //! \brief Vector rejection.
    //! \param a The vector.
    //! \param b The normal.
    //! \return Vector rejection ( v - projection( v, normal ) ).
    template <typename T>
    constexpr T rejection( T v, T normal )
    {
      return ( v - projection( v, normal ) );
    }

    //! \fn inline Real angleBetween( const T& v1, const T& v2 )
    //! \brief Angle between two vectors.
    //! \param [in] v1 The first vector.
    //! \param [in] v2 The second vector.
    //! \returns The angle between the vectors in radians.
    template <typename T>
    inline Real angleBetween( const T& v1, const T& v2 )
    {
      auto lenProd = glm::length( v1 ) * glm::length( v2 );
      if ( lenProd < 1e-6f )
        lenProd = 1e-6f;
      auto f = ( glm::dot( v1, v2 ) / lenProd );
      return math::acos( math::clamp( f, -( glm::one<Real>() ), glm::one<Real>() ) );
    }

    //! \fn inline quaternion quaternionFrom( Real pitch, Real yaw, Real roll )
    //! \brief Build a quaternion from pitch/yaw/roll rotation values.
    //! \param [in] pitch Rotation along X axis.
    //! \param [in] yaw Rotation along Y axis.
    //! \param [in] roll Rotation along Z axis.
    //! \returns Quaternion from pitch/yaw/roll.
    inline quaternion quaternionFrom( Real pitch, Real yaw, Real roll )
    {
      return glm::toQuat( glm::yawPitchRoll( yaw, pitch, roll ) );
    }

    inline quaternion quaternionFrom( Radians angle, vec3 axis )
    {
      return angleAxis( angle, axis );
    }

    //! \fn inline T interpolateLinear( const T& v1, const T& v2, const Real interp )
    //! Linear interpolation between v1..v2 at interp[0..1]
    template <typename T>
    constexpr T interpolateLinear( const T& v1, const T& v2, const Real interp )
    {
      return ( v1 * ( numbers::one - interp ) + v2 * interp );
    }

    //! \fn constexpr T interpolateSmoothstep(const T& v1, const T& v2, const float interp)
    //! Smoothstep interpolation between v1..v2 at interp[0..1]
    //! \param v1 Start value
    //! \param v2 End value
    //! \param interp Interpolation phase between [0..1]
    //! \return Interpolated result of type T
    template <typename T>
    constexpr T interpolateSmoothstep( const T& v1, const T& v2, const float interp )
    {
      auto t = ( interp * interp * ( numbers::three - numbers::two * interp ) );
      return ( v1 * ( numbers::one - t ) + v2 * t );
    }

    //! \fn constexpr T interpolateSmootherstep(const T& v1, const T& v2, const float interp)
    //! Ken Perlin's smootherstep interpolation between v1..v2 at interp[0..1]
    //! \param v1 Start value
    //! \param v2 End value
    //! \param interp Interpolation phase between [0..1]
    //! \return Interpolated result of type T
    template <typename T>
    constexpr T interpolateSmootherstep( const T& v1, const T& v2, const float interp )
    {
      auto t = ( interp * interp * interp * ( interp * ( interp * numbers::six - numbers::fifteen ) + numbers::ten ) );
      return ( v1 * ( numbers::one - t ) + v2 * t );
    }

    //! \fn inline T interpolateCosine( const T& v1, const T& v2, const Real interp )
    //! Cosine interpolation between v1..v2 at interp[0..1]
    template <typename T>
    constexpr T interpolateCosine( const T& v1, const T& v2, const Real interp )
    {
      auto t = ( numbers::one - cos( interp * numbers::pi ) ) / numbers::two;
      return ( v1 * ( numbers::one - t ) + v2 * t );
    }

    //! \fn inline T interpolateCubic( const T& v0, const T& v1, const T& v2, const T& v3, const Real interp )
    //! Cubic spline between v1..v2 at interp[0..1]
    template <typename T>
    constexpr T interpolateCubic( const T& v0, const T& v1, const T& v2, const T& v3, const Real interp )
    {
      auto t = interp * interp;
      auto a0 = v3 - v2 - v0 + v1;
      auto a1 = v0 - v1 - a0;
      auto a2 = v2 - v0;
      return ( a0 * interp * t + a1 * t + a2 * interp + v1 );
    }

    //! \fn inline T interpolateQuadraticBezier( const T& v1, const T& v2, const T& control, const Real interp )
    //! Quadratic Beziér interpolation between v1..v2 at interp[0..1]
    //! using control point control
    template <typename T>
    constexpr T interpolateQuadraticBezier( const T& v1, const T& v2, const T& control, const Real interp )
    {
      auto t = ( numbers::one - interp );
      return t * t * v1 + numbers::two * t * interp * control + interp * interp * v2;
    }

  }

  struct rotator
  {
    Real roll = numbers::zero;
    Real pitch = numbers::zero;
    Real yaw = numbers::zero;
    rotator( const quat& q )
    {
      auto rs = numbers::two * ( q.w * q.x + q.y * q.z );
      auto rc = numbers::one - numbers::two * ( q.x * q.x + q.y * q.y );
      roll = degrees( math::atan2( rs, rc ) );
      auto pd = ( q.w * q.y - q.x * q.z );
      auto ps = math::sqrt( numbers::one + numbers::two * pd );
      auto pc = math::sqrt( numbers::one - numbers::two * pd );
      pitch = degrees( numbers::two * math::atan2( ps, pc ) - numbers::pi / numbers::two );
      auto ys = numbers::two * ( q.w * q.z + q.x * q.y );
      auto yc = numbers::one - numbers::two * ( q.y * q.y + q.z * q.z );
      yaw = degrees( math::atan2( ys, yc ) );
    }
    inline quat toQ() const
    {
      simd::vec4f sines, cosines;
      simd::vec4f src(
        radians( roll * numbers::half ),
        radians( pitch * numbers::half ),
        radians( yaw * numbers::half ),
        0.0f );
      src.sincos( sines, cosines );

      quat q {};
      q.w = cosines.e0 * cosines.e1 * cosines.e2 + sines.e0 * sines.e1 * sines.e2;
      q.x = sines.e0 * cosines.e1 * cosines.e2 - cosines.e0 * sines.e1 * sines.e2;
      q.y = cosines.e0 * sines.e1 * cosines.e2 + sines.e0 * cosines.e1 * sines.e2;
      q.z = cosines.e0 * cosines.e1 * sines.e2 - sines.e0 * sines.e1 * cosines.e2;
      return q;
    }
  };

}