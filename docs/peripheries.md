# Peripherals

Parts from Bits & Parts (Netherlands) unless stated otherwise.

## microSD card reader

**SD Card Reader breakout board/module SPI, MicroSD** — Bits & Parts, €2.95

<https://www.bitsandparts.nl/en/sd-card-reader-breakout-board-module-spi-microsd-p100241>

Listed as temporarily unavailable at the time of writing. In-stock alternative
from the same shop:

**SD Card Reader breakout board/module SPI Mini** — Bits & Parts, €2.25

<https://www.bitsandparts.nl/en/sd-card-reader-card-reader-breakout-board-module-spi-mini-p1929523>

Both are plain SPI breakouts: 3.3 V only, no regulator, no level shifters, pins
on 0.1" spacing. Jumper wires only.

## Infra DAC (SAI2)

**Adafruit PCM5102 I2S DAC** — ART ADA6250, BerryBase (Berlin, EU)

<https://b2b.berrybase.de/Sensoren-/-Module/Audio-/-Schall/Adafruit-PCM5102-I2S-DAC-Modul-Stereo-Line-Ausgang-16-328239Bit-8-3848239kHz-1128239dB-SNR-3-58239V/>

Retail orders via www.berrybase.de, same item number.

- PCM5102A, I2S in, 16/24/32-bit, 8–384 kHz
- **supply 3–5 V** — runs directly from the Seed's 3V3
- 3.3 V logic inputs
- **no MCLK needed** — BCK/WS/DIN only, which suits SAI2
- stereo line output, full scale **2.1 V RMS**, on solder pads / header pins (headers included)
- 3.5 mm jack also fitted; unused, the pins are what we want

The infra channel is mono, so use one output (L or R) — or both, summed — into
the mono class-D amp's input terminals. The DAC only ever sees line level; the
class-D stage is what drives the bass shaker.

## Wiring to the Daisy Seed 2

SDMMC1 pins in libDaisy are fixed (`SdmmcHandler`):

| Signal | GPIO | Seed pin |
|--------|------|----------|
| CK | PC12 | D6 |
| CMD | PD2 | D5 |
| D0 | PC8 | D4 |
| D1 | PC9 | D3 |
| D2 | PC10 | D2 |
| D3 | PC11 | D1 |

An SPI breakout labels the same card contacts differently:

| Breakout pin | Card signal |
|--------------|-------------|
| CLK | CLK |
| DI | CMD |
| DO | DAT0 |
| CS | DAT3 |

So these modules give you **CK, CMD and DAT0** — enough for **1-bit SDMMC**:

```
breakout 3V3 ──► Seed 3V3
breakout GND ──► Seed GND
breakout CLK ──► D6
breakout DI  ──► D5   (CMD)
breakout DO  ──► D4   (DAT0)
```

Set the bus width accordingly in `MediaPlayer::Init()`:

```cpp
cfg.width = SdmmcHandler::BusWidth::BITS_1;
```

4-bit SDMMC needs DAT1 and DAT2 as well, which these SPI breakouts do not bring
out. Use a socket that exposes every card contact (Pololu 2597 type) if full
4-bit width is wanted. 1-bit is ample for WAV playback.

None of the SDMMC pins are ADC pins, so the analogue inputs stay free.
