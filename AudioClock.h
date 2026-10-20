#pragma once
#include "daisy_seed.h"

#include <cstdint>

/** Sample rates the unit can run at.
 *
 *  The SAI takes its clock from PLL3 and divides it by an integer, so 44.1k
 *  needs PLL3 moved to 45.1584 MHz (45.1584 MHz / 256 / 4 = 44100).
 *  48/96k use the stock PLL3 of 49.1667 MHz.
 */
enum class Rate : uint32_t
{
    RATE_44100 = 44100,
    RATE_48000 = 48000,
    RATE_96000 = 96000,
};

/** Picks the closest supported rate for an audio sample rate in Hz. */
Rate RateFor(float fs);

/** Programs PLL3 and the SAI for the given rate. Audio must be stopped. */
void SetAudioRate(daisy::DaisySeed &hw, Rate rate);

/** Sample rate in Hz for a Rate. */
static inline float RateHz(Rate r) { return static_cast<float>(r); }
