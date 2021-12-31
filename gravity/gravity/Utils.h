#pragma once

// Quick reverse square root from Quake 3 source code 
inline float Q_rsqrt(float number)  noexcept
{
    int i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = *reinterpret_cast<int*>(&y);        // evil floating point bit level hacking
    i = 0x5f3759df - (i >> 1);              // what the hug?
    y = *reinterpret_cast<float*>(&i);
    y = y * (threehalfs - (x2 * y * y));    // 1st iteration

    return y;
}

template <typename T>
T LoopValue(const T& val, const T& minValue, const T& maxValue)  noexcept
{
    if (val >= minValue) // "likely()" == first branch of the if
    {
        if (val < maxValue)  // "likely()" == first branch of the if
            return val;
        return val - (maxValue - minValue);
    }
    return val + (maxValue - minValue);
}


template <typename T>
const T& ValueCap(const T& val, const  T& min, const T& max)  noexcept
{
    if (val >= min)
    {
        if (val <= max)
            return val;
        return max;
    }
    return min;
}


//float InterlockedCompareExchange(float volatile * _Destination, float _Exchange, float _Comparand)  noexcept
//{
//    static_assert(sizeof(float) == sizeof(long),
//        "InterlockedCompareExchange(float*,float,float): expect float to be same size as long");
//
//    auto res = ::_InterlockedCompareExchange(
//        reinterpret_cast<volatile long*>(_Destination),
//        *reinterpret_cast<long*>(&_Exchange),
//        *reinterpret_cast<long*>(&_Comparand)
//    );
//
//    return *reinterpret_cast<float*>(&res);
//}
