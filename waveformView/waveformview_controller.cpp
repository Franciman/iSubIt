#include "waveformview.h"
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>

#include <iostream>
#include <algorithm>

void WaveformView::mouseDoubleClickEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev)
    /*int y_coord = ev->pos().y();
    int x_coord = ev->pos().x();

    int x_ms = pixelToTime(x_coord) + PositionMs;

    // Ignore double clicks on the ruler
    if(y_coord >= height() - DisplayRulerHeight) return;

    RangeList &SelectedSubs = getRangeListFromPos(y_coord);

    // If these subtitles can't be edited, they can't be selected
    if(!SelectedSubs.Editable) return;

    // TODO: Add Dynamic edit mode

    auto it = std::lower_bound(SelectedSubs.Subs.begin(), SelectedSubs.Subs.end(), x_ms, [](SrtSubtitle &it, int x) {
        return it.Time.EndTime < x;
    });

    while(it != SelectedSubs.Subs.end() && it->Time.StartTime >= x_ms) ++it;

    // There is no range at x_coord position
    if(it == SelectedSubs.Subs.end()) return;

    setSelectedRange(it->Time);
    updateView(::Selection);

    // TODO: Add Karaoke
    */

}

void WaveformView::wheelEvent(QWheelEvent *ev)
{
    Q_UNUSED(ev)
    /*QPoint angleDelta = ev->angleDelta();

    if(angleDelta.isNull()) return;

    if(ev->modifiers() & Qt::ControlModifier) // Horizontal Zoom
    {
       int NewPageSize;
       if(angleDelta.y() > 0)
       {
           NewPageSize = std::round(PageSizeMs * 80 / 100);
       }
       else
       {
           NewPageSize = std::round(PageSizeMs * 125/100);
       }
       int MouseCursorPosMs = pixelToTime(ev->pos().x()) + PositionMs;
       int NewMouseCursorPosMs = std::round(ev->pos().x() * (NewPageSize / width()));

       zoomRange(MouseCursorPosMs - NewMouseCursorPosMs, MouseCursorPosMs + (NewPageSize - NewMouseCursorPosMs));
    }
    else if(ev->modifiers() & Qt::ShiftModifier) // Vertical Zoom
    {
        if(angleDelta.y() > 0)
        {
            if(VerticalScaling >= 0 && VerticalScaling <= 395) VerticalScaling += 5;
        }
        else
        {
            if(VerticalScaling >= 5 && VerticalScaling <= 400) VerticalScaling -= 5;
        }
    }
    else // Time scrolling
    {
        volatile int ScrollAmount = std::round(PageSizeMs / 4);
        if(ScrollAmount == 0) ScrollAmount = 1;
        if(angleDelta.y() > 0)
        {
            setPositionMs(PositionMs - ScrollAmount);
        }
        else
        {
            setPositionMs(PositionMs + ScrollAmount);
        }
    }
    updateView(PageSize);*/
}

void WaveformView::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev)
    /*
    // Let double clicks be handled only by mouseDoubleClickEvent
    // And do nothing in cases when the waveform is not loaded
    if(ev->flags() & Qt::MouseEventCreatedDoubleClick || LengthMs == 0) return;

    // Ignore clicks on ruler
    if(ev->pos().y() >= height() - DisplayRulerHeight) return;

    MouseIsDown = true;

    int UpdateFlags = 0;

    RangeList &RL = getRangeListFromPos(ev->pos().y());

    mousePressCoolEdit(ev, UpdateFlags, RL);

    // TODO: Add Middle button stuff

    updateView(UpdateFlags);*/
}

void WaveformView::mouseMoveEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev)
}

void WaveformView::mouseReleaseEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev)
    /*int y_coord = ev->pos().y();

    // Ignore events on the ruler
    if(y_coord >= height() - DisplayRulerHeight) return;

    RangeList &RL = getRangeListFromPos(y_coord);

    MouseIsDown = false;*/
}

void WaveformView::mousePressCoolEdit(QMouseEvent *ev, int &UpdateFlags, RangeList &RL)
{
    Q_UNUSED(ev)
    Q_UNUSED(UpdateFlags)
    Q_UNUSED(RL)
    /*
    if(ev->button() == Qt::LeftButton)
    {

    }
    */
}

void WaveformView::zoomRange(Range &range)
{

    if(range.StartTime > range.EndTime) return;
    int NewPageSize = range.EndTime - range.StartTime;
    int NewPosition;
    if(NewPageSize < 100)
    {
        NewPageSize = 100;
        NewPosition = range.StartTime + ((range.EndTime - range.StartTime - NewPageSize) / 2);
    }
    else
    {
        if(NewPageSize < 0) NewPageSize = 0;
        if(NewPageSize > LengthMs) NewPageSize = LengthMs;
        NewPosition = range.StartTime;
    }
    if(NewPageSize < 0) NewPageSize = 0;
    if(NewPageSize > LengthMs - PageSizeMs) NewPageSize = LengthMs - PageSizeMs;
    int UpdateFlags = 0;
    if(NewPageSize != PageSizeMs) UpdateFlags |= PageSize;
    if(NewPosition != PositionMs) UpdateFlags |= Position;
    if(UpdateFlags) updateView(UpdateFlags);

}

void WaveformView::zoomRange(int Start, int End)
{
    Range R;
    R.StartTime = Start;
    R.EndTime = End;
    zoomRange(R);
}

void WaveformView::zoomCenteredOn(int Center, int PageSize)
{
    Range R;
    R.StartTime = Center - (PageSize / 2);
    R.EndTime = Center + (PageSize / 2);
    zoomRange(R);
}

void WaveformView::zoomAll()
{
    Range R;
    R.StartTime = 0;
    R.EndTime = LengthMs;
    zoomRange(R);
}

void WaveformView::zoomIn()
{
    Range R;
    R.StartTime = PositionMs + std::round(PageSizeMs / 3);
    R.EndTime  = PositionMs + std::round((PageSizeMs / 3) * 2);
    zoomRange(R);
}

void WaveformView::zoomOut()
{
    Range R;
    R.StartTime = PositionMs - PageSizeMs;
    R.EndTime = PositionMs + PageSizeMs * 2;
    zoomRange(R);
}

void WaveformView::scrollContentsBy(int, int)
{
    volatile int NewPositionMs = horizontalScrollBar()->value();
    if(NewPositionMs + PageSizeMs - 1 > LengthMs)
    {
        PositionMs = LengthMs - PageSizeMs;
    }
    else
    {
        PositionMs = NewPositionMs;
    }
    updateView(Position);
}
