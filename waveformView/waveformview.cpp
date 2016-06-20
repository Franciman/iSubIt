#include "waveformview.h"

#include <QResizeEvent>
#include <QPainter>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QThread>
#include <cmath>

#include "subtitlemodel.h"

#include "mediaProcessor/mediafile.h"
#include "mediaProcessor/mediaprocessor.h"

#include "constrain.h"

RangeList &WaveformView::getRangeListFromPos(int y)
{
    int SubHeight = (height() - DisplayRulerHeight) / DisplayRangeLists.size();
    int idx = std::floor(y / SubHeight);
    Constrain(idx, 0, (int)DisplayRangeLists.size() - 1);
    return *DisplayRangeLists[DisplayRangeLists.size() - idx - 1];
}

/*void WaveformViewport::setSelectedRange(Range &subRange)
{
    Selection = subRange;
}*/

WaveformView::WaveformView(QWidget *parent) :
        QAbstractScrollArea(parent)
{
    Viewport = new WaveformView::ViewportWidget(this);
    Viewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(Viewport);
    QWidget *scrollWidget = new QWidget(this);
    scrollWidget->setLayout(layout);
    setViewport(scrollWidget);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    horizontalScrollBar()->setRange(0, LengthMs);
    horizontalScrollBar()->setSingleStep(50);
    horizontalScrollBar()->setValue(0);

    OffscreenWav = QPixmap(Viewport->size());
    Offscreen2 = QPixmap(Viewport->size());
    Offscreen = QPixmap(Viewport->size());

    OldPositionMs = PositionMs = 0;
    OldPageSizeMs = PageSizeMs = 0;
    LengthMs = 0;
    CursorMs = 0;
    OldPlayCursorMs = PlayCursorMs = 0;
    VerticalScaling = 100;
    DisplayRulerHeight = 20;

    ShowTextInRanges = true;
    IsPlaying = false;
    AutoScrolling = true;
    MouseIsDown = false;

    DEMode = DynamicEditMode::None;

    SnappingEnabled = true;
    EnableMouseAntiOverlapping = true;

    setMouseTracking(true);
    Viewport->setMouseTracking(true);
}

void WaveformView::updatePeaks(Peaks &&P)
{
    PeakList = std::move(P);
    LengthMs = 1000 * ((PeakList.peaksNumber() * PeakList.samplesPerPeak()) / PeakList.sampleRate());
    PageSizeMs = 8000;
    //zoomRange(0, 15000);
    horizontalScrollBar()->setRange(0, LengthMs);
    horizontalScrollBar()->setPageStep(PageSizeMs);
    updateView(PageSize);
}

void WaveformView::setPositionMs(int pos)
    {
        Constrain(pos, 0, LengthMs - PageSizeMs);
        if(pos != PositionMs)
        {
            PositionMs = pos;
            horizontalScrollBar()->setValue(PositionMs);
            updateView(Position);
        }
    }

void WaveformView::changeModel(const QString &, SubtitleModel *model)
{
    DisplayRangeLists.clear();
    if(model->hasVOSubtitles())
    {
        DisplayRangeLists.push_back(model->getVOSubtitles());
    }
    DisplayRangeLists.push_back(&model->getSubtitles());
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(changeSubs(QModelIndex,QModelIndex,QVector<int>)));
}

void WaveformView::changeSubs(QModelIndex, QModelIndex, QVector<int>)
{

}

void WaveformView::changePlayCursorPos(int Time)
{
    Constrain(Time, 0, LengthMs);
    if(PlayCursorMs != Time)
    {
        OldPlayCursorMs = PlayCursorMs;
        PlayCursorMs = Time;
        if(AutoScrolling && !MouseIsDown && (Time < PositionMs || Time >= PositionMs + PageSizeMs))
        {
            setPositionMs(Time - (PageSizeMs / 8));
        }
        else
        {
            updateView(PlayCursor);
        }
    }
}

bool WaveformView::viewportEvent(QEvent *ev)
{
    switch(ev->type())
    {
    case QEvent::Resize:
        OffscreenWav = QPixmap(Viewport->size());
        Offscreen2 = QPixmap(Viewport->size());
        Offscreen = QPixmap(Viewport->size());
        updateView(PageSize);
        return true;
    case QEvent::MouseButtonDblClick:
        mouseDoubleClick((QMouseEvent*)ev);
        return true;
    case QEvent::MouseButtonPress:
        mousePress((QMouseEvent*)ev);
        return true;
    case QEvent::MouseMove:
        mouseMove((QMouseEvent*)ev);
        return true;
    case QEvent::MouseButtonRelease:
        mouseRelease((QMouseEvent*)ev);
        return true;
    case QEvent::Wheel:
        wheel((QWheelEvent*)ev);
        return true;
    case QEvent::ContextMenu:
        showContextMenu((QContextMenuEvent*)ev);
        return true;
    default:
        return false;
    }
}

int WaveformView::findSnappingPos(int PosMs, RangeList &RL)
{
    const int SNAPPING_DISTANCE_PIXEL = 8;
    int Result = -1, Candidate = -1;
    // int Idx, IdxForward, IdxBackward, DistForward, DistForward;
    int SnappingDistance = pixelToTime(SNAPPING_DISTANCE_PIXEL);

    if(MinimumBlank > 0)
    {
        if(Info1.exists())
        {
            Candidate = Info1.getSnappingPoint(MinimumBlank);
            if(std::abs(Candidate - PosMs) <= SnappingDistance)
            {
                return Candidate;
            }
        }

        if(Info2.exists())
        {
            Candidate = Info2.getSnappingPoint(MinimumBlank);
            if(std::abs(Candidate - PosMs) <= SnappingDistance)
            {
                return Candidate;
            }
        }

        // TODO: Add SceneChange stuff
        return Result;
    }
}

int WaveformView::findCorrectedSnappingPos(int Pos, RangeList &RL)
{
    int Result = findSnappingPos(Pos, RL);
    if(Result != -1 && EnableMouseAntiOverlapping)
    {
        // Do a correction here
        if(MinSelTime != -1 && Result < MinSelTime)
        {
            Result = MinSelTime;
        }
        if(MaxSelTime != -1 && Result > MaxSelTime)
        {
            Result = MaxSelTime;
        }
    }
    return Result;
}

void WaveformView::setSelectedRange(Range *subRange, bool UpdateDisplay)
{
    if(!subRange)
    {
        TheSelection.StartTime = TheSelection.EndTime = 0;
    }
    if(SelectedRange != subRange)
    {
        SelectedRange = subRange;
        if(SelectedRange)
        {
            TheSelection.StartTime = SelectedRange->StartTime;
            TheSelection.EndTime = SelectedRange->EndTime;
            //setMinBlankAt(SelectedRange->StartTime, );
        }

        if(UpdateDisplay)
        {
            updateView(Selection | TheRange);
        }
        emit selectionChanged();
    }
}

bool WaveformView::setMinBlankOnIdx(int idx, RangeList &RL)
{
    bool ChangedInStep;
    bool Result = false;

    if(idx > 0)
    {
        ChangedInStep = Info1.setInfo(&RL.Subtitles[idx - 1].Time, MinBlankInfoPart::End);
    }
    else
    {
        ChangedInStep = Info1.setInfo(nullptr, MinBlankInfoPart::Invalid);
    }
    Result |= ChangedInStep;

    if(idx < RL.Subtitles.size() - 1)
    {
        ChangedInStep = Info2.setInfo(&RL.Subtitles[idx + 1].Time, MinBlankInfoPart::Start);
    }
    else
    {
        ChangedInStep = Info2.setInfo(nullptr, MinBlankInfoPart::Invalid);
    }
    Result |= ChangedInStep;
    return Result;
}

bool WaveformView::setMinBlankAt(int TimeMs, RangeList &RL)
{
    int idx;
    bool ChangedInStep;
    bool Result = false;

    if(!RL.Editable)
    {
        Result = Info1.setInfo(nullptr, MinBlankInfoPart::Invalid);
        Result |= Info2.setInfo(nullptr, MinBlankInfoPart::Invalid);
        return Result;
    }

    idx = RL.getRangeIdxAt(TimeMs);
    if(idx != -1)
    {
        ChangedInStep = setMinBlankOnIdx(idx, RL);
        Result |= ChangedInStep;
    }
    else
    {
        idx = RL.findInsertPos(TimeMs, -1);
        if(idx > 0)
        {
            ChangedInStep = Info1.setInfo(&RL.Subtitles[idx - 1].Time, MinBlankInfoPart::End);
        }
        else
        {
            ChangedInStep = Info1.setInfo(nullptr, MinBlankInfoPart::Invalid);
        }
        Result |= ChangedInStep;

        if(idx < RL.Subtitles.size())
        {
            ChangedInStep = Info2.setInfo(&RL.Subtitles[idx].Time, MinBlankInfoPart::Start);
        }
        else
        {
            ChangedInStep = Info2.setInfo(nullptr, MinBlankInfoPart::Invalid);
        }
        Result |= ChangedInStep;
    }
    return Result;
}

bool WaveformView::checkSubtitleForDynamicSelection(Range *R, int CursorPosMs, int RangeSelWindow, const QPoint &mousePos, RangeList &RL)
{
    DynamicEditMode NewDEMode = DynamicEditMode::None;
    int i;
    int Height = Viewport->height() - DisplayRulerHeight;
    int y1, y2;

    bool Result = false;
    if((R->EndTime - R->StartTime) / RangeSelWindow > 2)
    {
        if(mousePos.y() > 0 && mousePos.y() < Height)
        {
            if(std::abs(CursorPosMs - R->StartTime) < RangeSelWindow)
            {
                NewDEMode = DynamicEditMode::Start;
                DynamicEditTime = R->StartTime;
            }
            else if(std::abs(CursorPosMs - R->EndTime) < RangeSelWindow)
            {
                NewDEMode = DynamicEditMode::End;
                DynamicEditTime = R->EndTime;
            }
            else
            {
                // TODO: Add karaoke check
            }
        }
    }

    if(NewDEMode != DynamicEditMode::None)
    {
        Result = true;
        setCursor(QCursor(Qt::SizeHorCursor));
        DEMode = NewDEMode;
        DynamicSelRangeOld = DynamicSelRange;
        if(*R != TheSelection) DynamicSelRange = R;
        else if(SelectedRange) DynamicSelRange = SelectedRange;
        else DynamicSelRange = nullptr;
    }

    if(DynamicSelRange != DynamicSelRangeOld)
    {
        if(DynamicSelRange && DEMode != DynamicEditMode::None)
        {
            setMinBlankAt(DynamicSelRange->StartTime, RL);
        }
        updateView(TheRange);
    }
    return Result;
}

void WaveformView::ViewportWidget::paintEvent(QPaintEvent *)
{
    if(Offscreen)
    {
        QPainter painter(this);
        painter.drawPixmap(0, 0, *Offscreen);
    }
}
