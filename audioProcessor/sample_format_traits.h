#ifndef SAMPLE_FORMAT_TRAITS_H
#define SAMPLE_FORMAT_TRAITS_H

#include <cstdint>
#include <cmath>

using std::int16_t;
using std::int32_t;
using std::uint8_t;

template<class SampleFormat>
struct sample_format_traits
{
    inline static int16_t convertToInt16(SampleFormat sample);
};

template<>
inline int16_t sample_format_traits<uint8_t>::convertToInt16(uint8_t sample)
{
    return sample;
}

template<>
inline int16_t sample_format_traits<int16_t>::convertToInt16(int16_t sample)
{
    return sample;
}

template<>
inline int16_t sample_format_traits<int32_t>::convertToInt16(int32_t sample)
{
    return sample;
}

template<>
inline int16_t sample_format_traits<float>::convertToInt16(float sample)
{
   int32_t res = std::round(sample * INT16_MAX);
   if(res > INT16_MAX)
   {
       res = INT16_MAX;
   }
   else if(res < INT16_MIN)
   {
       res = INT16_MIN;
   }
   return sample_format_traits<int32_t>::convertToInt16(res);
}

template<>
inline int16_t sample_format_traits<double>::convertToInt16(double sample)
{
    int32_t res = std::round(sample * INT16_MAX);
    if(res > INT16_MAX)
    {
        res = INT16_MAX;
    }
    else if(res < INT16_MAX)
    {
        res = INT16_MIN;
    }
    return sample_format_traits<int32_t>::convertToInt16(res);
}


#endif // SAMPLE_FORMAT_TRAITS_H
