#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include "peaks.h"

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;

class AudioStreamIterator;

class PeaksExtractor
{
public:
    PeaksExtractor(AudioStreamIterator &stream);
    ~PeaksExtractor();
    // This function throws an FFmpegError
    Peaks makePeaks();

    int sampleRate() const;
private:
    AudioStreamIterator &Stream;
    AVCodecContext *CodecCtx;
    AVCodec *Codec;
};

#endif // MEDIAFILE_H
