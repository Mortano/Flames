#include "FlameCalculator.h"
#include <random>
#include <opencv2/core/mat.hpp>

namespace flame
{
   FlameCalculator::FlameCalculator( const FlameFunctionSet& functions, size_t width, size_t height, size_t superSampling ) :
      _functions( functions ),
      _histogram( width * superSampling, height * superSampling ),
      _superSampling( superSampling ),
      _isRunning( false )
   {
   }

   void FlameCalculator::Start()
   {
      if ( _isRunning ) throw std::exception( "Can't start FlameCalculator twice!" );
      _isRunning = true;
      _executor = std::thread( [this]() { Iterate(); } );
   }

   void FlameCalculator::Stop()
   {
      if ( !_isRunning ) return;
      _isRunning = false;
      _executor.join();
   }

   void FlameCalculator::TakeSnapshot(SimpleHistogram_t& otherHistogram) const
   {
      std::lock_guard<std::mutex> guard( _snapshotMutex );
      _histogram.CopyTo( otherHistogram );
   }

   void FlameCalculator::Iterate()
   {
      //std::random_device rnd;
      XorShiftRnd rnd;
      std::uniform_real_distribution<float> zeroOneDistribution( 0.f, 1.f );
      std::uniform_real_distribution<float> minusOneOneDistribution( -1.f, 1.f );

      cv::Point2f point = { minusOneOneDistribution( rnd ), minusOneOneDistribution( rnd ) };
      Color3_8 lastColor;

      //How many iterations are done within each critical section
      constexpr auto IterationGranularity = 2 << 14;

      while ( _isRunning )
      {
         _snapshotMutex.lock();

         for ( auto i = 0; i < IterationGranularity; i++ )
         {
            auto& rndFunction = RandomFunction( zeroOneDistribution( rnd ) );
            point = rndFunction( point );            

            auto hx = static_cast<int>( ( point.x + 1 ) * ( _histogram.GetWidth() / 2 ) );
            auto hy = static_cast<int>( ( point.y + 1 ) * ( _histogram.GetHeight() / 2 ) );

            if ( hx < 0 || hx >= _histogram.GetWidth() || hy < 0 || hy >= _histogram.GetHeight() ) continue;

            auto& histogramEntry = _histogram[{hx, hy}];
            histogramEntry.count++;

            auto& curColor = rndFunction.IsColorPreserving() ? lastColor : rndFunction.GetColor();
            histogramEntry.color = histogramEntry.color.BlendWith( curColor, 0.5f );
            lastColor = curColor;
         }

         _snapshotMutex.unlock();

         //std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
      }
   }

   const FlameFunction& FlameCalculator::RandomFunction( float uniformRnd ) const
   {
      auto accum = 0.f;
      for ( auto& func : _functions.GetFunctions() )
      {
         accum += func.first;
         if ( uniformRnd < accum ) return func.second;
      }
      return _functions.GetFunctions().back().second;
   }
}
