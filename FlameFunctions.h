#pragma once

#include <opencv2/core/core.hpp>
#include "Colors.h"
#include <numeric>

namespace flame
{

   namespace impl
   {
      inline cv::Point2f VariationLinear( const cv::Point2f& p )
      {
         return p;
      }

      inline cv::Point2f VariationSpherical( const cv::Point2f& p )
      {
         auto rSqrInv = 1.f / ( p.x * p.x + p.y * p.y );
         return{ p.x * rSqrInv, p.y * rSqrInv };
      }

      inline cv::Point2f VariationSinusoidal( const cv::Point2f& p )
      {
         return{ std::sin( p.x ), std::sin( p.y ) };
      }

      inline cv::Point2f VariationSwirl( const cv::Point2f& p )
      {
         auto rSqr = p.x*p.x + p.y*p.y;
         auto sinR = std::sin( rSqr );
         auto cosR = std::cos( rSqr );
         return{ p.x*sinR - p.y*cosR, p.x*cosR + p.y*sinR };
      }

      inline cv::Point2f VariationHeart( const cv::Point2f& p )
      {
         auto r = std::sqrtf( p.x * p.x + p.y * p.y );
         auto theta = std::atan( p.x / p.y );
         return{ r * std::sin( theta * r ), -r * std::cos( theta * r ) };
      }
   }

   //! \brief Variation functions
   struct Variations
   {
      using Func_t = cv::Point2f( *)( const cv::Point2f& );
      static constexpr Func_t Linear = impl::VariationLinear;
      static constexpr Func_t Spherical = impl::VariationSpherical;
      static constexpr Func_t Sinusoidal = impl::VariationSinusoidal;
      static constexpr Func_t Swirl = impl::VariationSwirl;
      static constexpr Func_t Heart = impl::VariationHeart;
   };

   struct Coefficients
   {
      union
      {
         struct
         {
            float a, b, c, d, e, f;
         };
         float data[6];
      };

      static Coefficients Build( float a, float b, float c, float d, float e, float f )
      {
         return{ a,b,c,d,e,f };
      }
   };

   struct FuncData
   {
      Variations::Func_t func;
      Coefficients coefficients;
      float weight;
   };

   //! \brief Encapsulates a single fractal flame function
   class FlameFunction
   {
   public:
      FlameFunction( std::initializer_list<Variations::Func_t> variations,
                     std::initializer_list<Coefficients> coefficients,
                     std::initializer_list<float> weights ) :
         _isColorPreserving( true )
      {
         assert( variations.size() == coefficients.size() );
         assert( variations.size() == weights.size() );
         _variations.reserve( variations.size() );
         for ( size_t idx = 0; idx < variations.size(); idx++ )
         {
            _variations.push_back( { *( variations.begin() + idx ), *( coefficients.begin() + idx ), *( weights.begin() + idx ) } );
         }
      }

      FlameFunction( std::initializer_list<Variations::Func_t> variations,
                     std::initializer_list<Coefficients> coefficients,
                     std::initializer_list<float> weights,
                     const Color3_8& color ) :
         FlameFunction( std::move( variations ), std::move( coefficients ), std::move( weights ) )
      {
         _color = color;
         _isColorPreserving = false;
      }

      FlameFunction( const FlameFunction& ) = default;
      FlameFunction( FlameFunction&& ) = default;

      //! \brief Applies all variations to the given point 
      cv::Point2f operator()( const cv::Point2f& point ) const
      {
         cv::Point2f ret;
         for ( const auto& funcData : _variations )
         {
            const auto& c = funcData.coefficients;
            ret += funcData.weight * funcData.func( { point.x * c.a + point.y * c.b + c.c, point.x * c.d + point.y * c.e + c.f } );
         }
         return ret;
      }

      const Color3_8& GetColor() const { return _color; }
      auto IsColorPreserving() const { return _isColorPreserving; }

   private:
      std::vector<FuncData> _variations;
      Color3_8 _color;
      bool _isColorPreserving;
   };

   //! \brief Symmetry types
   enum class Symmetry
   {
      MirrorX,
      MirrorY,
      Rotate180,
      Rotate120,
      Rotate90,
      Rotate72,
      Rotate60
   };

   std::vector<FlameFunction> MakeSymmetryFunction( Symmetry symmetry );

   //! \brief Stores multiple functions and their probabilities
   class FlameFunctionSet
   {
   public:
      //! \brief Returns pairs of functions and their probabilities
      const auto& GetFunctions() const { return _functions; }

      void AddFunction( FlameFunction&& function, float probability )
      {
         _functions.emplace_back( probability, std::move( function ) );
      }

      void AddSymmetries( std::initializer_list<Symmetry> symmetries )
      {
         NormalizeProbabilities();
         auto symmetriesCount = std::accumulate(symmetries.begin(), symmetries.end(), 1, [](auto base, Symmetry sym)
         {
            switch (sym)
            {            
            case Symmetry::Rotate120: return base + 2;
            case Symmetry::Rotate90: return base + 3;
            case Symmetry::Rotate72: return base + 4;
            case Symmetry::Rotate60: return base + 5;
            default: return base + 1;
            }
         });
         auto symmetryProbability = 1.f / symmetriesCount;

         //Adjust probabilities of the other functions
         for ( auto& pair : _functions ) pair.first /= symmetriesCount;

         for( auto symmetry : symmetries )
         {
            auto symFunctions = MakeSymmetryFunction( symmetry );
            for( auto& symFunc : symFunctions )
            {
               _functions.emplace_back( symmetryProbability, std::move(symFunc) );
            }            
         }
      }

      //! \brief Normalizes the probabilities of all functions so that they sum up to 1
      void NormalizeProbabilities()
      {
         auto sumOfProbabilities = std::accumulate( _functions.begin(), _functions.end(), 0.f, []( auto& l, auto& r )
         {
            return l + r.first;
         } );
         auto invProbabilities = 1.f / sumOfProbabilities;
         for ( auto& pair : _functions ) pair.first *= invProbabilities;
      }
   private:
      using Pair_t = std::pair<float, FlameFunction>;
      std::vector<Pair_t> _functions;
   };

}
