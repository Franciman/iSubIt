#ifndef CONSTRAIN_H
#define CONSTRAIN_H

template<class T>
inline void Constrain(T &val, const T &min, const T &max)
{
    if(val < min)
    {
        val = min;
    }
    else if(val > max)
    {
        val = max;
    }
}



#endif // CONSTRAIN_H
