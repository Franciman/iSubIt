#ifndef RANGE_H
#define RANGE_H
/*
#include <QVector>
#include <QColor>

struct Range
{
    int StartTime;
    int StopTime;
    QVector<int> SubTime;

    QColor MarkColor;
    int IconIndex;

    Range() { }
    Range(const Range &other) { }

    void addSubTime(int NewTime);
    void clearSubTimes();
    void deleteSubTimeAt(int Idx);
    // !!! Indexes used below are index of the interval between markers !!!
    int getSubTimeRange(int PosMs, Range *R);
    void getSubTimeRangeAt(int Idx, Range *R);
    bool updateSubTimeFromText(QString &Text);
    bool setTimes(int Start, int Stop); // Returns true if changed
    QString toString() const;
    int getDuration() const;
    int getMiddle() const;

    bool operator==(const Range &other) const;

    bool operator!=(const Range &other) const;

    bool operator<(const Range &other) const
    {
        if(StartTime == other.StartTime)
        {
            return StopTime < other.StopTime;
        }
        else
        {
            return StartTime < other.StartTime;
        }
    }
};

struct RangeFactory
{
    Range *createRange()
    {
        return new Range;
    }

    Range *createRangeSS(int Start, int Stop)
    {
        Range *Result = createRange();
        if(Start <= Stop)
        {
            Result->StartTime = Start;
            Result->StopTime = Stop;
        }
        else
        {
            Result->StartTime = Stop;
            Result->StopTime = Start;
        }
        return Result;
    }
};


*/
#endif // RANGE_H
