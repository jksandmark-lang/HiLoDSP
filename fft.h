#pragma once
#include "stmlib/stmlib.h"
#include "stmlib/fft/shy_fft.h"

class HiLoFFT
{
  public:
    struct Bin
    {
        float freq;
        float dbMagnitude;
    };

    HiLoFFT();
    ~HiLoFFT();

    void Init(size_t sweepLength);

    void StartCapture();
    void ProcessSample(float rawSample);
    void StopCapture();
    bool IsCaptureComplete() const;

    void ComputeSpectrum(float odr, float gPerLSB);

    size_t      GetBinCount() const;
    const Bin*  GetSpectrum() const;

  private:
    static constexpr size_t kMaxFftSize      = 4096;
    static constexpr size_t kMaxOutputBins   = 2048;
    static constexpr float  kDbFloor         = -60.0f;

    stmlib::ShyFFT<float, kMaxFftSize> fft_;
    float* captureBuffer_;
    float  buffer_[kMaxFftSize];
    size_t fftSize_;
    size_t sweepLength_;
    size_t writeIdx_;

    bool capturing_;
    bool captureDone_;

    Bin    spectrum_[kMaxOutputBins];
    size_t spectrumBinCount_;
};
