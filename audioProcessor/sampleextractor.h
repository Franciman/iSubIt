#ifndef SAMPLEEXTRACTOR_H
#define SAMPLEEXTRACTOR_H

#include "audio_frame_traits.h"
#include "sample_format_traits.h"

#include <memory>
#include <iostream>

template<class SampleFormat, bool Planar>
class SampleExtractor
{
    AVCodecContext *CodecCtx;
    int StreamIndex;
    int SamplesPerPeak;
    int SamplesConsidered;
    bool GotFirstSample;
    std::shared_ptr<AVFrame> Frame;
    Peaks &Result;
    Peak CurrPeak;
public:
    SampleExtractor(int samplesPerPeak, AVCodecContext *codecCtx, int streamIndex, Peaks &result) :
        CodecCtx(codecCtx),
        StreamIndex(streamIndex),
        SamplesPerPeak(samplesPerPeak),
        SamplesConsidered(0),
        GotFirstSample(false),
        Frame(nullptr, [](AVFrame *f) { av_frame_free(&f); }),
        Result(result)
    {
        if(SamplesPerPeak <= 0)
        {
           throw FFmpegError("The number of samples per peak must be an integer greater than 0") ;
        }
        Frame = std::shared_ptr<AVFrame>(av_frame_alloc());
        if(!Frame)
        {
            throw FFmpegError("Could not allocate frame, out of memory");
        }
    }

    ~SampleExtractor()
    {
    }

    void operator()(AVPacket &pkt)
    {
        int got_frame;
        if(pkt.stream_index != StreamIndex)
            return;
        AVPacket pkt_cpy = pkt;
        do
        {
            int ret = avcodec_decode_audio4(CodecCtx, Frame.get(), &got_frame, &pkt_cpy);
            if(ret < 0)
            {
                throw FFmpegError(ret);
            }
            if(got_frame)
            {
                processFrame();
            }
            pkt_cpy.size -= ret;
            pkt_cpy.data += ret;
        } while(pkt_cpy.size > 0);
    }

    void flush()
    {
        int got_frame;
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = nullptr;
        pkt.size = 0;
        while(true)
        {
            avcodec_decode_audio4(CodecCtx, Frame.get(), &got_frame, &pkt);
            if(got_frame)
            {
                processFrame();
            }
            else
            {
                break;
            }
        }
        av_free_packet(&pkt);

        // Push the remainder of samples
        if(SamplesConsidered != 0)
        {
            if(CurrPeak.min() < Result.minPeak()) Result.updateMinPeak(CurrPeak.min());
            else if(CurrPeak.max() > Result.maxPeak()) Result.updateMaxPeak(CurrPeak.max());
            Result.addPeak(CurrPeak);
        }
    }

private:
    void processFrame()
    {
        audio_frame_traits<SampleFormat, Planar>::for_each_sample(Frame.get(), [this](SampleFormat sample) {
            this->processSample(sample);
        });
        return;
    }

    void processSample(SampleFormat sample)
    {
        int16_t ConvertedSample = sample_format_traits<SampleFormat>::convertToInt16(sample);
        if(SamplesConsidered == 0)
        {
            if(!GotFirstSample)
            {
                Result.updateMinPeak(ConvertedSample);
                Result.updateMaxPeak(ConvertedSample);
                GotFirstSample = true;
            }
            CurrPeak.min(ConvertedSample);
            CurrPeak.max(ConvertedSample);
            ++SamplesConsidered;
        }
        else
        {
            if(ConvertedSample < CurrPeak.min()) CurrPeak.min(ConvertedSample);
            else if(ConvertedSample > CurrPeak.max()) CurrPeak.max(ConvertedSample);
            ++SamplesConsidered;
        }

        if(SamplesConsidered == SamplesPerPeak * Frame->channels)
        {
            if(CurrPeak.min() < Result.minPeak()) Result.updateMinPeak(CurrPeak.min());
            if(CurrPeak.max() > Result.maxPeak()) Result.updateMaxPeak(CurrPeak.max());
            Result.addPeak(CurrPeak);
            SamplesConsidered = 0;
        }

    }

    void normalizePeaks()
    {
        int maxAbsValue = std::max(std::abs(Result.minPeak()), std::abs(Result.maxPeak()));
        double normFactor = static_cast<double>(INT16_MAX + 1) / maxAbsValue;
        if(normFactor > 1.1)
        {
            for(Peak &p : Result)
            {
                p.min(std::round(p.min()) * normFactor);
                p.max(std::round(p.max()) * normFactor);
            }
        }
    }
};



#endif // SAMPLEEXTRACTOR_H
