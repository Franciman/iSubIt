#ifndef PACKETITERATOR_H
#define PACKETITERATOR_H

#include <memory>

struct AVPacket;

class InputFile;

// An iterator interface to packet reading
class PacketIterator
{
private:
    PacketIterator(InputFile &file);
    // This constructor creates the end iterator
    // it just sets File pointer to nullptr
    PacketIterator();

public:
    PacketIterator(const PacketIterator &other) :
        File(other.File),
        Packet(other.Packet)
    {

    }

    PacketIterator(PacketIterator &&other) :
        File(other.File),
        Packet(std::move(other.Packet))
    {

    }

    bool operator==(const PacketIterator &other) const
    {
        return File == other.File;
    }

    bool operator!=(const PacketIterator &other) const
    {
        return !(*this == other);
    }

    AVPacket *operator ->();

    AVPacket &operator*();

    AVPacket *operator&();

    PacketIterator &operator++();

    PacketIterator operator++(int)
    {
        PacketIterator res = *this;
        ++(*this);
        return res;
    }

private:
    InputFile *File;
    std::shared_ptr<AVPacket> Packet;

    friend class InputFile;
    friend class StreamPacketIterator;
};



#endif // PACKETITERATOR_H
