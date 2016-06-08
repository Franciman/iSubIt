#include "waveformview.h"
#include <QPainter>
#include <QTextOption>

#include "timemstoshortstring.h"

#include "constrain.h"

#include "subtitlemodel.h"

const QColor WavColor = QColor(0x00a7f24a);
const QColor WavBackColor = QColor(Qt::black);
const QColor ZeroLineColor = QColor(0x00518b0a);
const QColor RangeColor1 = QColor(0x003333ff);
const QColor RangeColor2 = QColor(0x00ff8000);
const QColor RangeColorNotEditable = QColor(0x00c0c0c0);
const QColor RulerBackColor = QColor(0x00514741);
const QColor RulerTopBottomLineColor = QColor(0x00beb5ae);
const QColor RulerTextColor = QColor(0x00e0e0e0);
const QColor RulerTextShadowColor = QColor(Qt::black);
const QColor CursorColor = QColor(Qt::yellow);

void WaveformView::updateView(int flags)
{
    if(flags & PageSize)
    {
        OffscreenWav = QPixmap(Viewport->size());
        Offscreen = QPixmap(Viewport->size());
        QPainter painter(&OffscreenWav);
        paintWav(painter, false);
        paintRuler(painter);
    }
    else if(flags & Position)
    {
       QPainter painter(&OffscreenWav);
       paintWav(painter, true);
       OldPositionMs = PositionMs;
       OldPageSizeMs = PageSizeMs;
       paintRuler(painter);
    }
    QPainter painter(&Offscreen);
    painter.drawPixmap(0, 0, OffscreenWav);
    int Height = Viewport->height() - DisplayRulerHeight;
    if(DisplayRangeLists.size() != 0)
    {
        int SubHeight = Height / DisplayRangeLists.size();

        // paintSceneChange

        unsigned int i = 0;
        for(auto subsList = DisplayRangeLists.cbegin(); subsList != DisplayRangeLists.cend(); ++subsList, ++i)
        {
            paintRanges(painter, **subsList, SubHeight * i, SubHeight * (i+1), i == 0 /* draw topLine? */, i == DisplayRangeLists.size() - 1 /* draw bottomLine ? */);
        }

        paintMinimumBlank(painter, 0, Height);
    }

    paintSelection(painter);
    paintCursor(painter);
    paintPlayCursor(painter);

    Viewport->setOffscreen(&Offscreen);
    Viewport->update();
}

void WaveformView::paintWav(QPainter &painter, bool TryOptimize)
{
    if(LengthMs == 0 || PeakList.empty())
    {
        painter.fillRect(painter.window(), WavBackColor);
    }
    else
    {
        painter.setPen(WavColor);

        QRect Rect = Viewport->rect();

        int Width = timeToPixel(PageSizeMs);

        double PeaksPerPixelScaled = (((PageSizeMs / 1000.0) * PeakList.sampleRate()) / PeakList.samplesPerPeak()) / Width;
        double StartPositionInPeaks = ((PositionMs / 1000.0) * PeakList.sampleRate()) / PeakList.samplesPerPeak();

        int x1_update = 0;
        int x2_update = Width;

        TryOptimize = false;
        if(TryOptimize && PageSizeMs == OldPageSizeMs)
        {
            int x_optim;
            if(PositionMs > OldPositionMs)
            {
                x_optim = timeToPixel(PositionMs - OldPositionMs);
                x2_update = Width;
                x1_update = x2_update - x_optim;
                Constrain(x1_update, 0, Width);
                if(x1_update != 0)
                {
                    painter.drawPixmap(0, 0, OffscreenWav, x_optim, 0, x1_update, OffscreenWav.height() - DisplayRulerHeight);
                }
            }
            else
            {
                x_optim = timeToPixel(OldPositionMs - PositionMs);
                x1_update = 0;
                x2_update = x_optim;
                Constrain(x2_update, 0, Width);
                if(x2_update != Width)
                {
                    painter.drawPixmap(x_optim, 0, OffscreenWav, 0, 0, Width - x_optim, OffscreenWav.height() - DisplayRulerHeight);
                }
            }
        }

        Rect.setLeft(x1_update);
        Rect.setRight(x2_update);
        Rect.setBottom(Rect.bottom() - DisplayRulerHeight);

        painter.fillRect(Rect, WavBackColor);

        int Middle = Rect.top() + Rect.height() / 2;

        int PeakMax, PeakMin;
        volatile unsigned int x_scaled;
        volatile unsigned int next_x_scaled;
        for(unsigned int x = x1_update; x < x2_update; x++)
        {
            x_scaled = std::round(x * PeaksPerPixelScaled + StartPositionInPeaks);
            if(x_scaled >= PeakList.peaksNumber()) x_scaled = PeakList.peaksNumber() - 1;
            PeakMax = PeakList[x_scaled].max();
            PeakMin = PeakList[x_scaled].min();

            next_x_scaled = std::min( std::round((x + 1) * PeaksPerPixelScaled + StartPositionInPeaks), (double)PeakList.peaksNumber() );
            for(unsigned int i = x_scaled + 1; i < next_x_scaled; i++)
            {
                if(PeakList[i].max() > PeakMax) PeakMax = PeakList[i].max();
                if(PeakList[i].min() < PeakMin) PeakMin = PeakList[i].min();
            }

            int y1 = std::round((((PeakMax * VerticalScaling) / 100.0) * Rect.height()) / 65536);
            int y2 = std::round((((PeakMin * VerticalScaling) / 100.0) * Rect.height()) / 65536);

            painter.drawLine(QPoint(x, Middle - y1), QPoint(x, Middle - y2));

        }

        painter.setPen(ZeroLineColor);
        painter.drawLine(QPoint(0, Middle), QPoint(Width, Middle));
    }
}

void WaveformView::paintRuler(QPainter &painter)
{
    if(DisplayRulerHeight > 0)
    {
        QRect Rect = painter.window();
        Rect.setTop(Rect.bottom() - DisplayRulerHeight);

        // Draw background
        painter.fillRect(Rect,RulerBackColor);

        // Draw horizontal line at top and bottom
        painter.setPen(RulerTopBottomLineColor);
        painter.drawLine(QPoint(0, Rect.top()), QPoint(width(), Rect.top()));
        painter.drawLine(QPoint(0, Rect.bottom() - 1), QPoint(width(), Rect.bottom() - 1));

        // Set the text font
        QFont font("Time New Roman", 8);
        painter.setFont(font);
        painter.setPen(RulerTextColor);
        QFontMetrics metrics(painter.font());

        int Width = timeToPixel(PageSizeMs);

        // Do some little calculation to try to show "round" time
        int MaxPosStep = std::round(double(Width) / (metrics.width("0:00:00.0") * 2.0));
        int StepMs = std::round(double(PageSizeMs) / MaxPosStep);
        if(StepMs == 0) StepMs = 1;

        int StepLog = std::trunc(std::pow(10, std::trunc(std::log10(StepMs))));
        StepMs = double(StepMs) / StepLog * (StepLog);

        int p = PositionMs; // / (StepMs * StepMs);
        int x;
        int x1, x2;
        int height, width;
        QString PosString;
        while(p < PositionMs + PageSizeMs)
        {
            // Draw main division
            x = timeToPixel(p - PositionMs);
            painter.drawLine(QPoint(x, Rect.top() + 1), QPoint(x, Rect.top() + 5));

            PosString = timeMsToShortString(p, StepLog);

            // Calculate text coordinates
            x1 = x - metrics.width(PosString) / 2;
            height = metrics.height();
            width = metrics.width(PosString);

            /*if(TimeStamps.contains(PosString))
            {
                // Draw text shadow
                painter.setPen(RulerTextShadowColor);
                painter.drawStaticText(x1 + 2, Rect.top() + 4, *TimeStamps.object(PosString));
                painter.setPen(RulerTextColor);
                painter.drawStaticText(x1, Rect.top() + 4, *TimeStamps.object(PosString));
            }
            else
            {
                QStaticText *timestamp = new QStaticText(PosString);
                TimeStamps.insert(PosString, timestamp);
                // Draw text shadow
                painter.setPen(RulerTextShadowColor);
                painter.drawStaticText(x1 + 2, Rect.top() + 4, *timestamp);

                // Draw text
                painter.setPen(RulerTextColor);
                painter.drawStaticText(x1, Rect.top() + 4, *timestamp);
            }*/
            painter.setPen(RulerTextShadowColor);
            painter.drawText(QRect(x1 + 2, Rect.top() + 4 + 2, width, height), PosString);
            painter.setPen(RulerTextColor);
            painter.drawText(QRect(x1, Rect.top() + 4, width, height), PosString);
            // Draw subdivision
            x2 = x + timeToPixel(StepMs / 2);
            painter.drawLine(QPoint(x2, Rect.top() + 1), QPoint(x2, Rect.top() + 3));
            p += StepMs;
        }
    }
}

void WaveformView::paintMinimumBlank(QPainter &painter, int rangeTop, int rangeBottom)
{
    if(ShowMinimumBlank)
    {
        if(Info1.exists())
        {
            drawAlphaRect(painter, Info1.getStart(MinimumBlank), Info1.getStop(MinimumBlank), rangeTop, rangeBottom);
        }

        if(Info2.exists())
        {
            drawAlphaRect(painter, Info2.getStart(MinimumBlank), Info2.getStop(MinimumBlank), rangeTop, rangeBottom);
        }
    }
}

void WaveformView::paintRanges(QPainter &painter, const RangeList &Subs, int topPos, int bottomPos, bool topLine, bool bottomLine)
{
    int RangeHeightDiv10 = (bottomPos - topPos) / 10;
    int y1 = topPos + RangeHeightDiv10;
    int y2 = bottomPos - RangeHeightDiv10;

    for(unsigned int i = 0; i < Subs.Subtitles.size(); ++i)
    {
        const Range &r = Subs.Subtitles[i].Time;
        int x1 = -1;
        int x2 = -1;
        if(r.StartTime >= PositionMs && r.StartTime < PositionMs + PageSizeMs)
        {
            x1 = timeToPixel(r.StartTime - PositionMs);
        }
        if(r.EndTime >= PositionMs && r.EndTime < PositionMs + PageSizeMs)
        {
            x2 = timeToPixel(r.EndTime - PositionMs);
        }

        bool ShowStart = x1 != -1;
        bool ShowEnd = x2 != -1 && x2 != x1;
        bool FullHLines = r.StartTime < PositionMs && r.EndTime >= PositionMs + PageSizeMs;
        if(Subs.Editable)
        {
            if(ShowStart || ShowEnd || FullHLines)
            {
                if(!(i % 2))
                {
                   painter.setPen(RangeColor1);
                }
                else
                {
                    painter.setPen(RangeColor2);
                }
            }
        }
        else
        {
            painter.setPen(RangeColorNotEditable);
        }


        if(ShowStart)
        {
            painter.drawLine(QPoint(x1, topPos), QPoint(x1, bottomPos));
        }

        if(ShowEnd)
        {
            painter.drawLine(QPoint(x2, topPos), QPoint(x2, bottomPos));
        }

        if(FullHLines)
        {
           x1 = 0;
           x2 = width() - 1;
        }

        if(x1 != -1 || x2 != -1)
        {
            if(x1 == -1) x1 = 0;
            else if(x2 == -1) x2 = width() - 1;

            //painter.pen().setWidth(2);
            if(topLine)
            {
                painter.drawLine(QPoint(x1, y1), QPoint(x2, y1));
            }
            if(bottomLine)
            {
                painter.drawLine(QPoint(x1, y2), QPoint(x2, y2));
            }
            //painter.pen().setWidth(1);
        }

       if(ShowTextInRanges && x2-x1 > 10)
       {
           const int TextMargins = 5, MinSpace = 25;
           QRect CustomDrawRect(QPoint(x1 + TextMargins, y1), QPoint(x2 - TextMargins, y2));
           if(CustomDrawRect.width() > MinSpace)
           {
               QTextOption Opt = topLine ? QTextOption() : QTextOption(Qt::AlignBottom);
               painter.drawText(CustomDrawRect, Subs.Subtitles[i].Text, Opt);
           }
       }

       // TODO: Add karaoke

       // TODO: Add markzone

       // TODO: Add clocksynchro
    }
}

void WaveformView::paintSelection(QPainter &painter)
{
    if(TheSelection.StartTime > 0)
    {
        int SelectionStartTime = TheSelection.StartTime;
        int SelectionStopTime = TheSelection.EndTime;
        int x1 = timeToPixel(SelectionStartTime - PositionMs);
        int x2 = timeToPixel(SelectionStopTime - PositionMs);
        if(x1 == x2)
        {
            if(SelectionStartTime >= PositionMs && SelectionStopTime <= PositionMs + PageSizeMs)
            {
                painter.drawLine(QPoint(x1, 0), QPoint(x2, height() - DisplayRulerHeight));
            }
        }
        else
        {
            if(x1 < 0) x1 = 0;
            if(x2 >= width()) x2 = width() - 1;
            QRect SelRect = Viewport->rect();
            SelRect.setLeft(x1);
            SelRect.setRight(x2+1);
            SelRect.setBottom(SelRect.bottom() - DisplayRulerHeight);
            // InvertRect
            painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
            painter.fillRect(SelRect, Qt::white);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
    }
}

void WaveformView::paintCursor(QPainter &painter)
{
   int CanvasHeight = Viewport->height() - DisplayRulerHeight;
   if(CursorMs >= PositionMs && CursorMs <= PositionMs + PageSizeMs)
   {
       int pos = timeToPixel(CursorMs - PositionMs);
       painter.setPen(QPen(CursorColor, 1, Qt::DotLine));
       painter.setCompositionMode(QPainter::RasterOp_NotSourceXorDestination);
       painter.drawLine(QPoint(pos, 0), QPoint(pos, CanvasHeight));
       painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
   }
}

void WaveformView::paintPlayCursor(QPainter &painter)
{
    if(IsPlaying)
    {
        if(PlayCursorMs >= PositionMs && PlayCursorMs <= PositionMs + PageSizeMs)
        {
            painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
            painter.setPen(Qt::white);
            int x = timeToPixel(PlayCursorMs - PositionMs);
            painter.drawLine(QPoint(x, 0), QPoint(x, painter.window().height() - DisplayRulerHeight));
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
    }
}

void WaveformView::drawAlphaRect(QPainter &painter, int t1, int t2, int y1, int y2)
{
    int x1 = timeToPixel(t1 - PositionMs);
    int x2 = timeToPixel(t2 - PositionMs);

    int Width = timeToPixel(PageSizeMs);
    Constrain(x1, 0, Width);
    Constrain(x2, 0, Width);

    QRect Rect;
    Rect.setLeft(x1);
    Rect.setRight(x2);
    Rect.setTop(y1);
    Rect.setBottom(y2);

    painter.fillRect(Rect, QColor(255, 255, 255, 80));
}

/*void WaveformView::paintEvent(QPaintEvent *ev)
{
    if(OffscreenWav.size() != Viewport->size())
    {
        updateView(PageSize);
    }
    QAbstractScrollArea::paintEvent(ev);
}*/
