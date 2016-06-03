#include "subtitlemodel.h"

#include "srtParser/srtparser.h"
#include "waveformView/timemstoshortstring.h"

#include <stdexcept>

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
