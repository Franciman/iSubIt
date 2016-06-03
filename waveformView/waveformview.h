#ifndef WAVEFORMVIEW_H
#define WAVEFORMVIEW_H

#include <QImage>
#include <QPixmap>
#include <QWidget>
#include <QAbstractScrollArea>
#include <QCache>
#include <QStaticText>
#include <QThread>

#include "srtParser/srtsubtitle.h"

#include <cmath>

#include "mediaProcessor/mediafile.h"
#include "mediaProcessor/mediaprocessor.h"
#include "mediaProcessor/ffmpegerror.h"

#include <iostream>

class SubtitleModel;
class RangeList;

enum UpdateFlags
{
    Position = 2,
    PageSize = 4,
    Selection = 8
};

class WaveformView : public QAbstractScrollArea
{
    Q_OBJECT


    class ViewportWidget : public QWidget
    {
        QImage *Offscreen;
    public:
        ViewportWidget(QWidget *parent = 0) :
                QWidget(parent),
                Offscreen(nullptr)
        { }

        void setOffscreen(QImage *offscreen)
        {
            Offscreen = offscreen;
        }

    protected:
        virtual void paintEvent(QPaintEvent *) override;
    };

    Peaks PeakList;

    std::vector<RangeList*> DisplayRangeLists;

    QImage OffscreenWav;
    QImage Offscreen;

    SubtitleModel *Subs;


    Range Selection;
    int SelectionOrigin;


    int PageSizeMs;
    int PositionMs;

    int LengthMs;

    int OldPositionMs;
    int OldPageSizeMs;

    int CursorMs;

    int VerticalScaling;

    int DisplayRulerHeight;

    bool MouseIsDown;

    bool ShowTextInRanges;


    ViewportWidget *Viewport;
public:
    WaveformView(QWidget *parent = 0);

    void zoomRange(Range &range);
    void zoomRange(int Start, int End);
    void zoomCenteredOn(int Center, int PageSize);
    void zoomAll();
    void zoomIn();
    void zoomOut();
    void zoomAndSelectRange(Range &range);

protected:
    virtual void scrollContentsBy(int, int) override;

    //virtual void paintEvent(QPaintEvent *ev) override;

    virtual bool viewportEvent(QEvent *ev) override;

    virtual void mouseDoubleClickEvent(QMouseEvent *ev) override;
    virtual void wheelEvent(QWheelEvent *ev) override;
    virtual void mousePressEvent(QMouseEvent *ev) override;
    virtual void mouseMoveEvent(QMouseEvent *ev) override;
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
private:
    void updateView(int flags);
    void paintWav(QPainter &painter, bool TryOptimize);
    void paintRuler(QPainter &painter);
    void paintRanges(QPainter &painter, const RangeList &Subs, int topPos, int bottomPos, bool topLine, bool bottomLine);
    void paintSelection(QPainter &painter);
    void paintCursor(QPainter &painter);
    void paintPlayCursor(QPainter &painter);



public:
    void updatePeaks(Peaks &&P);

private:

    inline int timeToPixel(int time) const
    {
        if(PageSizeMs == 0)
            return 0;
        else
        {
            //std::cout << double(Viewport->width()) << std::endl;
            //std::cout << double(PageSizeMs) / Viewport->width() << std::endl;
            return std::round(time / (double(PageSizeMs) / Viewport->width()));
        }
    }
    inline int pixelToTime(int pixel) const
    {
        return std::round(pixel * (PageSizeMs / Viewport->width()));
    }

    void setPositionMs(int pos)
    {
        if(pos < 0) pos = 0;
        if(pos > LengthMs - PageSizeMs) pos = LengthMs - PageSizeMs;
        if(pos != PositionMs)
        {
            PositionMs = pos;
            updateView(Position);
        }
    }

    RangeList &getRangeListFromPos(int y);
    void setSelectedRange(Range &subRange);
    void mousePressCoolEdit(QMouseEvent *ev, int &UpdateFlags, RangeList &RL);

public Q_SLOTS:
    void changeModel(const QString &, SubtitleModel *model);
    void changeSubs(QModelIndex, QModelIndex, QVector<int>);
    //void changeSubtitleSelection(int index, SrtSubtitle &Sub);

Q_SIGNALS:
    //void selectSubtitle(int, SrtSubtitle &);

};


#endif // WAVEFORMVIEW_H
