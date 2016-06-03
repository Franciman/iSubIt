#ifndef EXTRACTWAVDIALOG_H
#define EXTRACTWAVDIALOG_H

#include <QDialog>
#include "mediaProcessor/peaks.h"

#include <thread>

namespace Ui {
class ExtractWavDialog;
}

class MediaExtractor;
class MediaFile;

struct AVStream;

class ExtractWavDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExtractWavDialog(const QString &videoFile, QWidget *parent = 0);
    ~ExtractWavDialog();

    Peaks &&getPeaks() { return std::move(peaks); }

private Q_SLOTS:
    void enableOkButton();
    void disableExtractButton();
    void startExtraction();
    void trackProgress(int progress);

private:
    Ui::ExtractWavDialog *ui;
    std::vector<AVStream *> AudioStreams;
    MediaFile *File;
    MediaExtractor *Extractor;
    Peaks peaks;
};

#endif // EXTRACTWAVDIALOG_H
