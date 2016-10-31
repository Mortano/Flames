#pragma once
#include "Histogram.h"
#include "FlameFunctions.h"
#include <thread>
#include <mutex>
#include <atomic>

namespace flame
{
   
   //! \brief Performs the calculations for a fractal flame into a histogram. This is done on a unique thread
   class FlameCalculator
   {
   public:
      using Ptr = std::unique_ptr<FlameCalculator>;

      FlameCalculator( const FlameFunctionSet& functions, size_t width, size_t height, size_t superSampling );

      void Start();
      void Stop();

      void TakeSnapshot(SimpleHistogram_t& otherHistogram) const;
   private:
      void Iterate();

      const FlameFunction& RandomFunction( float uniformRnd ) const;

      const FlameFunctionSet& _functions;
      SimpleHistogram_t _histogram;
      const size_t _superSampling;

      std::thread _executor;
      mutable std::mutex _snapshotMutex;
      std::atomic_bool _isRunning;
   };

}
