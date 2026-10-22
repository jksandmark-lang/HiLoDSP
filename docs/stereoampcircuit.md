# Outputs and Headphone Switching

Stereo throughout. Two channels (L and R), each with its own contact pair in the
switched jack and its own pull-down. The infra channel is mono and has no
headphone path.

## Channels

| Index | Signal |
|-------|--------|
| 0 | Stereo L (speakers + headphones) |
| 1 | Stereo R (speakers + headphones) |
| 2 | Infra (mono, speakers only) |

There is no software headphone channel. Headphones are the **same stereo L/R
pair**, split off in hardware, one signal/switch pair per channel.

## How the speaker mute works

The headphone output is a **switched 3.5 mm stereo (TRS) jack**. Per channel it
has:

- a **signal pin** — tip for L, ring for R — the contact that touches the plug
- a **switch pin** — normally touching its own signal pin

Reference: R. Elliott (ESP), *Phone Jacks and Plugs*, figure 7 "TRS
Speaker/Headphone Switching" — <https://sound-au.com/articles/phone-jacks.htm>

> "TRS sockets often have basic switching that disconnects internal speakers (for
> example) when a set of headphones is plugged in. This is very common."

> "The version shown is the simplest arrangement, using simple SPST switches for
> Tip and Ring."

> "Simple switching such as that shown is very common. The signal will rarely be
> the actual speaker output, as connecting headphones directly to a power amp
> would not be sensible."

Same topology worked through on Stack Exchange, tied to the jack pinout —
<https://electronics.stackexchange.com/questions/596424/headphone-and-speaker-switching>

> "The signal should go to pin-2, and exit through pin-4 to speaker amplifier."

That answer describes one channel (the amp there is mono); a stereo build repeats
it for the ring contact.

No relay, no GPIO, no firmware. The mute is a spring contact broken by the plug.

## Version A — line level through the jack (no headphone amp)

Identical for L (tip) and R (ring):

```
DAC / line out ──► signal pin (tip or ring)
switch pin     ──► power amp input ──┬── 47 kΩ ── GND
```

- **No plug inserted:** signal pin and switch pin are still touching, so the
  signal passes straight through to that channel of the power amp.
- **Plug inserted:** the plug pushes the spring away from the switch pin, the
  contact opens, and the power amp input is disconnected. The signal now goes
  into the plug — the headphones.

Two channels means two signal pins, two switch pins and two pull-downs.

## Version B — with a headphone amp

On this type of jack the switch pin is normally shorted to its own signal pin, so
the headphone amp cannot drive the signal pin while the switch pin feeds the power
amp — with nothing plugged in the amp output would sit on the power amp input.
For this version the jack needs **isolated switch contacts**: a separate
normally-closed pair, not one tied to tip/ring (e.g. the Jalco JS5077 listed
below, "twin isolated switched contacts").

```
DAC L ──┬──► headphone amp L ──► jack tip
        └──► isolated NC contact ──► power amp L input ──┬── 47 kΩ ── GND

DAC R ──┬──► headphone amp R ──► jack ring
        └──► isolated NC contact ──► power amp R input ──┬── 47 kΩ ── GND
```

- **No plug inserted:** both isolated contacts closed, both power amp inputs get
  line level from the DAC. Headphone amp output goes nowhere (no plug).
- **Plug inserted:** both isolated contacts open, both power amp inputs
  disconnect and mute, both headphone channels get the amplified signal.

Headphone amp output never reaches the power amp.

Headphone amp — **stereo**, TPA6120A2, in stock at Bits & Parts (NL):

<https://www.bitsandparts.nl/en/headphone-amplifier-tpa6120-p1924319>

That board needs a **dual supply, ±5 V to ±15 V** — it is not a single-rail part.

## Pull-down resistors

One per channel, on each power amp input, to ground:

- **47 kΩ**, 0.25 W, 1 % (metal film)
- **two required** — one for L, one for R
- holds the power amp input at 0 V while the switch contact is open, so it does
  not hum, buzz, or click on plug insertion
- the value only has to be low enough to kill the floating input and high enough
  not to load line level — 47 kΩ sits comfortably between the two

## Caveats

- The switch contacts are for **line level**, not speaker level. It goes between
  the DAC and the power amp input, never after the amp.
- Contact rating is small (typically a few hundred mA / 30 V). Line level is
  well inside that.
- It is mechanical, so there can be a small click on insertion. The 47 kΩ
  pull-down limits it.
- Some jacks short tip to the switch pin during partial insertion. At line level
  that is harmless; it only matters if the contacts carry speaker current.

## Switched jack

**Small — 3.5 mm, breadboard/PCB**

Adafruit — Breadboard-Friendly 3.5 mm Stereo Headphone Jack, 0.1" pin
spacing, with disconnect switches on both Left and Right:

<https://www.adafruit.com/product/1699>

Its switches are tied to tip/ring, so it suits **Version A** only. Adafruit does
not publish a contact rating for this part.

**Large — 6.35 mm (1/4"), PCB**

Jalco JS5077, Rapid order code 20-1399, £3.48:

<https://www.rapidonline.com/jalco-js5077-professional-switched-jack-skt-20-1399>

- 6.35 mm stereo, right-angle PCB mount, solder terminals
- Rated 1 A / 12 V
- Twin isolated switched contacts
- Insulated threaded bush, chrome ring bezel, mounting nut included

Same circuit as the 3.5 mm jack: normally-closed switch contacts, opened by the
plug.
