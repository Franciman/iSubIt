#include "rangelist.h"
#include <algorithm>
/*
void RangeList::fullSort()
{
    std::sort(List.begin(), List.end());
}

int RangeList::findInsertPos(const Range &r) const
{
    auto pos = std::lower_bound(List.begin(), List.end(), r);
    return std::distance(List.begin(), pos);
}

int RangeList::getRangeIdxAt(int PosMs) const
{
    Range R;
    int i;
    int Result = -1;
    if(List.empty()) return Result;

    i = findInsertPos(PosMs, -1);
    while(i >= 0)
    {
        R = List[i];
        if(R.StartTime <= PosMs && R.StopTime >= PosMs)
        {
            return i;
        }
        --i;
    }
}

MinBlankInfo::MinBlankInfo() :
    Range(nullptr)
{ }

bool MinBlankInfo::exists() const
{
    return Range;
}

bool MinBlankInfo::setInfo(Range NewRange, MinBlankInfoPart NewPart)
{
    int Result = false;
    if(!R|| (*R!= NewRange))
    {
        R= new Range(NewRange);
        Result = true;
    }
    if(Part != NewPart)
    {
        Part = NewPart;
        Result = true;
    }
    return Result;
}

int MinBlankInfo::getStart(int MinBlankTime) const
{
    if(Part == MinBlankInfoPart::Start)
    {
        return R->StartTime - MinBlankTime;
    }
    else
    {
        return R->StopTime;
    }
}

int MinBlankInfo::getStop(int MinBlankTime) const
{
    if(Part == MinBlankInfoPart::Start)
    {
        return R->StartTime;
    }
    else
    {
        return R->StopTime + MinBlankTime;
    }
}

int MinBlankInfo::getSnappingPoint(int MinBlankTime) const
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
*/
