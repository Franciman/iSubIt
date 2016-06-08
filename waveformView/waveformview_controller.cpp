#include "waveformview.h"
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMenu>

#include <iostream>
#include <algorithm>


#include "constrain.h"

#include "subtitlemodel.h"

void WaveformView::mouseDoubleClick(QMouseEvent *ev)
{
    int y_coord = ev->pos().y();
    int x_coord = ev->pos().x();

    int x_ms = pixelToTime(x_coord) + PositionMs;

    // Ignore double clicks on the ruler
    if(y_coord >= height() - DisplayRulerHeight) return;
    if(DisplayRangeLists.size() == 0) return;

    RangeList &SelectedSubs = getRangeListFromPos(y_coord);

    // If these subtitles can't be edited, they can't be selected
    if(!SelectedSubs.Editable) return;

    // TODO: Add Dynamic edit mode
    if(DynamicSelRange)
    {
        switch(DEMode)
        {
        case DynamicEditMode::Start:
            emit rangeStartDoubleClick(DynamicSelRange);
            return;
        case DynamicEditMode::End:
            emit rangeStopDoubleClick(DynamicSelRange);
            return;
        }
    }

    int idx = SelectedSubs.getRangeIdxAt(CursorMs);
    if(idx == -1) return;

    setSelectedRange(&SelectedSubs.Subtitles[idx].Time, false);
    updateView(Selection);

    emit selectedRange(SelectedRange, false);

    // TODO: Add Karaoke

}

void WaveformView::wheel(QWheelEvent *ev)
{
    QPoint angleDelta = ev->angleDelta();

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
    updateView(PageSize);
}

void WaveformView::mousePress(QMouseEvent *ev)
{
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

    updateView(UpdateFlags);
}

void WaveformView::mouseMove(QMouseEvent *ev)
{
    QPoint mousePos = ev->pos();
    // Manual clipping implementation
    if(Clipping)
    {
        if(mousePos.x() < ClippingRect.left())
        {
            mousePos.setX(ClippingRect.left());
            QCursor::setPos(mapToGlobal(mousePos));
        }
        else if(mousePos.x() > ClippingRect.right())
        {
            mousePos.setX(ClippingRect.right());
            QCursor::setPos(mapToGlobal(mousePos));
        }
    }

    // Let double clicks be handled only by mouseDoubleClickEvent
    // And do nothing in cases when the waveform is not loaded
    if(ev->flags() & Qt::MouseEventCreatedDoubleClick || LengthMs == 0) return;

    int UpdateFlags = 0;
    int NewCursorPos;
    RangeList &RL = getRangeListFromPos(mousePos.y());
    int Width = timeToPixel(PageSizeMs);
    int X = mousePos.x();

    if(MouseIsDown)
    {
        if(ev->buttons() == Qt::LeftButton)
        {
            Constrain(X, 0, Width);
            NewCursorPos = pixelToTime(X) + PositionMs;

            // Make sure to clip selection
            if(MinSelTime != -1)
            {
                Constrain(NewCursorPos, MinSelTime, std::numeric_limits<int>::max());
            }
            if(MaxSelTime != -1)
            {
                Constrain(NewCursorPos, 0, MaxSelTime);
            }

            // Snapping
            if(!(ev->modifiers() & Qt::ControlModifier) && SnappingEnabled)
            {
                int SnappingPos = findCorrectedSnappingPos(NewCursorPos, RL);
                if(SnappingPos != -1)
                {
                    NewCursorPos = SnappingPos;
                }
            }

            if(SelectionOrigin != -1 && SelectionOrigin != NewCursorPos)
            {
                //Update selection
                if(NewCursorPos > SelectionOrigin)
                {
                    TheSelection.StartTime = SelectionOrigin;
                    TheSelection.EndTime = NewCursorPos;
                }
                else
                {
                    TheSelection.StartTime = NewCursorPos;
                    TheSelection.EndTime = SelectionOrigin;
                }
                UpdateFlags |= Selection;
                if(SelectedRange)
                {
                    NeedSortSelectedSub = true;
                    if(*SelectedRange != TheSelection)
                    {
                        SelectedRange->StartTime = TheSelection.StartTime;
                        SelectedRange->EndTime = TheSelection.EndTime;
                        UpdateFlags |= TheRange;
                        emit selectedRangeChange();
                    }
                }
                emit selectionChanged();
            }

            if(CursorMs != NewCursorPos && DEMode == DynamicEditMode::None)
            {
                CursorMs = NewCursorPos;
                UpdateFlags |= Cursor;
                emit cursorChange();
            }
        }

        // TODO: Add Middle button
    }
    else if(RL.Editable)
    {
        Constrain(X, 0, Width);
        int CursorPosMs = pixelToTime(X) + PositionMs;

        // Dynamic selection
        if(ev->modifiers() == 0)
        {
            // Find a subtitle under the mouse
            int RangeSelWindow = pixelToTime(4);
            if(RangeSelWindow < 1) RangeSelWindow = 1;

            // First pass: check only inside sub
            Range *RangeUnder = RL.findFirstRangeAt(CursorPosMs, 0);
            while(RangeUnder)
            {
                if(checkSubtitleForDynamicSelection(RangeUnder, CursorPosMs, RangeSelWindow, mousePos, RL)) return;
                RangeUnder = RL.findNextRange();
            }

            // Second pass: Wider search
            RangeSelWindow = pixelToTime(2);
            if(RangeSelWindow < 1) RangeSelWindow = 1;
            RangeUnder = RL.findFirstRangeAt(CursorPosMs, RangeSelWindow);
            while(RangeUnder)
            {
                if(checkSubtitleForDynamicSelection(RangeUnder, CursorPosMs, RangeSelWindow, mousePos, RL)) return;
                RangeUnder = RL.findNextRange();
            }

            // Check selection
            if(!selectionIsEmpty())
            {
                RangeSelWindow = pixelToTime(4);
                if(RangeSelWindow < 1) RangeSelWindow = 1;
                if(checkSubtitleForDynamicSelection(&TheSelection, CursorPosMs, RangeSelWindow, mousePos, RL))
                {
                    return;
                }
            }
        }

        if(setMinBlankAt(CursorPosMs, RL)) UpdateFlags |= TheRange;
        setCursor(QCursor(Qt::ArrowCursor));
        DEMode = DynamicEditMode::None;
        DynamicSelRangeOld = DynamicSelRange;
        DynamicSelRange = nullptr;
        if(DynamicSelRange != DynamicSelRangeOld) UpdateFlags |= TheRange;
    }
    else
    {
        // TO clear min blank info
        if(setMinBlankAt(-1, RL))
        {
            UpdateFlags |= TheRange;
        }
        DEMode = DynamicEditMode::None;
        DynamicSelRangeOld = DynamicSelRange;
        DynamicSelRange = nullptr;
        if(DynamicSelRangeOld != DynamicSelRange)
        {
            UpdateFlags |= TheRange;
        }
    }
    updateView(UpdateFlags);
}

void WaveformView::mouseRelease(QMouseEvent *ev)
{
    int y_coord = ev->pos().y();

    // Ignore events on the ruler
    if(y_coord >= height() - DisplayRulerHeight) return;

    if(DisplayRangeLists.size() == 0) return;

    RangeList &RL = getRangeListFromPos(y_coord);

    if(TheSelection.duration() < 40 && (!SelectedRange || !DynamicSelRange))
    {
        clearSelection();
    }

    if(NeedSortSelectedSub)
    {
        emit selectedRangeChanged(true);
        RL.fullSort();
        NeedSortSelectedSub = false;
    }

    SelectionOrigin = -1;
    ScrollOrigin = -1;
    OldSelectionStart = OldSelectionEnd = -1;


    MouseIsDown = false;
    DEMode = DynamicEditMode::None;
    DynamicSelRange = nullptr;
    MinSelTime = MaxSelTime = -1;
    Clipping = false;
}

void WaveformView::mousePressCoolEdit(QMouseEvent *ev, int &UpdateFlags, RangeList &RL)
{
    if(ev->button() == Qt::LeftButton)
    {
        // TODO: Add Karaoke

        QPoint MousePos = ev->pos();
        // Readjust mouse cursor position
        if(DEMode == DynamicEditMode::Start || DEMode == DynamicEditMode::End)
        {
            MousePos.setX(timeToPixel(DynamicEditTime - PositionMs));
            QCursor::setPos(mapToGlobal(MousePos));
        }

        int NewCursorPos = pixelToTime(MousePos.x()) + PositionMs;

        // Snapping
        if(SnappingEnabled && !(ev->modifiers() & Qt::ControlModifier))
        {
            int SnappingPos = findCorrectedSnappingPos(NewCursorPos, RL);
            if(SnappingPos != -1)
            {
                NewCursorPos = SnappingPos;
            }
        }

        if(ev->modifiers() & Qt::ShiftModifier || DEMode != DynamicEditMode::None)
        {
            // DynamicSelectionRange
            if(DynamicSelRange && DynamicSelRange != SelectedRange)
            {
               setSelectedRange(DynamicSelRange, false);
               UpdateFlags |= Selection;
               emit selectedRange(SelectedRange, true);
            }

            if(NewCursorPos > TheSelection.StartTime + TheSelection.duration() / 2)
            {
                // We are close to the end of the selection
                if(selectionIsEmpty())
                {
                    if(NewCursorPos > CursorMs)
                    {
                        TheSelection.EndTime = NewCursorPos;
                        TheSelection.StartTime = CursorMs;
                    }
                    else
                    {
                        TheSelection.EndTime = CursorMs;
                        TheSelection.StartTime = NewCursorPos;
                    }
                }
                else
                {
                    TheSelection.EndTime = NewCursorPos;
                }

                SelectionOrigin = TheSelection.StartTime;
            }
            else
            {
                // We are close to the start of the selection
                TheSelection.StartTime = NewCursorPos;
                SelectionOrigin = TheSelection.EndTime;

            }

            if(SelectedRange)
            {
                NeedSortSelectedSub = true;
                OldSelectionStart = SelectedRange->StartTime;
                OldSelectionEnd = SelectedRange->EndTime;
                SelectedRange->StartTime = TheSelection.StartTime;
                SelectedRange->EndTime = TheSelection.EndTime;
                UpdateFlags |= TheRange;
                if(DEMode == DynamicEditMode::None)
                {
                    emit selectedRangeChange();
                }
            }
            emit selectionChanged();
            UpdateFlags |= Selection;
        }
        else
        {
            if(TheSelection.StartTime != TheSelection.EndTime)
            {
                UpdateFlags |= Selection;
            }
            SelectionOrigin = NewCursorPos;
            setSelectedRange(nullptr, false);
        }

        // TODO: Add mouse anti overlapping case
        if(EnableMouseAntiOverlapping)
        {
            int x1 = 0;
            int Width =timeToPixel(PageSizeMs);
            int x2 = Width;

            MaxSelTime = MinSelTime = -1;
            int i = RL.findInsertPos(NewCursorPos, -1);
            if(i >= 0)
            {
                if(SelectedRange)
                {
                    if(NewCursorPos == SelectedRange->StartTime)
                    {
                        if(i > 0)
                        {
                            MinSelTime = RL.Subtitles[i-1].Time.EndTime + 1;
                            x1 = timeToPixel(MaxSelTime - PositionMs);
                        }
                        MaxSelTime = SelectedRange->EndTime - 1;
                        x2 = timeToPixel(MaxSelTime - PositionMs);
                    }
                    else
                    {
                        MinSelTime = SelectedRange->StartTime + 1;
                        x1 = timeToPixel(MinSelTime - PositionMs);
                        if(i < RL.Subtitles.size())
                        {
                            MaxSelTime = RL.Subtitles[i+1].Time.StartTime -1;
                            x2 = timeToPixel(MaxSelTime - PositionMs);
                        }
                    }
                }
                else
                {
                    if(i > 0 && NewCursorPos >= RL.Subtitles[i-1].Time.StartTime && NewCursorPos <= RL.Subtitles[i-1].Time.EndTime)
                    {
                        // Selection only INSIDE subtitle range
                    }
                    else
                    {
                        // Selection only OUTSIDE subtitle range
                        if(i > 0)
                        {
                            MinSelTime = RL.Subtitles[i - 1].Time.EndTime + 1;
                            x1 = timeToPixel(MinSelTime - PositionMs);
                        }
                        if(i < RL.Subtitles.size())
                        {
                            MaxSelTime = RL.Subtitles[i].Time.StartTime - 1;
                            x2 = timeToPixel(MaxSelTime - PositionMs);
                        }
                    }
                }
            }
            Constrain(x1, 0, Width);
            Constrain(x2, 0, Width);
            Clipping = true;
            ClippingRect.setLeft(x1 - 2);
            ClippingRect.setRight(x2 + 3);
            ClippingRect.setTop(0);
            ClippingRect.setBottom(Viewport->height());
        }

        if(NewCursorPos != CursorMs && DEMode == DynamicEditMode::None)
        {
            CursorMs = NewCursorPos;
            emit cursorChange();
            UpdateFlags |= Cursor;
        }
    }
}

void WaveformView::showContextMenu(QContextMenuEvent *ev)
{
    QMenu *Menu = new QMenu(this);
    Menu->addAction("Yo");
    QAction *Res = Menu->exec(ev->pos());
    if(Res)
    {
        std::cout << "Yay" << std::endl;
    }
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
