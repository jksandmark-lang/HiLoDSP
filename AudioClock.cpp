#include "AudioClock.h"

#include "stm32h7xx_hal.h"

using namespace daisy;

/** Reprogram PLL3 (SAI kernel clock). Audio must be stopped. */
static void SetPll3(uint32_t n, uint32_t fracn)
{
    RCC_PeriphCLKInitTypeDef clk;
    HAL_RCCEx_GetPeriphCLKConfig(&clk);

    clk.PeriphClockSelection
        |= RCC_PERIPHCLK_SAI1 | RCC_PERIPHCLK_SAI2 | RCC_PERIPHCLK_ADC
           | RCC_PERIPHCLK_I2C4;

    clk.PLL3.PLL3M      = 6;
    clk.PLL3.PLL3N      = n;
    clk.PLL3.PLL3P      = 16;
    clk.PLL3.PLL3Q      = 4;
    clk.PLL3.PLL3R      = 32;
    clk.PLL3.PLL3RGE    = RCC_PLL3VCIRANGE_1;
    clk.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    clk.PLL3.PLL3FRACN  = fracn;

    HAL_RCCEx_PeriphCLKConfig(&clk);
}

Rate RateFor(float fs)
{
    if(fs < 46000.f)
        return Rate::RATE_44100;
    if(fs < 72000.f)
        return Rate::RATE_48000;
    return Rate::RATE_96000;
}

void SetAudioRate(DaisySeed &hw, Rate rate)
{
    switch(rate)
    {
        case Rate::RATE_44100:
            SetPll3(271, 7782); // 45.1584 MHz -> divider 4 -> 44.1k
            hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
            break;

        case Rate::RATE_48000:
            SetPll3(295, 0); // 49.1667 MHz -> divider 4 -> 48.0k
            hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
            break;

        case Rate::RATE_96000:
            SetPll3(295, 0); // 49.1667 MHz -> divider 2 -> 96.0k
            hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_96KHZ);
            break;
    }
}
