#ifndef FRAMESPROCESSOR_H
#define FRAMESPROCESSOR_H

#include "peaks.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "mediafile.h"
#include "sampleextractor.h"
#include "scenechangeextractor.h"

#include <QObject>


struct FramesProcessor : public QObject
{
    Q_OBJECT
public:
    template<class SampleFormat, bool Planar>
    void processFrames(MediaFile &media, AVCodecContext *AudioCodecCtx, AVCodecContext *VideoCodecCtx, AVStream *audioStream, AVStream *videoStream, Peaks &PeakList);

Q_SIGNALS:
    void progress(int);
};

template<class SampleFormat, bool Planar>
void FramesProcessor::processFrames(MediaFile &media, AVCodecContext *AudioCodecCtx, AVCodecContext *VideoCodecCtx, AVStream *audioStream, AVStream *videoStream, Peaks &PeakList)
{
   AVPacket pkt;
   AVFrame *frame = av_frame_alloc();

   if(!frame)
       throw FFmpegError("Could not allocate memory for frame");

   av_init_packet(&pkt);
   pkt.data = nullptr;
   pkt.size = 0;

   int audio_stream_index = audioStream->index;
   int video_stream_index = videoStream->index;

   PeaksExtractor<SampleFormat, Planar> PExtractor(AudioCodecCtx, PeakList);
   //SceneChangeExtractor SCExtractor(videoStream, VideoCodecCtx, SceneChanges);

   double time_base = static_cast<double>(AudioCodecCtx->time_base.num) / AudioCodecCtx->time_base.den;

   while(media.getNextPacket(pkt))
   {
      AVPacket orig_pkt = pkt;
      if(pkt.stream_index == audio_stream_index)
      {
          PExtractor(pkt);
      }
      else if(pkt.stream_index == video_stream_index)
      {
          //SCExtractor(pkt);
      }
      emit progress(((orig_pkt.pts * time_base) * 100) / media.duration_in_seconds());
      av_packet_unref(&orig_pkt);
   }

   PExtractor.flush();
   //SCExtractor.flush();
   PExtractor.normalizePeaks();
}


#endif // FRAMESPROCESSOR_H
