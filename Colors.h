#pragma once
#include <stdint.h>
#include <type_traits>

#include "TypeUtil.h"

namespace flame
{
   //! \brief RGB color structure
   template<typename _DataType>
   struct Color3
   {
      using DataType = _DataType;

      union
      {
         struct
         {
            _DataType r, g, b;
         };
         _DataType rgb[3];
      };

      constexpr Color3() : r( 0 ), g( 0 ), b( 0 ) {}
      constexpr Color3( _DataType r, _DataType g, _DataType b ) :
         r( r ), g( g ), b( b )
      {
      }

      constexpr Color3( const Color3& ) = default;
      Color3& operator=( const Color3& ) = default;

      template<typename _OtherDataType>
      std::enable_if_t<SizeLessEqualThan_v<_OtherDataType, _DataType>, Color3&>
         operator+=( const Color3<_OtherDataType>& other )
      {
         r += other.r;
         g += other.g;
         b += other.b;
         return *this;
      }

      //! \brief Blends this color with another color using the given ratio. The ratio of 1 means that 100% of the 
      //!        other color is used, a ratio of 0 means that 100% of this color is used.
      //! \param other Color to blend with
      //! \param ratio Ratio for blending
      //! \returns Blended color
      constexpr Color3 BlendWith( const Color3& other, float ratio ) const
      {
         return{
            static_cast<_DataType>( r*( 1.f - ratio ) + other.r*ratio ),
            static_cast<_DataType>( g*( 1.f - ratio ) + other.g*ratio ),
            static_cast<_DataType>( b*( 1.f - ratio ) + other.b*ratio )
         };
      }

      //! \brief Unsafe copy function that copies the values of this color to the given memory region
      void CopyTo( uint8_t* mem )
      {
         std::memcpy( mem, rgb, 3 * sizeof( _DataType ) );
      }

      //! \brief Unsafe copy function that copies the values of this color in inverse order (B,G,A) to the
      //!        given memory region
      void CopyToInverse( uint8_t* mem )
      {
         auto memCasted = reinterpret_cast<_DataType*>( mem );
         memCasted[0] = b;
         memCasted[1] = g;
         memCasted[2] = r;
      }

   };

   namespace
   {
      template<typename T1, typename T2>
      using BiggerType_t = std::conditional_t<( sizeof( T1 ) > sizeof( T2 ) ), T1, T2>;
   }

   template<typename _DataType1, typename _DataType2>
   constexpr Color3<BiggerType_t<_DataType1, _DataType2>>
      operator+( const Color3<_DataType1>& l, const Color3<_DataType2>& r )
   {
      using Bigger_t = BiggerType_t<_DataType1, _DataType2>;
      return{ static_cast<Bigger_t>( l.r + r.r ) , static_cast<Bigger_t>( l.g + r.g ), static_cast<Bigger_t>( l.b + r.b ) };
   }

   template<typename _DataType>
   constexpr Color3<_DataType> operator*( const Color3<_DataType>& col, float ratio )
   {
      return{
         static_cast<_DataType>( col.r * ratio ),
         static_cast<_DataType>( col.g * ratio ),
         static_cast<_DataType>( col.b * ratio )
      };
   }

   //! \brief Returns a random color initialized from the given random number provider
   //! \param rnd Function object that returns random numbers
   //! \returns Random color
   template<typename Color, typename Rnd>
   constexpr Color RandomColor( Rnd&& rnd )
   {
      using DataType = typename Color::DataType;
      return{
         static_cast<DataType>( rnd() % std::numeric_limits<DataType>::max() ),
         static_cast<DataType>( rnd() % std::numeric_limits<DataType>::max() ),
         static_cast<DataType>( rnd() % std::numeric_limits<DataType>::max() )
      };
   }   

   using Color3_8 = Color3<uint8_t>;
   using Color3_16 = Color3<uint16_t>;
   using Color3_32 = Color3<uint32_t>;
   using Color3_f = Color3<float>;

}
