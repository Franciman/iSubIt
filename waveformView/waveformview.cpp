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

/*RangeList &WaveformViewport::getRangeListFromPos(int y)
{
    int SubHeight = (height() - DisplayRulerHeight) / Subs->rowCount();
    int idx = std::floor(y / SubHeight);
    return TheModel.Subtitles[idx];
}*/

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

    OffscreenWav = QImage(Viewport->size(), QImage::Format_ARGB32);
    Offscreen = QImage(Viewport->size(), QImage::Format_ARGB32);

    OldPositionMs = PositionMs = 0;
    OldPageSizeMs = PageSizeMs = 0;
    LengthMs = 0;
    CursorMs = 0;
    VerticalScaling = 100;
    DisplayRulerHeight = 20;

    ShowTextInRanges = true;
}

void WaveformView::updatePeaks(Peaks &&P)
{
    PeakList = std::move(P);
    LengthMs = 1000 * ((PeakList.peaksNumber() * PeakList.samplesPerPeak()) / PeakList.sampleRate());
    PageSizeMs = 8000;
    horizontalScrollBar()->setRange(0, LengthMs);
    updateView(PageSize);
}

void WaveformView::changeModel(const QString &, SubtitleModel *model)
{
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

bool WaveformView::viewportEvent(QEvent *ev)
{
    if(ev->type() == QEvent::Resize)
    {
        OffscreenWav = QImage(Viewport->size(), QImage::Format_ARGB32);
        Offscreen = QImage(Viewport->size(), QImage::Format_ARGB32);
        updateView(PageSize);
        return true;
    }
    return false;
}

void WaveformView::ViewportWidget::paintEvent(QPaintEvent *)
{
    if(Offscreen)
    {
        QPainter painter(this);
        painter.drawImage(0, 0, *Offscreen);
    }
}
