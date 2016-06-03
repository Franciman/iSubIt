#ifndef SUBTITLEMODEL_H
#define SUBTITLEMODEL_H

#include <QAbstractTableModel>
#include <QTemporaryFile>

#include "srtParser/srtsubtitle.h"

#include <iostream>

struct RangeList
{
    bool Editable;
    std::vector<SrtSubtitle> Subtitles;
    RangeList(bool editable, std::vector<SrtSubtitle> &&Subs) :
        Editable(editable),
        Subtitles(std::move(Subs))
    { }
};

class SubtitleModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SubtitleModel(bool editable, std::vector<SrtSubtitle> &&Subs, QObject *parent = 0);
    SubtitleModel(bool editable, std::vector<SrtSubtitle> &&Subs, bool VOeditable, std::vector<SrtSubtitle> &&VOSubs, QObject *parent = 0);
    SubtitleModel(bool editable, const char *subtitles_filename, QObject *parent = 0);
    SubtitleModel(bool editable, const char *subtitles_filename, bool VOeditable, const char *vo_filename, QObject *parent = 0);

    SubtitleModel(const SubtitleModel &) = delete;

    virtual ~SubtitleModel();

    SubtitleModel &operator=(const SubtitleModel &) = delete;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QString tmpFilePath() const
    {
        //std::cout << File.fileName().toStdString() << std::endl;
        return File.fileName();
    }

    const QString &getSubtitleAt(int idx) const
    {
        return Subtitles.Subtitles[idx].Text;
    }

    const QString &getVOSubtitleAt(int idx) const
    {
        return VOSubtitles->Subtitles[idx].Text;
    }

    RangeList &getSubtitles()
    {
        return Subtitles;
    }

    bool hasVOSubtitles() const
    {
        return VOSubtitles;
    }

    RangeList *getVOSubtitles()
    {
        return VOSubtitles;
    }

private:
    void saveToFile();

private:
    RangeList Subtitles;
    RangeList *VOSubtitles;
    QTemporaryFile File;
};

#endif // SUBTITLEMODEL_H
