#include "newprojectdialog.h"
#include "ui_newprojectdialog.h"

#include <QSignalMapper>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

NewProjectDialog::NewProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewProjectDialog)
{
    ui->setupUi(this);

    QSignalMapper *mapper = new QSignalMapper(this);

    connect(ui->browseVideoButton, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(ui->browseSubtitleButton, SIGNAL(clicked()), mapper, SLOT(map()));
    connect(ui->browseVOButton, SIGNAL(clicked()), mapper, SLOT(map()));

    mapper->setMapping(ui->browseVideoButton, ui->videoEdit);
    mapper->setMapping(ui->browseSubtitleButton, ui->subEdit);
    mapper->setMapping(ui->browseVOButton, ui->voEdit);

    connect(mapper, SIGNAL(mapped(QWidget*)), this, SLOT(browse(QWidget*)));
}

NewProjectDialog::~NewProjectDialog()
{
    delete ui;
}

void NewProjectDialog::accept()
{
    if(ui->videoEdit->text().isEmpty())
    {
        QMessageBox::warning(this, "Missing data", "Missing video filename");
    }
    else if(ui->subEdit->text().isEmpty())
    {
        QMessageBox::warning(this, "Missing data", "Missing subtitle filename");
    }
    else
    {
        QFileInfo videoInfo(ui->videoEdit->text());
        QFileInfo subInfo(ui->subEdit->text());
        if(!(videoInfo.exists() || videoInfo.isFile()))
        {
            QMessageBox::warning(this, "Invalid data", "Invalid video filename");
        }
        else if(!(subInfo.exists() || subInfo.isFile()))
        {
            QMessageBox::warning(this, "Invalid data", "Invalid subtitle filename");
        }
        else
        {
            VideoFile = ui->videoEdit->text();
            SubtitleFile = ui->subEdit->text();

            QFileInfo voInfo(ui->voEdit->text());
            if(voInfo.isFile())
            {
                VOFilename = ui->voEdit->text();
            }
            else
            {
                VOFilename = "";
            }
            QDialog::accept();
        }
    }
}

void NewProjectDialog::browse(QWidget *lineEdit)
{
    QLineEdit *TheLineEdit = (QLineEdit*)lineEdit;
    QString path = TheLineEdit->text().isEmpty() ? "/home/francesco/Desktop" : TheLineEdit->text();
    QString result = QFileDialog::getOpenFileName(this, "Open file", path);
    TheLineEdit->setText(result);
}
