#include "audioProcessor/packetiterator.h"
#include "audioProcessor/inputfile.h"
#include "audioProcessor/ffmpegerror.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

// PacketIterator implementation

PacketIterator::PacketIterator(InputFile &file) :
    File(&file),
    Packet(new AVPacket)
{
    av_init_packet(Packet.get());
    Packet->data = nullptr;
    Packet->size = 0;
    ++(*this); // Get next packet
}

PacketIterator::PacketIterator() :
    File(nullptr),
    Packet()
{ }

AVPacket *PacketIterator::operator ->()
{
    return Packet.get();
}

AVPacket &PacketIterator::operator *()
{
    return *Packet;
}

AVPacket *PacketIterator::operator &()
{
    return Packet.get();
}

PacketIterator &PacketIterator::operator++()
{
    try
    {
        int result = File->readNextPacket(*Packet);
    }
    catch(const FFmpegError &)
    {
        // Consider the stream to be finished
        File = nullptr;
    }
    return *this;
}

