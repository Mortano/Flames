#pragma once
#include <cstdint>
#include "Colors.h"
#include <vector>
#include <algorithm>
#include "MathUtil.h"

namespace flame
{

   struct HistogramEntry
   {
      static constexpr HistogramEntry Blank() { return{ 0, Color3_8(), 0 }; }

      uint32_t count;
      Color3_8 color;
      uint8_t  unused;
   };

   template<typename _EntryType>
   class Histogram
   {
   public:
      Histogram( size_t width, size_t height ) :
         _width( width ),
         _height( height )
      {
         _entries.resize( width * height );
      }

      Histogram( const Histogram& ) = default;

      //! \brief Access an entry in this histogram with the given {x,y} coordinates
      _EntryType& operator[]( const std::pair<size_t, size_t>& idx )
      {
#ifdef _DEBUG
         if ( idx.first >= _width || idx.second >= _height ) throw std::exception( "Index out of bounds!" );
#endif
         return _entries[( idx.second * _width ) + idx.first];
      }

      _EntryType& operator[]( size_t idx )
      {
         return _entries[idx];
      }

      void Clear()
      {
         for ( auto& entry : _entries )
         {
            entry = _EntryType::Blank();
         }
      }

      //! \brief Copies the content of this histogram to the given other histogram
      void CopyTo( Histogram& other ) const
      {
         if ( _width != other._width || _height != other._height ) throw std::exception( "Size mismatch!" );
         auto thisPtr = _entries.data();
         auto otherPtr = other._entries.data();
         std::memcpy( otherPtr, thisPtr, _width * _height * sizeof( _EntryType ) );
      }

      //! \brief Resolves the histogram into a range of colors. The range has to be big enough to store all
      //!        the entries of the histogram, divided by the superSampling squared
      template<typename RndIter>
      void Resolve( RndIter begin, RndIter end, size_t superSampling = 1 );

      auto GetWidth() const { return _width; }
      auto GetHeight() const { return _height; }

   private:
      const size_t _width, _height;
      std::vector<_EntryType> _entries;
   };

   //! \brief A simple histogram that does not support concurrent access
   using SimpleHistogram_t = Histogram<HistogramEntry>;

   namespace impl
   {
      //! \brief Resolve histogram with supersampling of 1 (i.e. no supersampling)
      template<typename RndIter>
      void ResolveImpl_SS1( RndIter begin, const std::vector<HistogramEntry>& histogram, size_t width, size_t height, float maxIntensity )
      {
         for ( const auto& entry : histogram )
         {
            auto intensity = std::log2f( static_cast<float>( entry.count ) ) / maxIntensity;
            *begin++ = entry.color * intensity;
         }
      }

      template<typename RndIter>
      void ResolveImpl_SS2( RndIter begin, const std::vector<HistogramEntry>& histogram, size_t width, size_t height, float maxIntensity )
      {
         //Loop unrolling for better performance
         const auto scaleFactor = 1.f / ( 4 * maxIntensity );
         for ( size_t y = 0; y < height; y += 2 )
         {
            for ( size_t x = 0; x < width; x += 2 )
            {
               auto& e1 = histogram[y*width + x];
               auto& e2 = histogram[y*width + x + 1];
               auto& e3 = histogram[( y + 1 )*width + x];
               auto& e4 = histogram[( y + 1 )*width + x + 1];

               //auto i1 = std::log2f( static_cast<float>( e1.count ) ) * scaleFactor;
               //auto i2 = std::log2f( static_cast<float>( e2.count ) ) * scaleFactor;
               //auto i3 = std::log2f( static_cast<float>( e3.count ) ) * scaleFactor;
               //auto i4 = std::log2f( static_cast<float>( e4.count ) ) * scaleFactor;

               auto i1 = static_cast<float>( FastLog2( e1.count ) ) * scaleFactor;
               auto i2 = static_cast<float>( FastLog2( e2.count ) ) * scaleFactor;
               auto i3 = static_cast<float>( FastLog2( e3.count ) ) * scaleFactor;
               auto i4 = static_cast<float>( FastLog2( e4.count ) ) * scaleFactor;

               *begin++ = ( e1.color * i1 + e2.color * i2 + e3.color * i3 + e4.color * i4 );
            }
         }
      }

      template<typename RndIter>
      void ResolveImpl_SSHigh( RndIter begin, const std::vector<HistogramEntry>& histogram, size_t width, size_t height, float maxIntensity, size_t ss )
      {
         auto invSamples = 1.f / ss;
         for ( size_t y = 0; y < height; y += ss )
         {
            for ( size_t x = 0; x < width; x += ss )
            {
               Color3_16 accumulator;
               for ( auto ssy = y; ssy < y + ss; ssy++ )
               {
                  for ( auto ssx = x; ssx < x + ss; ssx++ )
                  {
                     const auto& entry = histogram[ssy*width + ssx];
                     auto intensity = std::log2f( static_cast<float>( entry.count ) ) / maxIntensity;
                     accumulator += entry.color * intensity;
                  }
               }
               *begin++ = Color3_8(
                  static_cast<uint8_t>( accumulator.r * invSamples ),
                  static_cast<uint8_t>( accumulator.g * invSamples ),
                  static_cast<uint8_t>( accumulator.b * invSamples )
               );
            }
         }
      }
   }

   template <>
   template <class RndIter>
   void Histogram<HistogramEntry>::Resolve( RndIter begin, RndIter end, size_t superSampling )
   {
#ifdef _DEBUG
      auto dist = std::distance( begin, end );
      if ( dist != ( _width / superSampling ) * ( _height / superSampling ) ) throw std::exception( "Range has the wrong size!" );
#endif
      auto maxCountIter = std::max_element( _entries.begin(), _entries.end(), []( const auto& l, const auto& r )
      {
         return l.count < r.count;
      } );
      auto logMaxCount = std::log2f( static_cast<float>( ( *maxCountIter ).count ) );

      switch ( superSampling )
      {
      case 1:
         impl::ResolveImpl_SS1( begin, _entries, _width, _height, logMaxCount );
         break;
      case 2:
         impl::ResolveImpl_SS2( begin, _entries, _width, _height, logMaxCount );
         break;
      default:
         impl::ResolveImpl_SSHigh( begin, _entries, _width, _height, logMaxCount, superSampling );
         break;
      }
   }

   template<typename _EntryType>
   void MergeHistograms( std::vector<Histogram<_EntryType>>& histograms )
   {
      auto& dst = histograms[0];
      for ( size_t h = 1; h < histograms.size(); h++ )
      {
         auto& from = histograms[h];
         for ( size_t idx = 0; idx < from.GetWidth() * from.GetHeight(); idx++ )
         {
            dst[idx].count += from[idx].count;
            dst[idx].color = dst[idx].color.BlendWith( from[idx].color, 0.5f );
         }
      }
   }

}
