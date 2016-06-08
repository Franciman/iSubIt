#include "subtitlemodel.h"

#include "srtParser/srtparser.h"
#include "waveformView/timemstoshortstring.h"

#include <stdexcept>
#include <algorithm>

#include "constrain.h"

int compareRanges(const Range &R1, const Range &R2)
{
    if(R1.StartTime < R2.StartTime) return 1;
    else if(R1.StartTime > R2.StartTime) return -1;
    else
    {
        if(R1.EndTime < R2.EndTime) return 1;
        else if(R1.EndTime > R2.EndTime) return -1;
        else return 0;
    }
}

int RangeList::findInsertPos(const Range &R) const
{
    int Min = 0;
    int Max = Subtitles.size() - 1;
    int Mid = (Max + Min) / 2;
    const Range *RangeCursor;
    int CompareResult;
    while(Min <= Max)
    {
        RangeCursor = &Subtitles[Mid].Time;
        CompareResult = compareRanges(*RangeCursor, R);
        if(CompareResult == 1) // RangeCursor < R
        {
            Min = Mid + 1;
        }
        else if(CompareResult == -1) // RangeCursor > R
        {
            Max = Mid - 1;
        }
        else // RangeCursor = R
        {
            return Mid;
        }
        Mid = (Max + Min) / 2;
    }
    return Min;
}

Range *RangeList::findFirstRangeAt(int PosMs, int ExpandBy)
{
    if(Subtitles.size() == 0) return nullptr;

    SearchStartAt = PosMs;
    SearchExpandBy = ExpandBy;
    SearchIdx = findInsertPos(SearchStartAt - SearchExpandBy, -1);
    Constrain(SearchIdx, 0, (int)Subtitles.size() - 1);
    while(SearchIdx > 0 && Subtitles[SearchIdx].Time.EndTime > SearchStartAt + SearchExpandBy)
    {
        --SearchIdx;
    }
    return findNextRange();
}

Range *RangeList::findNextRange()
{
    Range *Result = nullptr;
    while(SearchIdx >= 0 && SearchIdx < Subtitles.size())
    {
        Range &R = Subtitles[SearchIdx].Time;
        SearchIdx++;
        if(R.StartTime - SearchExpandBy > SearchStartAt) return Result;
        else if(R.StartTime - SearchExpandBy <= SearchStartAt && R.EndTime + SearchExpandBy >= SearchStartAt)
        {
            Result = &R;
        }
    }
    return Result;
}

void RangeList::fullSort()
{
    std::sort(Subtitles.begin(), Subtitles.end(), [](SrtSubtitle S1, SrtSubtitle S2)
    {
        return compareRanges(S1.Time, S2.Time) > 0;
    });
}

int RangeList::getRangeIdxAt(int pos) const
{
    int Res = -1;
    if(Subtitles.empty()) return Res;

    int i = findInsertPos(pos, -1);
    if(i < 0) i = 0;
    if(i >= Subtitles.size()) i = Subtitles.size() - 1;
    while(i >= 0)
    {
        if(Subtitles[i].Time.StartTime <= pos && Subtitles[i].Time.EndTime >= pos)
        {
            Res = i;
        }
        i--;
    }
    return Res;
}

std::vector<SrtSubtitle> readSubsFromFile(const char *filename)
{
    QFile File(filename);
    if(File.open(QFile::ReadOnly))
    {
        try
        {
            SrtParser Parser(&File);
            return std::move(Parser.parseSubs());
        }
        catch(SrtParseError &)
        {
            return {};
        }
    }
    else
    {
        return {};
    }
}

SubtitleModel::SubtitleModel(bool editable, std::vector<SrtSubtitle> &&Subs, QObject *parent) :
    QAbstractTableModel(parent),
    Subtitles(editable, std::move(Subs)),
    VOSubtitles(nullptr)
{
    if(!File.open())
        throw std::runtime_error(File.errorString().toStdString());
    saveToFile();
}

SubtitleModel::SubtitleModel(bool editable, const char *subtitles_filename, QObject *parent) :
    SubtitleModel(editable, readSubsFromFile(subtitles_filename), parent)
{
}

SubtitleModel::SubtitleModel(bool editable, std::vector<SrtSubtitle> &&Subs, bool VOeditable, std::vector<SrtSubtitle> &&VOSubs, QObject *parent) :
    SubtitleModel(editable, std::move(Subs), parent)
{
    VOSubtitles = new RangeList(VOeditable, std::move(VOSubs));
}

SubtitleModel::SubtitleModel(bool editable, const char *subtitles_filename, bool VOeditable, const char *vo_filename, QObject *parent) :
    SubtitleModel(editable, subtitles_filename, parent)
{
    VOSubtitles = new RangeList(VOeditable, readSubsFromFile(vo_filename));
}

SubtitleModel::~SubtitleModel()
{
    File.close();
    delete VOSubtitles;
}

int SubtitleModel::rowCount(const QModelIndex &) const
{
    return Subtitles.Subtitles.size();
}

int SubtitleModel::columnCount(const QModelIndex &) const
{
    return 4;
}

QVariant SubtitleModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DisplayRole)
    {
        if(index.row() >= Subtitles.Subtitles.size()) return QVariant();

        switch(index.column())
        {
        case 0:
            return index.row() + 1;
        case 1:
            return timeMsToShortString(Subtitles.Subtitles[index.row()].Time.StartTime, 1);
        case 2:
            return timeMsToShortString(Subtitles.Subtitles[index.row()].Time.EndTime, 1);
        case 3:
        {
            QString Text = Subtitles.Subtitles[index.row()].Text;
            return Text.replace('\n', '|');
        }
        }
    }
    return QVariant();
}

QVariant SubtitleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole)
    {
        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case 0:
                return QString("#");
            case 1:
                return QString("Start");
            case 2:
                return QString("Stop");
            case 3:
                return QString("Text");
            }
        }
    }
    return QVariant();
}

void SubtitleModel::saveToFile()
{
  for(auto &sub : Subtitles.Subtitles)
  {
      sub.writeToFile(&File);
  }
}
