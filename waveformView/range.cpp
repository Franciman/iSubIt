#include "range.h"
#include <string.h>
#include "karasplit.h"

/*void Range::addSubTime(int NewTime)
{
    QVector::iterator InsertIt;
    while(InsertIdx != SubTime.end())
    {
        if(*InsertIt > NewTime)
            break;
        ++InsertIdx;
    }
    ++InsertIdx;

    SubTime.insert(InsertIdx, NewTime);
}

void Range::clearSubTimes()
{
    SubTime.clear();
}

void Range::deleteSubTimeAt(int Idx)
{
    SubTime.remove(Idx);
}

int Range::getSubTimeRange(int PosMs, Range *R)
{
    int Result = -1;
    if((SubTime.size() <= 0) || (PosMs < StartTime) || (PosMs > StopTime))
    {
        return Result;
    }

    int KStart = -1;
    int KEnd = -1;
    for(int i = 0; i < SubTime.size(); i++)
    {
        if(PosMs < SubTime[i])
        {
            Result = i;
            if(i > 0)
            {
                KStart = SubTime[i-1];
            }
            else
            {
                KStart = StartTime;
            }
            KStop = SubTime[i];
            break;
        }
    }
    if(KStart == -1)
    {
        Result = SubTime.size();
        KStart = SubTime[Result-1];
        KStop = StopTime;
    }
    if(R)
    {
        R->StartTime = KStart;
        R->StopTime = KStop;
    }
}

void Range::getSubTimeRangeAt(int Idx, Range *R)
{
    if((SubTime.size() <= 0) || (Idx < 0) || (Idx >= SubTime.size()))
        return;

    if(Idx == 0)
    {
        R->StartTime = StartTime;
        R->StopTime = SubTime[0];
    }
    else if(Idx == SubTime.size() - 1)
    {
        R->StartTime = SubTime[Idx - 1];
        R->StopTime = SubTime[Idx];
    }
    else
    {
        R->StartTime = SubTime[Idx - 1];
        R->StopTime = SubTime[Idx];
    }
}

bool Range::updateSubTimeFromText(QString &Text)
{
    QString WordArray;
    QVector<int> KTimeArray, AbsTimeArray;
    int AccuTime;
    bool AreTimesDifferent;

    KaraSplit(Text, KTimeArray);

    if(KTimeArray.empty())
    {
        if(!SubTime.empty())
        {
            clearSubTimes();
        }
        return true;
    }

    AbsTimeArray.reserve(KTimeArray.size() - 1);
    AccuTime = StartTime;
    for(int i = 0; i < KTimeArray.size() - 1; i++)
    {
        AccuTime = AccuTime + (KTimeArray[i] * 10);
        AbsTimeArray[i] = AccuTime;
    }

    AreTimesDifferent = false;
    if(AbsTimeArray.size() != SubTime.size())
    {
        AreTimesDifferent = true;
    }
    else
    {
        int i = 0;
        while(i < AbsTimeArray.size())
        {
            if(AbsTimeArray[i] != SubTime[i])
            {
                AreTimesDifferent = true;
                break;
            }
            i++;
        }
    }

    if(AreTimesDifferent)
    {
        for(int i = 0; i < AbsTimeArray.size(); i++)
        {
            SubTime[i] = AbsTimeArray[i];
        }
    }
    return AreTimesDifferent;
}

bool Range::setTimes(int Start, int Stop)
{
    int Result = false;
    if(StartTime != Start)
    {
        StartTime = Start;
        Result = true;
    }
    if(StopTime != Stop)
    {
        StopTime = Stop;
        Result = true;
    }
    return Result;
}

QString Range::toString() const
{
    char testo[256];
    sprintf(test, "[%d, %d]", StartTime, StopTime);
    return QString(testo);
}

int Range::getDuration() const
{
    return StopTime - StartTime;
}

int Range::getMiddle() const
{
    return StartTime + (getDuration() / 2);
}
*/
