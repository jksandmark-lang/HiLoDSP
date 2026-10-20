#include "CrossoverProcessor.h"

void CrossoverProcessor::Init(float sampleRate)
{
    struct Band
    {
        daisysp::Svf* left;
        daisysp::Svf* right;
        float         freq;
    };

    const Band bands[] = {
        // ST band
        {&hpSTLeft_, &hpSTRight_, hpSTFreq_},
        {&bnd1STLeft_, &bnd1STRight_, bnd1STFreq_},
        {&bnd2STLeft_, &bnd2STRight_, bnd2STFreq_},
        {&bnd3STLeft_, &bnd3STRight_, bnd3STFreq_},
        // Infra band
        {&hpInfraLeft_, &hpInfraRight_, hpInfraFreq_},
        {&bnd1InfraLeft_, &bnd1InfraRight_, bnd1InfraFreq_},
        {&bnd2InfraLeft_, &bnd2InfraRight_, bnd2InfraFreq_},
        {&bnd3InfraLeft_, &bnd3InfraRight_, bnd3InfraFreq_},
        {&lpInfraLeft_, &lpInfraRight_, lpInfraFreq_},
    };

    for(const auto& band : bands)
    {
        band.left->Init(sampleRate);
        band.right->Init(sampleRate);
        band.left->SetRes(kRes);
        band.right->SetRes(kRes);
        band.left->SetFreq(band.freq);
        band.right->SetFreq(band.freq);
    }
}

void CrossoverProcessor::Process(float  inL,
                                 float  inR,
                                 float& outSTL,
                                 float& outSTR,
                                 float& outInfra)
{
    outSTL = ProcessChannel_(hpSTLeft_, bnd1STLeft_, bnd2STLeft_,
                             bnd3STLeft_, nullptr, inL);
    outSTR = ProcessChannel_(hpSTRight_, bnd1STRight_, bnd2STRight_,
                             bnd3STRight_, nullptr, inR);

    outInfra = 0.5f * (ProcessChannel_(hpInfraLeft_, bnd1InfraLeft_, bnd2InfraLeft_,
                                       bnd3InfraLeft_, &lpInfraLeft_, inL)
                     + ProcessChannel_(hpInfraRight_, bnd1InfraRight_, bnd2InfraRight_,
                                       bnd3InfraRight_, &lpInfraRight_, inR));
}

// ── Private helpers ──

float CrossoverProcessor::ProcessChannel_(daisysp::Svf&  hp,
                                          daisysp::Svf&  bnd1,
                                          daisysp::Svf&  bnd2,
                                          daisysp::Svf&  bnd3,
                                          daisysp::Svf*  lp,
                                          float          input)
{
    hp.Process(input);
    bnd1.Process(hp.High());
    bnd2.Process(bnd1.Peak());
    bnd3.Process(bnd2.Peak());
    if(lp == nullptr)
        return bnd3.Peak();
    lp->Process(bnd3.Peak());
    return lp->Low();
}