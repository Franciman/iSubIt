#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "subtitlemodel.h"
#include "newprojectdialog.h"

#include "extractwavdialog.h"

#include "srtParser/srtparser.h"

#include <QFile>
#include <QTemporaryFile>
#include <QMessageBox>

#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    Model(nullptr),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->playButton, SIGNAL(clicked()), ui->videoPlayer, SLOT(play()));
    connect(ui->playButton, SIGNAL(clicked()), ui->waveform, SLOT(startPlaying()));
    connect(ui->pauseButton, SIGNAL(clicked()), ui->videoPlayer, SLOT(pause()));
    connect(ui->pauseButton, SIGNAL(clicked()), ui->waveform, SLOT(startPlaying()));

    connect(ui->videoPlayer, SIGNAL(positionChanged(int)), this, SLOT(updateTime(int)));

    connect(ui->videoPlayer, SIGNAL(positionChanged(int)), ui->waveform, SLOT(changePlayCursorPos(int)));

    connect(ui->actionNew_Project, SIGNAL(triggered()), this, SLOT(newProject()));


    connect(this, SIGNAL(changeProject(QString, SubtitleModel*)), ui->videoPlayer, SLOT(changeModel(QString,SubtitleModel*)));
    connect(this, SIGNAL(changeProject(QString,SubtitleModel*)), ui->waveform, SLOT(changeModel(QString,SubtitleModel*)));
    connect(ui->subtitleList, SIGNAL(doubleClicked(QModelIndex)), SLOT(updateText(QModelIndex)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete Model;
}

void MainWindow::updateTime(int TimeMs)
{
    ui->timeLabel->setText(QString::number(TimeMs));
}

void MainWindow::updateText(QModelIndex idx)
{
    ui->plainTextEdit->setPlainText(Model->getSubtitleAt(idx.row()));
}

void MainWindow::newProject()
{
    NewProjectDialog *dialog = new NewProjectDialog(this);
    if(dialog->exec() == QDialog::Accepted)
    {
        ExtractWavDialog *wavDialog = new ExtractWavDialog(dialog->videoFile(), this);
        if(wavDialog->exec() == QDialog::Accepted)
        {
            try
            {
                delete Model;
                if(dialog->VOFile().isEmpty())
                {
                    Model = new SubtitleModel(true, dialog->subtitleFile().toStdString().c_str(), this);
                }
                else
                {
                    Model = new SubtitleModel(true, dialog->subtitleFile().toStdString().c_str(), false, dialog->VOFile().toStdString().c_str(), this);
                }
                emit changeProject(dialog->videoFile(), Model);
                ui->subtitleList->setModel(Model);
                ui->waveform->updatePeaks(wavDialog->getPeaks());
            }
            catch(const std::runtime_error &Error)
            {
                QMessageBox::critical(this, "I/O Error", Error.what());
            }
        }
    }

}
