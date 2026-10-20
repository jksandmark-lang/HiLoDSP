#pragma once
#include "svf.h"


class CrossoverProcessor
{
  public:
    void Init(float sampleRate);

    void Process(float  inL,
                 float  inR,
                 float& outSTL,
                 float& outSTR,
                 float& outInfra);

  private:
    // ── ST Band ──
    daisysp::Svf hpSTLeft_, hpSTRight_;
    daisysp::Svf bnd1STLeft_, bnd1STRight_;
    daisysp::Svf bnd2STLeft_, bnd2STRight_;
    daisysp::Svf bnd3STLeft_, bnd3STRight_;

    // ── Infra Band ──
    daisysp::Svf hpInfraLeft_, hpInfraRight_;
    daisysp::Svf bnd1InfraLeft_, bnd1InfraRight_;
    daisysp::Svf bnd2InfraLeft_, bnd2InfraRight_;
    daisysp::Svf bnd3InfraLeft_, bnd3InfraRight_;
    daisysp::Svf lpInfraLeft_, lpInfraRight_;

    // ── Crossover frequencies ──
    static constexpr float kRes = 0.7f;

    float hpSTFreq_      = 115.f;
    float hpInfraFreq_   = 7.f;
    float bnd1STFreq_    = 250.f;
    float bnd2STFreq_    = 1000.f;
    float bnd3STFreq_    = 4000.f;
    float bnd1InfraFreq_ = 17.f;
    float bnd2InfraFreq_ = 26.f;
    float bnd3InfraFreq_ = 33.f;
    float lpInfraFreq_   = 45.f;

    // ── Helpers ──
    float ProcessChannel_(daisysp::Svf&  hp,
                          daisysp::Svf&  bnd1,
                          daisysp::Svf&  bnd2,
                          daisysp::Svf&  bnd3,
                          daisysp::Svf*  lp,
                          float          input);
};