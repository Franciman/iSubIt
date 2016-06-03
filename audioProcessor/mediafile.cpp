#include "audioProcessor/mediafile.h"

extern "C"
{
#include <libavformat/avformat.h>
}

#include "audioProcessor/inputfile.h"
#include "audioProcessor/ffmpegerror.h"
#include "audioProcessor/sampleextractor.h"

#include <algorithm>
#include <cmath>

template<class SampleFormat, bool Planar>
void getPeaks(PacketIterator &begin, PacketIterator &end, AVCodecContext *CodecCtx, int CurrStreamIndex, int SamplesPerPeak, Peaks &Result)
{
    SampleExtractor<SampleFormat, Planar> extractor(SamplesPerPeak, CodecCtx, CurrStreamIndex, Result);
    std::for_each(begin, end, extractor);
    extractor.flush();
}

PeaksExtractor::PeaksExtractor(AudioStreamIterator &stream) :
    Stream(stream),
    CodecCtx(nullptr),
    Codec(nullptr)
{
    CodecCtx = (*Stream)->codec;
    Codec = avcodec_find_decoder(CodecCtx->codec_id);
    if(!Codec)
    {
       throw FFmpegError("Could not find decoder for this audio stream");
    }
    int ret = avcodec_open2(CodecCtx, Codec, nullptr);
    if(ret < 0)
    {
        throw FFmpegError(ret);
    }
}

PeaksExtractor::~PeaksExtractor()
{
    avcodec_close(CodecCtx);
}

int PeaksExtractor::sampleRate() const
{
    return CodecCtx->sample_rate;
}

Peaks PeaksExtractor::makePeaks()
{
    InputFile &File = Stream.getInputFile();
    PacketIterator start = File.packets_begin();
    PacketIterator finish = File.packets_end();
    int CurrStreamIndex = (*Stream)->index;
    int SamplesPerPeak = sampleRate() / 100;
    Peaks result(SamplesPerPeak, sampleRate());
    switch(CodecCtx->sample_fmt)
    {
    case AV_SAMPLE_FMT_DBL:
        getPeaks<double, false>(start, finish, CodecCtx, CurrStreamIndex, SamplesPerPeak, result);
        break;
    case AV_SAMPLE_FMT_DBLP:
        getPeaks<double, true>(start, finish, CodecCtx, CurrStreamIndex, SamplesPerPeak, result);
        break;
    case AV_SAMPLE_FMT_FLT:
        getPeaks<float, false>(start, finish, CodecCtx, CurrStreamIndex, SamplesPerPeak, result);
        break;
    case AV_SAMPLE_FMT_FLTP:
        getPeaks<float, true>(start, finish, CodecCtx, CurrStreamIndex, SamplesPerPeak, result);
        break;
    case AV_SAMPLE_FMT_S32:
        getPeaks<int32_t, false>(start, finish, CodecCtx, CurrStreamIndex, SamplesPerPeak, result);
        break;
    case AV_SAMPLE_FMT_S32P:
        getPeaks<int32_t, true>(start, finish, CodecCtx, CurrStreamIndex, SamplesPerPeak, result);
        break;
    case AV_SAMPLE_FMT_S16:
        getPeaks<int16_t, false>(start, finish, CodecCtx, CurrStreamIndex, SamplesPerPeak, result);
        break;
    case AV_SAMPLE_FMT_S16P:
        getPeaks<int16_t, true>(start, finish, CodecCtx, CurrStreamIndex, SamplesPerPeak, result);
        break;
    case AV_SAMPLE_FMT_U8:
        getPeaks<uint8_t, false>(start, finish, CodecCtx, CurrStreamIndex, SamplesPerPeak, result);
        break;
    case AV_SAMPLE_FMT_U8P:
        getPeaks<uint8_t, true>(start, finish, CodecCtx, CurrStreamIndex, SamplesPerPeak, result);
        break;
    default:
        throw FFmpegError("Sample format not supported");
    }
    return std::move(result);
}
