#pragma once
#include <cstdint>
#include <chrono>

inline uint32_t FastLog2( uint32_t v )
{
   constexpr static unsigned int b[] = { 0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000 };
   constexpr static unsigned int S[] = { 1, 2, 4, 8, 16 };

   uint32_t r = 0; // result of log2(v) will go here
   for ( auto i = 4; i >= 0; i-- ) // unroll for speed...
   {
      if ( v & b[i] )
      {
         v >>= S[i];
         r |= S[i];
      }
   }
   return r;
}

//! \brief Fast, simple random number generator
class XorShiftRnd
{
public:
   XorShiftRnd() :
      //Don't ask...
      x( std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count() ),
      y( 362436069 ),
      z( 521288629 )
   {
   }

   uint32_t operator()()
   {
      uint32_t t;
      x ^= x << 16;
      x ^= x >> 5;
      x ^= x << 1;

      t = x;
      x = y;
      y = z;
      z = t ^ x ^ y;

      return z;
   }

   auto min() const { return 0; }
   auto max() const { return static_cast<uint32_t>( -1 ); }
private:
   uint32_t x, y, z;
};