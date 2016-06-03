#ifndef PEAKS_H
#define PEAKS_H

#include <cstdint>
#include <vector>

using std::int16_t;

class Peak
{
    int16_t Min;
    int16_t Max;
public:
    Peak() { }
    Peak(int16_t min, int16_t max) :
        Min(min),
        Max(max)
    { }

    int16_t min() const
    {
        return Min;
    }

    int16_t max() const
    {
        return Max;
    }

    void min(int16_t value)
    {
        Min = value;
    }

    void max(int16_t value)
    {
        Max = value;
    }
};

class Peaks
{
    std::vector<Peak> PeakList;
    int16_t MinPeak;
    int16_t MaxPeak;
    int SamplesPerPeak;
    int SampleRate;
public:
    Peaks(int samplesPerPeak, int sampleRate)
    {
        SamplesPerPeak = samplesPerPeak;
        SampleRate = sampleRate;
    }

    Peaks(Peaks &&other) :
        PeakList(std::move(other.PeakList)),
        MinPeak(other.MinPeak),
        MaxPeak(other.MaxPeak),
        SamplesPerPeak(other.SamplesPerPeak),
        SampleRate(other.SampleRate)
    { }
    Peaks(std::vector<Peak> &&peakList, int16_t minPeak, int16_t maxPeak, int samplesPerPeak, int sampleRate) :
        PeakList(std::move(peakList)),
        MinPeak(minPeak),
        MaxPeak(maxPeak),
        SamplesPerPeak(samplesPerPeak),
        SampleRate(sampleRate)
    { }

    Peaks &operator=(Peaks &&other)
    {
        PeakList = std::move(other.PeakList);
        MinPeak = other.MinPeak;
        MaxPeak = other.MaxPeak;
        SamplesPerPeak = other.SamplesPerPeak;
        SampleRate = other.SampleRate;
        return *this;
    }

    void samplesPerPeak(int value)
    {
        SamplesPerPeak = value;
    }

    int samplesPerPeak() const
    {
        return SamplesPerPeak;
    }

    void sampleRate(int value)
    {
        SampleRate = value;
    }

    int sampleRate() const
    {
        return SampleRate;
    }

    int16_t minPeak() const
    {
        return MinPeak;
    }

    int16_t maxPeak() const
    {
        return MaxPeak;
    }

    std::vector<Peak>::iterator peaks_begin()
    {
        return PeakList.begin();
    }

    std::vector<Peak>::iterator peaks_end()
    {
        return PeakList.end();
    }

    bool empty() const
    {
        return PeakList.empty();
    }

    void addPeak(Peak &P)
    {
        PeakList.emplace_back(P.min(), P.max());
    }

    void updateMaxPeak(int16_t newMax)
    {
        MaxPeak = newMax;
    }

    void updateMinPeak(int16_t newMin)
    {
        MinPeak = newMin;
    }

    std::vector<Peak>::size_type peaksNumber() const
    {
        return PeakList.size();
    }

    Peak &operator[](std::size_t num)
    {
        return PeakList[num];
    }

    const Peak &operator[](std::size_t num) const
    {
        return PeakList[num];
    }

};

#endif // PEAKS_H
