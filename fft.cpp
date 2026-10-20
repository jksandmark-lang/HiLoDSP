#include "fft.h"
#include <cmath>

HiLoFFT::HiLoFFT()
: fftSize_(0)
, sweepLength_(0)
, writeIdx_(0)
, captureBuffer_(nullptr)
, capturing_(false)
, captureDone_(false)
, spectrumBinCount_(0)
{
}

HiLoFFT::~HiLoFFT()
{
    delete[] captureBuffer_;
}

void HiLoFFT::Init(size_t sweepLength)
{
    sweepLength_ = sweepLength;

    fftSize_ = 1;
    while(fftSize_ * 2 <= sweepLength_ && fftSize_ * 2 <= kMaxFftSize)
    {
        fftSize_ *= 2;
    }

    delete[] captureBuffer_;
    captureBuffer_ = new float[sweepLength_];

    fft_.Init();

    writeIdx_         = 0;
    capturing_        = false;
    captureDone_      = false;
    spectrumBinCount_ = 0;
}

void HiLoFFT::StartCapture()
{
    writeIdx_    = 0;
    capturing_   = true;
    captureDone_ = false;
}

void HiLoFFT::ProcessSample(float rawSample)
{
    if(!capturing_)
        return;

    captureBuffer_[writeIdx_++] = rawSample;

    if(writeIdx_ >= sweepLength_)
    {
        capturing_   = false;
        captureDone_ = true;
    }
}

void HiLoFFT::StopCapture()
{
    capturing_ = false;
    if(writeIdx_ >= sweepLength_)
    {
        captureDone_ = true;
    }
}

bool HiLoFFT::IsCaptureComplete() const
{
    return captureDone_;
}

void HiLoFFT::ComputeSpectrum(float odr, float gPerLSB)
{
    // Copy capture into FFT workspace
    for(size_t i = 0; i < fftSize_; ++i)
    {
        buffer_[i] = captureBuffer_[i];
    }

    // FFT (in-place, variable size)
    size_t numPasses = 0;
    {
        size_t n = fftSize_;
        while(n >>= 1) { ++numPasses; }
    }
    fft_.Direct(buffer_, buffer_, numPasses);

    // Extract bins 0 - 500 Hz
    float binWidth  = odr / static_cast<float>(fftSize_);
    size_t numBins  = fftSize_ / 2;
    size_t binLimit = static_cast<size_t>(500.0f / binWidth) + 1;
    if(binLimit < numBins) numBins = binLimit;
    spectrumBinCount_ = (numBins < kMaxOutputBins) ? numBins : kMaxOutputBins;

    for(size_t bin = 0; bin < spectrumBinCount_; ++bin)
    {
        float re, im;
        if(bin == 0)
        {
            re = buffer_[0];
            im = 0.0f;
        }
        else
        {
            re = buffer_[2 * bin];
            im = buffer_[2 * bin + 1];
        }

        float mag  = sqrtf(re * re + im * im) / static_cast<float>(fftSize_);
        float gMag = mag * gPerLSB;

        float dbMag;
        if(gMag <= 0.0f)
            dbMag = kDbFloor;
        else
        {
            dbMag = 20.0f * log10f(gMag);
            if(dbMag < kDbFloor) dbMag = kDbFloor;
        }

        spectrum_[bin].freq        = static_cast<float>(bin) * binWidth;
        spectrum_[bin].dbMagnitude = dbMag;
    }
}

size_t HiLoFFT::GetBinCount() const
{
    return spectrumBinCount_;
}

const HiLoFFT::Bin* HiLoFFT::GetSpectrum() const
{
    return spectrum_;
}