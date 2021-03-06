#include "videoplayer.h"
#include <stdexcept>
#include <QOpenGLContext>
#include <QMetaObject>

#include "subtitlemodel.h"

static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod((VideoPlayer*)ctx, "on_mpv_events", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name)
{
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if(!glctx) return nullptr;
    return (void *)glctx->getProcAddress(QByteArray(name));
}

VideoPlayer::VideoPlayer(QWidget *parent, Qt::WindowFlags f) :
    QOpenGLWidget(parent, f),
    //PosUpdateTimer(this)
    PosUpdate(mpv)
{
    mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
    if(!mpv)
        throw std::runtime_error("could not create mpv context");

    //mpv_set_option_string(mpv, "terminal", "yes");
    //mpv_set_option_string(mpv, "msg-level", "all=v");

    if(mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    mpv::qt::set_option_variant(mpv, "vo", "opengl-cb");

    mpv_gl = (mpv_opengl_cb_context *)mpv_get_sub_api(mpv, MPV_SUB_API_OPENGL_CB);
    if(!mpv_gl)
        throw std::runtime_error("OpenGL not compiled in");
    mpv_opengl_cb_set_update_callback(mpv_gl, VideoPlayer::on_update, (void*)this);
    connect(this, SIGNAL(frameSwapped()), SLOT(swapped()));

    //mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_set_wakeup_callback(mpv, wakeup, this);
    pause();
    TimePosition = 0;
    connect(&PosUpdate, SIGNAL(timeChanged(int)), this, SLOT(timeChange(int)));
    //connect(&PosUpdateTimer, SIGNAL(timeout()), this, SLOT(updateTime()));
    //PosUpdateTimer.start(40);
}

void TimeUpdater::run()
{
    double val;
    int TimePosition;
    while(true)
    {
        usleep(10);
        mpv_get_property(mpv, "time-pos", MPV_FORMAT_DOUBLE, &val);
        TimePosition = val * 1000;
        emit timeChanged(TimePosition);
    }
}

VideoPlayer::~VideoPlayer()
{
    makeCurrent();
    if(mpv_gl)
        mpv_opengl_cb_set_update_callback(mpv_gl, nullptr, nullptr);
    mpv_opengl_cb_uninit_gl(mpv_gl);
}

void VideoPlayer::updateTime()
{
    //double t;
    //mpv_get_property(mpv, "time-pos", MPV_FORMAT_DOUBLE, &t);
    mpv_get_property_async(mpv, 123, "time-pos", MPV_FORMAT_DOUBLE);
    //TimePosition = t * 1000;
    //Q_EMIT positionChanged(TimePosition);
}

void VideoPlayer::command(const QVariant &params)
{
    mpv::qt::command_variant(mpv, params);
}

void VideoPlayer::setProperty(const QString &name, const QVariant &value)
{
    mpv::qt::set_property_variant(mpv, name, value);
}

QVariant VideoPlayer::getProperty(const QString &name) const
{
    return mpv::qt::get_property_variant(mpv, name);
}

void VideoPlayer::initializeGL()
{
    int r = mpv_opengl_cb_init_gl(mpv_gl, nullptr, get_proc_address, nullptr);
    if(r < 0)
        throw std::runtime_error("could not initialize OpenGL");
}

void VideoPlayer::paintGL()
{
    mpv_opengl_cb_draw(mpv_gl, defaultFramebufferObject(), width(), -height());
}

void VideoPlayer::swapped()
{
    mpv_opengl_cb_report_flip(mpv_gl, 0);
}

void VideoPlayer::on_mpv_events()
{
    while(mpv)
    {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if(event->event_id == MPV_EVENT_NONE)
            break;
        handle_mpv_event(event);
    }
}

void VideoPlayer::play()
{
    int f = 0;
    mpv_set_property_async(mpv, 0, "pause", MPV_FORMAT_FLAG, &f);
    //PosUpdateTimer.start(20);
    PosUpdate.start();
}

void VideoPlayer::pause()
{
    int f = 1;
    PosUpdate.quit();
    mpv_set_property_async(mpv, 0, "pause", MPV_FORMAT_FLAG, &f);
    //PosUpdateTimer.stop();
    updateTime();
}

void VideoPlayer::changeModel(const QString &filenameVideo, SubtitleModel *subs)
{
    track_filename = filenameVideo;
    command(QStringList() << "loadfile" << track_filename);
    command(QStringList() << "sub-add" << subs->tmpFilePath() << "select");

    connect(subs, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(subsUpdated()));
}

void VideoPlayer::subsUpdated()
{
    command(QStringList() << "sub-reload");
}

void VideoPlayer::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_GET_PROPERTY_REPLY:
        TimePosition = *(double*)((mpv_event_property*)event->data)->data * 1000;
        //emit positionChanged(TimePosition);
        timeChange(TimePosition);
        break;
    //case MPV_EVENT_PROPERTY_CHANGE:
        //TimePosition = *(double*)((mpv_event_property*)event->data)->data * 1000;
        //emit positionChanged(TimePosition);
        //break;
    default:
        break;
    }
}

void VideoPlayer::on_update(void *ctx)
{
    QMetaObject::invokeMethod((VideoPlayer*)ctx, "update");
}
