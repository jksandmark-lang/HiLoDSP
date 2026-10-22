#include "daisy_seed.h"
#include "daisysp.h"
#include "AudioClock.h"
#include "CrossoverProcessor.h"
#include "MediaPlayer.h"

using namespace daisy;

static DaisySeed          hw;
static CrossoverProcessor crossover;
static MediaPlayer        player;

// Output fader, used to mute around sample-rate changes.
static volatile float gain_       = 1.0f;
static volatile float gainTarget_ = 1.0f;
static volatile bool  gainMuted_  = false;
static volatile float fadeStep_   = 1.0f / (96000.0f * 0.010f); // 10 ms

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float l, r;
        if(!player.Read(l, r))
        {
            l = in[0][i];
            r = in[1][i];
        }

        float stL, stR, infra;
        crossover.Process(l, r, stL, stR, infra);

        if(gain_ < gainTarget_)
        {
            gain_ += fadeStep_;
            if(gain_ > gainTarget_)
                gain_ = gainTarget_;
        }
        else if(gain_ > gainTarget_)
        {
            gain_ -= fadeStep_;
            if(gain_ < gainTarget_)
                gain_ = gainTarget_;
        }
        gainMuted_ = (gain_ == 0.0f);

        out[0][i] = stL * gain_;
        out[1][i] = stR * gain_;
        out[2][i] = infra * gain_;
    }
}

static Rate currentRate = Rate::RATE_96000;

/** Switches the audio clock and re-inits the crossover for the new rate. */
static void ApplyRate(Rate rate)
{
    if(rate == currentRate)
        return;

    // Fade out before touching the clocks.
    gainTarget_ = 0.0f;
    while(!gainMuted_)
        hw.DelayMs(1);

    hw.StopAudio();
    SetAudioRate(hw, rate);
    fadeStep_ = 1.0f / (static_cast<float>(rate) * 0.010f);
    crossover.Init(static_cast<float>(rate));
    hw.StartAudio(AudioCallback);
    currentRate = rate;

    gainTarget_ = 1.0f; // fade back in
}



int main(void)
{
    hw.Init();
    SetAudioRate(hw, Rate::RATE_96000);
    hw.SetAudioBlockSize(4);

    crossover.Init(hw.AudioSampleRate());

    hw.StartAudio(AudioCallback);

    // Play the first file on the card, if there is one.
    if(player.Init() && player.ScanDir("/") && player.FileCount() > 0
       && player.Open(static_cast<size_t>(0)))
    {
        ApplyRate(RateFor(player.SampleRate()));
    }

    while(1)
    {
        player.Update();

        // Back to the analogue rate once the file has finished.
        if(!player.IsPlaying() && currentRate != Rate::RATE_96000)
            ApplyRate(Rate::RATE_96000);
    }
}