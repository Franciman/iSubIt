#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>

namespace Ui {
class MainWindow;
}

class SubtitleModel;

class WaveformView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private Q_SLOTS:
    void newProject();
    void updateText(QModelIndex idx);
    void updateTime(int TimeMs);


Q_SIGNALS:
    void changeProject(const QString &, SubtitleModel *);

private:
    SubtitleModel *Model;
    Ui::MainWindow *ui; 
};

#endif // MAINWINDOW_H
