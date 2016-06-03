#ifndef RANGELIST_H
#define RANGELIST_H

#include "range.h"

/*class SrtSubtitle;

class RangeList
{
    bool Editable;
    QVector<Range> List;
    int SearchStartAt;
    int SearchIdx;
    int SearchExpandBy;


public:
    RangeList() :
        Editable(true)
    { }

    ~RangeList()
    {
        List.clear();
    }

    int add(Range &range)
    {
        int Result = findInsertPos(range);
        List.insert(Result, range);
        return Result;
    }

    QVector<Range>::iterator ranges_begin()
    {
        return List.begin();
    }

    QVector<Range>::iterator ranges_end()
    {
        return List.end();
    }

    Range *addAndReturnSibiling(const Range &r)
    {
        int InsertPos = findInsertPos(r);
        List.insert(InsertPos, r);
        if(InsertPos >= 0 && InsertPos < List.size())
        {
            return &List[InsertPos];
        }
        else
        {
            return nullptr;
        }
    }

    void addAtEnd(const Range &r)
    {
        List.push_back(r);
    }

    void fullSort();
    int findInsertPos(const Range &r) const;
    int findInsertPos(int Start, int Stop) const
    {
        Range R;
        R.StartTime = Start;
        if(Stop == -1)
        {
            R.StopTime = Start + 1;
        }
        else
        {
            R.StopTime = Stop;
        }
        return findInsertPos(R);
    }
    int getRangeIdxAt(int PosMs) const;
    void Delete(int index)
    {
        if(index >= 0 && index < List.size())
        {
            List.remove(index);
        }
    }

    int indexOf(const Range &r) const
    {
        List.indexOf(r);
    }
    void move(int currIndex, int NewIndex)
    {
        List.move(currIndex, NewIndex);
    }

    void clear()
    {
        List.clear();
    }

    Range *findFirstRangeAt(int PosMs, int expandBy = 0)
    {
        if(List.empty()) return nullptr;

        SearchStartAt = PosMs;
        SearchExpandBy = expandBy;
        SearchIdx = findInsertPos(SearchStartAt - SearchExpandBy, -1);
        while(SearchIdx > 0 && List[SearchIdx].StopTime > (SearchStartAt + SearchExpandBy))
        {
            --SearchIdx;
        }
        return findNextRange();
    }

    Range *findNextRange()
    {
        Range *Result = nullptr;
        int RangePos;
        while(SearchIdx >= 0 && SearchIdx < List.size())
        {
            RangePos = SearchIdx;
            ++SearchIdx;
            if(List[RangePos].StartTime - SearchExpandBy > SearchStartAt) return Result;
            else if(List[RangePos].StartTime - SearchExpandBy <= SearchStartAt && List[RangePos].StopTime + SearchExpandBy >= SearchStartAt)
            {
                RangePos = SearchIdx;
                return &List[RangePos];
            }
        }
    }

    bool editable() const { return Editable; }
    void editable(bool isEditable) { Editable = isEditable; }
    Range &getItem(int index)
    {
        return List[index];
    }

    int getCount() const
    {
        return List.count();
    }
};

struct SilentRangeInfo
{
    int Start;
    int Stop;
    double RmsSum;
    int RmsCount;
};

enum class SelectionMode
{
    CoolEdit,
    SSA
};

enum class DynamicEditMode
{
    None,
    Start,
    Stop,
    Karaoke
};

enum class MinBlankInfoPart
{
    Start,
    Stop,
    Invalid
};

class MinBlankInfo
{
    Range *R;
    MinBlankInfoPart Part;
public:
    MinBlankInfo();
    bool exists() const;
    bool setInfo(Range NewRange, MinBlankInfoPart NewPart);
    int getStart(int MinBlankTime) const;
    int getStop(int MinBlankTime) const;
    int getSnappingPoint(int MinBlankTime) const;
};*/

#endif // RANGELIST_H
