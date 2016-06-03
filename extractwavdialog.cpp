#include "extractwavdialog.h"
#include "ui_extractwavdialog.h"

#include "mediaProcessor/mediaprocessor.h"
#include "mediaProcessor/mediafile.h"
#include "mediaProcessor/ffmpegerror.h"

#include <QMessageBox>

extern "C"
{
#include <libavformat/avformat.h>
}

ExtractWavDialog::ExtractWavDialog(const QString &videoFile, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtractWavDialog)
{
    ui->setupUi(this);

    try
    {
        File = new MediaFile(videoFile.toStdString().c_str());
        AVStream **Streams = File->streams_begin();
        AVStream **StreamsEnd = File->streams_end();

        int i;
        ui->extractButton->setEnabled(false);

        for(i = 0; Streams != StreamsEnd; ++Streams)
        {
            if((*Streams)->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                ++i;
                QString text = QString("%1: %2").arg(i).arg((*Streams)->codec->codec_name);
                ui->audioStreams->addItem(text);
            }
        }
        if(i != 0)
        {
            ui->extractButton->setEnabled(true);
            ui->outputPlainTextEdit->setPlainText("Everything went ok");
            if(i == 1)
            {
                ui->audioStreams->setEnabled(false);
            }
        }
        else
        {
            ui->outputPlainTextEdit->setPlainText("Error:\nNo audio stream found.");
        }
    }
    catch(FFmpegError &err)
    {
        QString text(err.what());
        ui->outputPlainTextEdit->setPlainText(text);
    }

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->extractButton, SIGNAL(clicked()), this, SLOT(startExtraction()));
    connect(ui->extractButton, SIGNAL(clicked()), this, SLOT(disableExtractButton()));
}

ExtractWavDialog::~ExtractWavDialog()
{
    delete ui;
    delete File;
    delete Extractor;
}

void ExtractWavDialog::enableOkButton()
{
    if(Extractor->getException())
    {
        QMessageBox::critical(this, "Error", QString(Extractor->getException()))
    }
    ui->okButton->setEnabled(true);
}

void ExtractWavDialog::disableExtractButton()
{
    ui->extractButton->setEnabled(false);
}

void ExtractWavDialog::startExtraction()
{
    volatile int streamIndex = ui->audioStreams->currentIndex();
    if(streamIndex != -1)
    {
        AVStream **AudioStream = File->streams_begin() + streamIndex;
        AVStream **VideoStream = File->best_stream_of_type(AVMEDIA_TYPE_VIDEO);
        if(VideoStream == File->streams_end()) VideoStream = nullptr;
        Extractor = new MediaExtractor(*File, *AudioStream, nullptr, peaks);
        connect(Extractor, SIGNAL(finished()), this, SLOT(enableOkButton()));
        connect(ui->cancelButton, SIGNAL(clicked()), Extractor, SLOT(quit()));
        Extractor->start();
    }
}
