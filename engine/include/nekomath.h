﻿#pragma once
#include "neko_types.h"
#include <algorithm>
#include <boost/math/constants/constants.hpp>

#ifdef NEKO_MATH_DOUBLE
# define NEKO_MATH_FUNC_ROUND ::round
# define NEKO_MATH_FUNC_SQRT ::sqrt
# define NEKO_MATH_FUNC_FLOOR ::floor
# define NEKO_MATH_FUNC_CEIL ::ceil
# define NEKO_MATH_FUNC_SIN ::sin
# define NEKO_MATH_FUNC_COS ::cos
# define NEKO_MATH_FUNC_ABS ::abs
#else
# define NEKO_MATH_FUNC_ROUND ::roundf
# define NEKO_MATH_FUNC_SQRT ::sqrtf
# define NEKO_MATH_FUNC_FLOOR ::floorf
# define NEKO_MATH_FUNC_CEIL ::ceilf
# define NEKO_MATH_FUNC_SIN ::sinf
# define NEKO_MATH_FUNC_COS ::cosf
# define NEKO_MATH_FUNC_ABS ::abs
#endif

namespace neko {

  namespace math {

    //! \fn inline Real round( Real value )
    //! \brief Rounds to the nearest non-decimal value.
    //! \param value The value.
    //! \returns Given value rounded to nearest non-decimal.
    inline Real round( Real value )
    {
      return NEKO_MATH_FUNC_ROUND( value );
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

    //! \fn inline Real floor( Real value )
    //! \brief Floors the given value.
    //! \param value The value.
    //! \returns Given value rounded down.
    inline Real floor( Real value )
    {
      return NEKO_MATH_FUNC_FLOOR( value );
    }

    //! \fn inline Real ceil( Real value )
    //! \brief Ceilings the given value.
    //! \param value The value.
    //! \returns Given value rounded up.
    inline Real ceil( Real value )
    {
      return NEKO_MATH_FUNC_CEIL( value );
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
    inline Real clamp( Real value, Real low, Real high )
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
    inline T manhattan( T x1, T y1, T x2, T y2 )
    {
      return ( abs( x2 - x1 ) + abs( y2 - y1 ) );
    }
    //! \fn inline T interpolateLinear( const T& v1, const T& v2, const Real interp )
    //! Linear interpolation between v1..v2 at interp[0..1]
    template <typename T>
    inline T interpolateLinear( const T& v1, const T& v2, const Real interp )
    {
      return ( v1 * ( NEKO_ONE - interp ) + v2 * interp );
    }

    //! \fn inline T interpolateCosine( const T& v1, const T& v2, const Real interp )
    //! Cosine interpolation between v1..v2 at interp[0..1]
    template <typename T>
    inline T interpolateCosine( const T& v1, const T& v2, const Real interp )
    {
      const Real t = ( NEKO_ONE - cos( interp * pi ) ) / ( NEKO_ONE * 2 );
      return ( v1 * ( NEKO_ONE - t ) + v2 * t );
    }

    //! \fn inline T interpolateCubic( const T& v0, const T& v1, const T& v2, const T& v3, const Real interp )
    //! Cubic spline between v1..v2 at interp[0..1]
    template <typename T>
    inline T interpolateCubic( const T& v0, const T& v1, const T& v2, const T& v3, const Real interp )
    {
      const Real t = interp * interp;
      T a0 = v3 - v2 - v0 + v1;
      T a1 = v0 - v1 - a0;
      T a2 = v2 - v0;
      return ( a0 * interp * t + a1 * t + a2 * interp + v1 );
    }

    //! \fn inline T interpolateQuadraticBezier( const T& v1, const T& v2, const T& control, const Real interp )
    //! Quadratic Beziér interpolation between v1..v2 at interp[0..1]
    //! using control point control
    template <typename T>
    inline T interpolateQuadraticBezier( const T& v1, const T& v2, const T& control, const Real interp )
    {
      const Real t = ( NEKO_ONE - interp );
      return t * t * v1 + 2 * t * interp * control + interp * interp * v2;
    }

  }

}