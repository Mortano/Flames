#pragma once

template<typename T1, typename T2>
struct SizeLessThan
{
   static constexpr bool value = sizeof( T1 ) < sizeof( T2 );
};

template<typename T1, typename T2>
constexpr bool SizeLessThan_v = SizeLessThan<T1, T2>::value;

template<typename T1, typename T2>
struct SizeLessEqualThan
{
   static constexpr bool value = sizeof( T1 ) <= sizeof( T2 );
};

template<typename T1, typename T2>
constexpr bool SizeLessEqualThan_v = SizeLessEqualThan<T1, T2>::value;