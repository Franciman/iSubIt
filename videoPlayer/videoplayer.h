#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QOpenGLWidget>
#include <QTimer>
#include <mpv/client.h>
#include <mpv/opengl_cb.h>
#include <mpv/qthelper.hpp>

class SubtitleModel;

class VideoPlayer : public QOpenGLWidget
{
    Q_OBJECT

public:
    VideoPlayer(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~VideoPlayer();
    void command(const QVariant &params);
    void setProperty(const QString &name, const QVariant &value);
    QVariant getProperty(const QString &name) const;
    QSize sizeHint() const { return QSize(480, 270); }

Q_SIGNALS:
    void positionChanged(int value);
protected:
    void initializeGL() override;
    void paintGL() override;
private Q_SLOTS:
    void swapped();
    void on_mpv_events();
    void updateTime();
public Q_SLOTS:
    void play();
    void pause();
    void changeModel(const QString &filenameVideo, SubtitleModel *subs);
    void subsUpdated();
private:
    void handle_mpv_event(mpv_event *event);
    static void on_update(void *ctx);

    QTimer PosUpdateTimer;
    mpv::qt::Handle mpv;
    mpv_opengl_cb_context *mpv_gl;
    int TimePosition;
    QString track_filename;
    QString subs_filename;
};

#endif // VIDEOPLAYER_H
