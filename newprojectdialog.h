#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>

namespace Ui {
class NewProjectDialog;
}

class NewProjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewProjectDialog(QWidget *parent = 0);
    ~NewProjectDialog();

    const QString &videoFile() const
    {
        return VideoFile;
    }

    const QString &subtitleFile() const
    {
        return SubtitleFile;
    }

    const QString &VOFile() const
    {
        return VOFilename;
    }

protected Q_SLOTS:
    virtual void accept() override;

private Q_SLOTS:
    void browse(QWidget *lineEdit);


private:
    QString VideoFile;
    QString SubtitleFile;
    QString VOFilename;
    Ui::NewProjectDialog *ui;
};

#endif // NEWPROJECTDIALOG_H
