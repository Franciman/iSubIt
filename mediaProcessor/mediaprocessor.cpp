#include "mediaprocessor.h"

#include "ffmpegerror.h"

#include "peaks.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include "mediafile.h"
#include "sampleextractor.h"
#include "scenechangeextractor.h"

template<class SampleFormat, bool Planar>
void processFrames(MediaFile &media, AVCodecContext *AudioCodecCtx, AVCodecContext *VideoCodecCtx, AVStream *audioStream, AVStream *videoStream, Peaks &PeakList);

Peaks MediaExtractor::ExtractPeaksAndSceneChanges()
{
    int ret;
    int sample_rate;
    int samples_per_peak;

    AVCodecContext *AudioCodecCtx = AudioStream->codec;
    AVCodecContext *VideoCodecCtx = nullptr;

    AVCodec *AudioCodec = avcodec_find_decoder(AudioCodecCtx->codec_id);
    //AVCodec *VideoCodec = avcodec_find_decoder(VideoCodecCtx->codec_id);

    if(!AudioCodec)
        throw FFmpegError("Could not find any decoder for this audio stream");
    //if(!VideoCodec)
     //   throw FFmpegError("Could not find any decoder for this video stream");

    ret = avcodec_open2(AudioCodecCtx, AudioCodec, nullptr);
    if(ret < 0)
        throw FFmpegError(ret);

    //ret = avcodec_open2(VideoCodecCtx, VideoCodec, nullptr);
    if(ret < 0)
        throw FFmpegError(ret);

    sample_rate = AudioCodecCtx->sample_rate;
    samples_per_peak = sample_rate / 100;

    Peaks PeakList(samples_per_peak, sample_rate);

    switch(AudioCodecCtx->sample_fmt)
    {
    case AV_SAMPLE_FMT_DBL:
        processFrames<double, false>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_DBLP:
        processFrames<double, true>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_FLT:
        processFrames<float, false>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_FLTP:
        processFrames<float, true>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_S32:
        processFrames<int32_t, false>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_S32P:
        processFrames<int32_t, true>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_S16:
        processFrames<int16_t, false>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_S16P:
        processFrames<int16_t, true>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_U8:
        processFrames<uint8_t, false>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    case AV_SAMPLE_FMT_U8P:
        processFrames<uint8_t, true>(Media, AudioCodecCtx, VideoCodecCtx, AudioStream, VideoStream, PeakList);
        break;
    default:
        throw FFmpegError("Sample format not supported");
    }

    avcodec_close(AudioCodecCtx);
    //avcodec_close(VideoCodecCtx);
    emit finished();
    return std::move(PeakList);
}

template<class SampleFormat, bool Planar>
void processFrames(MediaFile &media, AVCodecContext *AudioCodecCtx, AVCodecContext *VideoCodecCtx, AVStream *audioStream, AVStream *videoStream, Peaks &PeakList)
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
      av_packet_unref(&orig_pkt);
   }

   PExtractor.flush();
   //SCExtractor.flush();
   PExtractor.normalizePeaks();
}
