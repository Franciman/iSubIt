#ifndef MINBLANKINFO_H
#define MINBLANKINFO_H

#include "subtitlemodel.h"

enum class MinBlankInfoPart
{
    Start,
    End,
    Invalid
};

class MinBlankInfo
{
    MinBlankInfoPart Part;
    Range *R;
public:
    MinBlankInfo() :
        Part(MinBlankInfoPart::Invalid),
        R(nullptr)
    { }

    bool exists() const
    {
        return R;
    }

    bool setInfo(Range *NewRange, MinBlankInfoPart NewPart)
    {
        bool Res = false;
        if(R)
        {
            if(R != NewRange)
            {
                R = NewRange;
                Res = true;
            }
            if(Part != NewPart)
            {
                Part = NewPart;
                Res = true;
            }
        }
        else
        {
            R = NewRange;
            Part = NewPart;
            Res = false;
        }
        return Res;
    }

    int getStart(int MinBlankTime) const
    {
        if(Part == MinBlankInfoPart::Start)
        {
            return R->StartTime - MinBlankTime;
        }
        else
        {
            return R->EndTime;
        }
    }

    int getStop(int MinBlankTime) const
    {
        if(Part == MinBlankInfoPart::Start)
        {
            return R->StartTime;
        }
        else
        {
            return R->EndTime + MinBlankTime;
        }
    }

    int getSnappingPoint(int MinBlankTime) const
    {
        if(Part == MinBlankInfoPart::Start)
        {
            return getStart(MinBlankTime);
        }
        else
        {
            return getStop(MinBlankTime);
        }
    }
};

#endif // MINBLANKINFO_H
