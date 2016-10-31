#include "FlameFunctions.h"

namespace flame
{

   constexpr double Pi = 3.14159265;

   namespace
   {
      FlameFunction MakeSymmetryMirrorX()
      {
         return
         {
            {Variations::Linear},
            {Coefficients::Build(1,0,0,0,-1,0)},
            {1.f}
         };
      }

      FlameFunction MakeSymmetryMirrorY()
      {
         return
         {
            { Variations::Linear },
            { Coefficients::Build( -1,0,0,0,1,0 ) },
            { 1.f }
         };
      }

      FlameFunction MakeRotationFunction( double angle )
      {
         auto cosAlpha = static_cast<float>( std::cos( angle ) );
         auto sinAlpha = static_cast<float>( std::sin( angle ) );
         return
         {
            { Variations::Linear },
            { Coefficients::Build( cosAlpha, -sinAlpha, 0, sinAlpha, cosAlpha, 0 ) },
            { 1.f }
         };
      }

      FlameFunction MakeSymmetryRotate120()
      {
         return MakeRotationFunction( 2.0994f );
      }
   }

   std::vector<FlameFunction> MakeSymmetryFunction( Symmetry symmetry )
   {
      switch ( symmetry )
      {
      case Symmetry::MirrorX: return{ MakeSymmetryMirrorX() };
      case Symmetry::MirrorY: return{ MakeSymmetryMirrorY() };
      case Symmetry::Rotate180: return{ MakeRotationFunction( Pi ) };
      case Symmetry::Rotate120: return{ 
         MakeRotationFunction( Pi * 2.0 / 3.0 ), 
         MakeRotationFunction( Pi * 4.0 / 3.0 ) };
      case Symmetry::Rotate90: return{ 
         MakeRotationFunction( Pi * 0.5 ), 
         MakeRotationFunction( Pi ), 
         MakeRotationFunction( Pi * 1.5 ) };
      case Symmetry::Rotate72: return{
         MakeRotationFunction( Pi * 2.0 / 5.0 ),
         MakeRotationFunction( Pi * 4.0 / 5.0 ),
         MakeRotationFunction( Pi * 6.0 / 5.0 ),
         MakeRotationFunction( Pi * 8.0 / 5.0 )
      };
      case Symmetry::Rotate60: return{
         MakeRotationFunction( Pi * 1.0 / 3.0 ),
         MakeRotationFunction( Pi * 2.0 / 3.0 ),
         MakeRotationFunction( Pi * 3.0 / 3.0 ),
         MakeRotationFunction( Pi * 4.0 / 3.0 ),
         MakeRotationFunction( Pi * 5.0 / 3.0 )
      };
      default: return{};
      }
   }
}
