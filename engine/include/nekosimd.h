#pragma once
#include "neko_types.h"

namespace neko {

  namespace simd {

    // clang-format off

    #define neko_avx_align __declspec( align( 32 ) )
    #define neko_avx_inline __forceinline

    neko_avx_align class vec8f {
    public:
      union
      {
        struct
        {
          float e0, e1, e2, e3, e4, e5, e6, e7;
        };
        float v[8];
        __m256 packed;
      };
      neko_avx_inline vec8f()
      {
        packed = _mm256_setzero_ps();
      }
      neko_avx_inline vec8f( const float value )
      {
        packed = _mm256_broadcast_ss( &value );
      }
      neko_avx_inline vec8f( const float* __restrict values )
      {
        packed = _mm256_load_ps( values );
      }
      neko_avx_inline vec8f( const __m256& rhs ): packed( rhs )
      {
      }
      neko_avx_inline vec8f( float _e0, float _e1, float _e2, float _e3, float _e4, float _e5, float _e6, float _e7 )
      {
        packed = _mm256_setr_ps( _e7, _e6, _e5, _e4, _e3, _e2, _e1, _e0 );
      }
      //! Load eight 32-byte boundary aligned sample values into vector
      neko_avx_inline void load( const float* __restrict values )
      {
        packed = _mm256_load_ps( values );
      }
      //! Load eight unaligned sample values into vector.
      //! For performance, prefer aligned loads instead.
      neko_avx_inline void loadUnaligned( const float* __restrict values )
      {
        packed = _mm256_loadu_ps( values );
      }
      //! Store eight sample values from vector to 32-byte boundary aligned memory
      //! using a non-temporal usage hint (data not reused, don't cache in SSB).
      //! Treating nontemporal data as temporal would be cache pollution = bad
      neko_avx_inline void storeNontemporal( float* __restrict values )
      {
        _mm256_stream_ps( values, packed );
      }
      //! Store eight sample values from vector to 32-byte boundary aligned memory
      //! using a temporal usage hint (data is reused, maybe cache in SSB).
      //! Subsequent stores of values cached in SSB may bypass L1, L2 & memctrl = good
      neko_avx_inline void storeTemporal( float* __restrict values )
      {
        _mm256_store_ps( values, packed );
      }
      //! Set all vector members to single value
      neko_avx_inline void set( const float* value )
      {
        packed = _mm256_broadcast_ss( value );
      }
      //! Set vector members individually
      neko_avx_inline void set( float _e0, float _e1, float _e2, float _e3, float _e4, float _e5, float _e6, float _e7 )
      {
        packed = _mm256_setr_ps( _e7, _e6, _e5, _e4, _e3, _e2, _e1, _e0 );
      }
      //! v = a + b
      neko_avx_inline vec8f operator+( const vec8f& rhs ) const
      {
        return _mm256_add_ps( packed, rhs.packed );
      }
      //! v = a - b
      neko_avx_inline vec8f operator-( const vec8f& rhs ) const
      {
        return _mm256_sub_ps( packed, rhs.packed );
      }
      //! v = a & b (bitwise and)
      neko_avx_inline vec8f operator&( const vec8f& rhs ) const
      {
        return _mm256_and_ps( packed, rhs.packed );
      }
      //! v = a | b (bitwise or)
      neko_avx_inline vec8f operator|( const vec8f& rhs ) const
      {
        return _mm256_or_ps( packed, rhs.packed );
      }
      //! v = a ^ b (bitwise xor)
      neko_avx_inline vec8f operator^( const vec8f& rhs ) const
      {
        return _mm256_xor_ps( packed, rhs.packed );
      }
      //! v = a * b
      neko_avx_inline vec8f operator*( const float scalar ) const
      {
        auto im = _mm256_set1_ps( scalar );
        return _mm256_mul_ps( packed, im );
      }
      //! v = a * b
      neko_avx_inline vec8f operator*( const vec8f& rhs ) const
      {
        return _mm256_mul_ps( packed, rhs.packed );
      }
      //! v = a / b
      neko_avx_inline vec8f operator/( const float scalar ) const
      {
        auto im = _mm256_set1_ps( scalar );
        return _mm256_div_ps( packed, im );
      }
      //! v = a / b
      neko_avx_inline vec8f operator/( const vec8f& rhs ) const
      {
        return _mm256_div_ps( packed, rhs.packed );
      }
      //! multiplication assignment
      neko_avx_inline vec8f& operator*=( const vec8f& rhs )
      {
        packed = _mm256_mul_ps( packed, rhs.packed );
        return *this;
      }
      //! division assignment
      neko_avx_inline vec8f& operator/=( const vec8f& rhs )
      {
        packed = _mm256_div_ps( packed, rhs.packed );
        return *this;
      }
      //! a == b
      neko_avx_inline bool operator==( const vec8f& rhs ) const
      {
        auto ret = _mm256_cmp_ps( packed, rhs.packed, _CMP_NEQ_OS );
        return ( _mm256_movemask_ps( ret ) == 0 );
      }
      //! a != b
      neko_avx_inline bool operator!=( const vec8f& rhs ) const
      {
        auto ret = _mm256_cmp_ps( packed, rhs.packed, _CMP_NEQ_OS );
        return ( _mm256_movemask_ps( ret ) != 0 );
      }
      //! v = round(a)
      neko_avx_inline vec8f round() const
      {
        return _mm256_round_ps( packed, _MM_FROUND_NINT );
      }
      //! v = ceil(a)
      neko_avx_inline vec8f ceil() const
      {
        return _mm256_round_ps( packed, _MM_FROUND_CEIL );
      }
      //! v = floor(a)
      neko_avx_inline vec8f floor() const
      {
        return _mm256_round_ps( packed, _MM_FROUND_FLOOR );
      }
      //! v = trunc(a)
      neko_avx_inline vec8f trunc() const
      {
        return _mm256_round_ps( packed, _MM_FROUND_TRUNC );
      }
      //! v = sqrt(a)
      neko_avx_inline vec8f sqrt() const
      {
        return _mm256_sqrt_ps( packed );
      }
      //! f = x + y + z + w
      neko_avx_inline float sum() const
      {
        auto hadd = _mm256_hadd_ps( packed, packed );
        auto hi = _mm256_extractf128_ps( hadd, 1 );
        auto lo = _mm256_castps256_ps128( hadd );
        auto ret = _mm_add_ss( lo, hi );
        return _mm_cvtss_f32( ret );
      }
      //! broadcast the 8 members of this block into four blocks of two 4-member vectors
      // [1,2,3,4,5,6,7,8]
      // =>
      // [1,1,1,1,2,2,2,2]
      // [3,3,3,3,4,4,4,4]
      // [5,5,5,5,6,6,6,6]
      // [7,7,7,7,8,8,8,8]
      neko_avx_inline void unpack8x4( vec8f& a, vec8f& b, vec8f& c, vec8f& d )
      {
        auto hipart = _mm256_extractf128_ps( packed, 1 );
        auto lopart = _mm256_castps256_ps128( packed );
        auto d0 = _mm_permute_ps( lopart, 0b00000000 );
        auto d1 = _mm_permute_ps( lopart, 0b01010101 );
        __m256 dummy256 = _mm256_castps128_ps256( d0 );
        a.packed = _mm256_insertf128_ps( dummy256, d1, 1 );
        d0 = _mm_permute_ps( lopart, 0b10101010 );
        d1 = _mm_permute_ps( lopart, 0b11111111 );
        dummy256 = _mm256_castps128_ps256( d0 );
        b.packed = _mm256_insertf128_ps( dummy256, d1, 1 );
        d0 = _mm_permute_ps( hipart, 0b00000000 );
        d1 = _mm_permute_ps( hipart, 0b01010101 );
        dummy256 = _mm256_castps128_ps256( d0 );
        c.packed = _mm256_insertf128_ps( dummy256, d1, 1 );
        d0 = _mm_permute_ps( hipart, 0b10101010 );
        d1 = _mm_permute_ps( hipart, 0b11111111 );
        dummy256 = _mm256_castps128_ps256( d0 );
        d.packed = _mm256_insertf128_ps( dummy256, d1, 1 );
      }
      //! v = a * b + c (fused multiply & add)
      static neko_avx_inline vec8f fma( const vec8f& a, const vec8f& b, const vec8f& c )
      {
        return _mm256_fmadd_ps( a.packed, b.packed, c.packed );
      }
      //! v = a * b - c (fused multiply & subtract)
      static neko_avx_inline vec8f fms( const vec8f& a, const vec8f& b, const vec8f& c )
      {
        return _mm256_fmsub_ps( a.packed, b.packed, c.packed );
      }
    };

  }

}