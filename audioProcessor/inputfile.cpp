#include "audioProcessor/inputfile.h"

#include "audioProcessor/ffmpegerror.h"

extern "C"
{
#include <libavformat/avformat.h>
}

AudioStreamIterator::AudioStreamIterator(StreamIterator begin, StreamIterator end) :
    Begin(begin),
    End(end)
{
    while(Begin != End && (*Begin)->codec->codec_type != AVMEDIA_TYPE_AUDIO)
    {
        ++Begin;
    }
}

AudioStreamIterator &AudioStreamIterator::operator ++()
{
    do
    {
        ++Begin;
    } while(Begin != End && (*Begin)->codec->codec_type != AVMEDIA_TYPE_AUDIO);
    return *this;
}

VideoStreamIterator::VideoStreamIterator(StreamIterator begin, StreamIterator end) :
    Begin(begin),
    End(end)
{
    while(Begin != End && (*Begin)->codec->codec_type != AVMEDIA_TYPE_VIDEO)
    {
        ++Begin;
    }
}

VideoStreamIterator &VideoStreamIterator::operator ++()
{
    do
    {
        ++Begin;
    } while(Begin != End && (*Begin)->codec->codec_type != AVMEDIA_TYPE_VIDEO);
    return *this;
}

bool InputFile::avInitiated = false;

InputFile::InputFile(const char *filename) :
    FormatCtx(nullptr)
{
    // If not registered, register formats and codecs
    initAllAV();

    // Open the input file
    int result = avformat_open_input(&FormatCtx, filename, nullptr, nullptr);
    if(result < 0)
    {
        // Issue an error
        throw FFmpegError(result);
    }

    //Retrieve stream informations
    result = avformat_find_stream_info(FormatCtx, nullptr);
    if(result < 0)
    {
        // Issue an error
        throw FFmpegError(result);
    }
}

InputFile::~InputFile()
{
    avformat_close_input(&FormatCtx);
}

StreamIterator InputFile::streams_begin()
{
    return StreamIterator(*this, FormatCtx->streams);
}

StreamIterator InputFile::streams_end()
{
    return StreamIterator(*this, &FormatCtx->streams[FormatCtx->nb_streams]);
}

StreamIterator InputFile::best_stream_of_type(MediaType type)
{
    AVMediaType Type;
    switch(type)
    {
    case MediaType::Audio:
        Type = AVMEDIA_TYPE_AUDIO;
        break;
    case MediaType::Video:
        Type = AVMEDIA_TYPE_VIDEO;
        break;
    case MediaType::Subtitle:
        Type = AVMEDIA_TYPE_SUBTITLE;
        break;
    default:
        throw FFmpegError("Invalid media type");
    }
    int result = av_find_best_stream(FormatCtx, Type, -1, -1, nullptr, 0);
    if(result < 0)
    {
        // Issue an error
        //throw FFmpegError(result);
        return streams_end();
    }
    // result indicates the index of the stream we're looking for in FormatCtx->streams
    return StreamIterator(*this, &FormatCtx->streams[result]);
}

int InputFile::readNextPacket(AVPacket &pkt)
{
    int res = av_read_frame(FormatCtx,  &pkt);
    if(res < 0)
    {
       // Issue an error
        throw FFmpegError(res);
    }
    return res;
}

bool InputFile::getNextPacket(AVPacket &pkt) noexcept
{
    return av_read_frame(FormatCtx, &pkt) >= 0;
}

double InputFile::duration_in_seconds() const
{
    return static_cast<double>(FormatCtx->duration) / AV_TIME_BASE;
}

void InputFile::initAllAV()
{
    if(!avInitiated)
    {
        av_register_all();
        avInitiated = true;
    }
}
