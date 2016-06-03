#ifndef EXTRACTWAVDIALOG_H
#define EXTRACTWAVDIALOG_H

#include <QDialog>
#include "mediaProcessor/peaks.h"

namespace Ui {
class ExtractWavDialog;
}

class MediaExtractor;
class MediaFile;

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

private:
    Ui::ExtractWavDialog *ui;
    MediaFile *File;
    MediaExtractor *Extractor;
    Peaks peaks;
};

#endif // EXTRACTWAVDIALOG_H
