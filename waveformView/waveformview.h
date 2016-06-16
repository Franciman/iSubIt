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

#include "minblankinfo.h"

#include <iostream>

class SubtitleModel;
class RangeList;

enum UpdateFlags
{
    Position = 2,
    PageSize = 4,
    Selection = 8,
    PlayCursor = 16,
    Cursor = 32,
    TheRange = 64
};

enum class DynamicEditMode
{
    Start,
    End,
    None
};

class WaveformView : public QAbstractScrollArea
{
    Q_OBJECT


    class ViewportWidget : public QWidget
    {
        QPixmap *Offscreen;
    public:
        ViewportWidget(QWidget *parent = 0) :
                QWidget(parent),
                Offscreen(nullptr)
        {
            setAttribute(Qt::WA_OpaquePaintEvent);
        }

        void setOffscreen(QPixmap *offscreen)
        {
            Offscreen = offscreen;
        }

    protected:
        virtual void paintEvent(QPaintEvent *) override;
    };

    Peaks PeakList;

    std::vector<RangeList*> DisplayRangeLists;

    QPixmap OffscreenWav;
    QPixmap Offscreen2;
    QPixmap Offscreen;

    SubtitleModel *Subs;


    Range *SelectedRange = nullptr;
    Range TheSelection;
    int SelectionOrigin = -1;

    int OldSelectionStart;
    int OldSelectionEnd;

    bool NeedSortSelectedSub = false;


    int PageSizeMs;
    int PositionMs;

    int LengthMs;

    int OldPositionMs;
    int OldPageSizeMs;

    int OldPlayCursorMs;
    int PlayCursorMs;

    int CursorMs;

    int VerticalScaling;

    int DisplayRulerHeight;

    bool MouseIsDown = false;

    bool ShowTextInRanges = true;

    bool IsPlaying = false;

    bool AutoScrolling = true;

    DynamicEditMode DEMode;
    int DynamicEditTime;
    Range DynamicEditSelection;
    Range *DynamicSelRangeOld = nullptr;
    Range *DynamicSelRange = nullptr;

    bool SnappingEnabled = true;
    bool EnableMouseAntiOverlapping = true;

    bool Clipping = false;
    QRect ClippingRect;

    int MinSelTime;
    int MaxSelTime;

    int MinimumBlank = 1;
    MinBlankInfo Info1, Info2;
    bool ShowMinimumBlank = true;

    bool SceneChangeEnabled = true;

    int ScrollOrigin = -1;

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

    void mouseDoubleClick(QMouseEvent *ev);
    void wheel(QWheelEvent *ev);
    void mousePress(QMouseEvent *ev);
    void mouseMove(QMouseEvent *ev);
    void mouseRelease(QMouseEvent *ev);
    void showContextMenu(QContextMenuEvent *ev);

private:
    void updateView(int flags);
    void paintWav(QPainter &painter, bool TryOptimize);
    void paintRuler(QPainter &painter);
    void paintMinimumBlank(QPainter &painter, int rangeTop, int rangeBottom);
    void paintRanges(QPainter &painter, const RangeList &Subs, int topPos, int bottomPos, bool topLine, bool bottomLine);
    void paintSelection(QPainter &painter);
    void paintCursor(QPainter &painter);
    void paintPlayCursor(QPainter &painter);
    void drawAlphaRect(QPainter &painter, int t1, int t2, int y1, int y2);



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
            return std::round(time / (PageSizeMs / Viewport->width()));
        }
    }
    inline int pixelToTime(int pixel) const
    {
        return std::round(pixel * (PageSizeMs / Viewport->width()));
    }

    void setPositionMs(int pos);

    RangeList &getRangeListFromPos(int y);
    void setSelectedRange(Range *subRange, bool UpdateDisplay);
    void mousePressCoolEdit(QMouseEvent *ev, int &UpdateFlags, RangeList &RL);

    int findSnappingPos(int PosMs, RangeList &RL);
    int findCorrectedSnappingPos(int Pos, RangeList &RL);

    bool selectionIsEmpty() const
    {
        return TheSelection.EndTime <= 0;
    }

    bool setMinBlankOnIdx(int idx, RangeList &RL);
    bool setMinBlankAt(int TimeMs, RangeList &RL);
    void clearSelection()
    {
        setSelectedRange(nullptr, false);
        updateView(Selection | TheRange);
    }

    bool checkSubtitleForDynamicSelection(Range *R, int CursorPosMs, int RangeSelWindow, const QPoint &mousePos, RangeList &RL);

public Q_SLOTS:
    void changeModel(const QString &, SubtitleModel *model);
    void changeSubs(QModelIndex, QModelIndex, QVector<int>);
    void startPlaying()
    {
        IsPlaying = true;
    }

    void stopPlaying()
    {
        IsPlaying = false;
        updateView(PlayCursor);
    }

    void changePlayCursorPos(int Time);

    //void changeSubtitleSelection(int index, SrtSubtitle &Sub);

Q_SIGNALS:
    //void selectSubtitle(int, SrtSubtitle &);
    void rangeStartDoubleClick(Range *);
    void rangeStopDoubleClick(Range *);
    void selectedRange(Range *, bool);
    void selectedRangeChange();
    void selectedRangeChanged(bool needToSort);
    void selectionChanged();
    void cursorChange();

};


#endif // WAVEFORMVIEW_H
