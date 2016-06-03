#ifndef INPUTFILE_H
#define INPUTFILE_H


#include <iterator>

#include "packetiterator.h"

struct AVStream;

class InputFile;

class StreamIterator : public std::iterator<std::input_iterator_tag, AVStream*>
{
protected:
    StreamIterator(InputFile &f, AVStream **streams) :
        File(f),
        CurrStream(streams)
    { }

public:
    AVStream *operator*()
    {
        return *CurrStream;
    }

    StreamIterator &operator++()
    {
        ++CurrStream;
        return *this;
    }
    StreamIterator operator++(int)
    {
        StreamIterator res = *this;
        ++(*this);
        return res;
    }

    bool operator==(const StreamIterator &other) const
    {
        return CurrStream == other.CurrStream;
    }

    bool operator!=(const StreamIterator &other) const
    {
        return !(*this == other);
    }

    InputFile &getInputFile()
    {
        return File;
    }

private:
    InputFile &File;
    AVStream **CurrStream;
    friend class InputFile;
};

// StreamIterator that filters only audio stream
class AudioStreamIterator
{
public:
    AudioStreamIterator(StreamIterator begin, StreamIterator end);

    AVStream *operator*()
    {
        return *Begin;
    }
    AudioStreamIterator &operator++();
    AudioStreamIterator operator++(int)
    {
        AudioStreamIterator res = *this;
        ++(*this);
        return res;
    }

    bool operator==(const AudioStreamIterator &other) const
    {
        return Begin == other.Begin;
    }

    bool operator!=(const AudioStreamIterator &other) const
    {
        return !(*this == other);
    }

    InputFile &getInputFile()
    {
        return Begin.getInputFile();
    }

private:
    StreamIterator Begin;
    StreamIterator End;
};

class VideoStreamIterator
{
public:
    VideoStreamIterator(StreamIterator begin, StreamIterator end);

    AVStream *operator*()
    {
        return *Begin;
    }
    VideoStreamIterator &operator++();
    VideoStreamIterator operator++(int)
    {
        VideoStreamIterator res = *this;
        ++(*this);
        return res;
    }

    bool operator==(const VideoStreamIterator &other) const
    {
        return Begin == other.Begin;
    }

    bool operator!=(const VideoStreamIterator &other) const
    {
        return !(*this == other);
    }

    InputFile &getInputFile()
    {
        return Begin.getInputFile();
    }
private:
    StreamIterator Begin;
    StreamIterator End;
};

enum class MediaType
{
    Audio,
    Video,
    Subtitle,
};

// FFmpeg type forward declaration
struct AVFormatContext;
struct AVPacket;

// This is the type of Files you want to read
// This type is not copyable, only moveable
class InputFile
{
public:
    // Open a file for reading data
    // This function throws an FFmpegError if an error occurs
    InputFile(const char *filename);
    InputFile(const InputFile &) = delete; // Disable copy constructor

    // Copy the pointer to the other FormatCtx
    // and set the other FormatCtx to nullptr
    // so that when other is deleted the file is not closed
    InputFile(InputFile &&other) :
        FormatCtx(other.FormatCtx)
    {
        other.FormatCtx = nullptr;
    }

    ~InputFile();

    InputFile &operator=(const InputFile &) = delete; // Disable assignment

    StreamIterator streams_begin();

    StreamIterator streams_end();

    AudioStreamIterator audio_streams_begin()
    {
        return AudioStreamIterator(streams_begin(), streams_end());
    }

    AudioStreamIterator audio_streams_end()
    {
        return AudioStreamIterator(streams_end(), streams_end());
    }

    VideoStreamIterator video_streams_begin()
    {
        return VideoStreamIterator(streams_begin(), streams_end());
    }

    VideoStreamIterator video_streams_end()
    {
        return VideoStreamIterator(streams_end(), streams_end());
    }

    // Return the best stream of `type` type
    // If none is found return streams_end();
    StreamIterator best_stream_of_type(MediaType type);

    // Same as best_stream_of_type
    // but specialized for audio stream
    // This is just a convenience function
    StreamIterator best_audio_stream()
    {
        return best_stream_of_type(MediaType::Audio);
    }

    StreamIterator best_video_stream()
    {
        return best_stream_of_type(MediaType::Video);
    }

    PacketIterator packets_begin()
    {
        return PacketIterator(*this);
    }

    PacketIterator packets_end()
    {
        return PacketIterator();
    }

    double duration_in_seconds() const;

    // This function throws an FFmpegError if an error occurs
    int readNextPacket(AVPacket &pkt);

    // This function returns true if the packet was read
    // false otherwise
    bool getNextPacket(AVPacket &pkt) noexcept;
private:

    static void initAllAV();

    static bool avInitiated;

    AVFormatContext *FormatCtx;
};

#endif // INPUTFILE_H
