#include <iostream>
#include <random>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "FlameFunctions.h"
#include "Histogram.h"
#include "FlameCalculator.h"
#include <future>

using namespace flame;

const int WinWidth = 1024;
const int WinHeight = 1024;
const int SuperSampling = 2;

int main( int argc, char** argv )
{
   const auto wndName = "Flames";
   const auto bpp = 3;

   cv::Mat mat = cv::Mat::zeros( WinWidth, WinHeight, CV_8UC3 );
   cv::namedWindow( wndName );

   FlameFunctionSet ffs;
   ffs.AddFunction(
      FlameFunction( { Variations::Linear }, { Coefficients::Build( 0.3f, 0, 0, 0, 0.3f, 0 ) }, { 1.f }, Color3_8( 138, 43, 226 ) ),
      0.33f
   );

   ffs.AddFunction(
      FlameFunction(
   { Variations::Heart, Variations::Sinusoidal },
   { Coefficients::Build( 0.3f, 0, 0, 0, 0.3f, 0.5f ), Coefficients::Build( 0.3f, 0.3f, 0.2f, 0.3f, 0.7f, 0.4f ) },
   { 0.8f, 0.2f }, Color3_8( 153, 50, 204 ) ),
      0.33f
   );

   ffs.AddFunction(
      FlameFunction( { Variations::Spherical }, { Coefficients::Build( 0.3f, 0, 0.5f, 0, 0.3f, 0 ) }, { 1.f }, Color3_8( 255, 105, 180 ) ),
      0.33f
   );

   ffs.AddSymmetries( { Symmetry::Rotate72 } );

   const auto Threads = 7;
   std::vector<SimpleHistogram_t> snapshotHistograms;
   std::vector<FlameCalculator::Ptr> calculators;
   for ( auto idx = 0; idx < Threads; idx++ )
   {
      snapshotHistograms.emplace_back( WinWidth * SuperSampling, WinHeight * SuperSampling );
      calculators.push_back(
         std::make_unique<FlameCalculator>( ffs,
                                            static_cast<size_t>( WinWidth ),
                                            static_cast<size_t>( WinHeight ),
                                            static_cast<size_t>( SuperSampling ) ) );
      calculators[idx]->Start();
   }

   std::vector<Color3_8> colors;
   colors.resize( WinWidth * WinHeight );

   while ( true )
   {
      for ( auto t = 0; t < Threads; t++ )
      {
         calculators[t]->TakeSnapshot( snapshotHistograms[t] );
      }
      flame::MergeHistograms( snapshotHistograms );
      snapshotHistograms[0].Resolve( colors.begin(), colors.end(), 2 );

      auto matPtr = mat.data;

      for ( auto y = 0; y < WinHeight; y++ )
      {
         for ( auto x = 0; x < WinWidth; x++ )
         {
            auto& col = colors[y * WinWidth + x];
            col.CopyToInverse( matPtr );
            matPtr += bpp;
         }
      }

      cv::imshow( wndName, mat );
      if ( cv::waitKey( 100 ) >= 0 ) break;
   }

   for ( auto& calc : calculators ) calc->Stop();

   return 0;
}
