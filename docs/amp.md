# Outputs and Headphone Switching

## Channels

| Index | Signal |
|-------|--------|
| 0 | Stereo L (speakers) |
| 1 | Stereo R (speakers) |
| 2 | Infra (mono) |

There is no software headphone channel. Headphones are the **same stereo L/R
pair**, split off in hardware.

## How the speaker mute works

The headphone output is a **switched 3.5 mm jack**. Per channel it has:

- a **tip/ring pin** — the contact that touches the plug
- a **switch pin** — normally touching the tip/ring pin

Wiring:

```
DAC / line out ──► tip pin
switch pin    ──► power amp input
```

- **No plug inserted:** tip pin and switch pin are still touching, so the signal
  passes straight through to the power amp.
- **Plug inserted:** the plug pushes the tip spring away from the switch pin, the
  contact opens, and the power amp input is disconnected. The signal now goes
  into the plug — the headphones.

No relay, no GPIO, no firmware. The mute is a spring contact broken by the plug.

Add a resistor (~47 kΩ) from the power amp input to ground so the amp input is
not left floating when the jack disconnects it.

## Caveats

- The switch contacts are for **line level**, not speaker level. It goes between
  the DAC and the power amp input, never after the amp.
- Contact rating is small (typically a few hundred mA / 30 V). Line level is
  well inside that.
- It is mechanical, so there can be a small click on insertion. A resistor from
  the amp input to ground limits it.

## Switched jack

**Small — 3.5 mm, breadboard/PCB**

Adafruit — Breadboard-Friendly 3.5 mm Stereo Headphone Jack, 0.1" pin
spacing, with disconnect switches on both Left and Right:

<https://www.adafruit.com/product/1699>

Adafruit does not publish a contact rating for this part.

**Large — 6.35 mm (1/4"), PCB**

Jalco JS5077, Rapid order code 20-1399, £3.48:

<https://www.rapidonline.com/jalco-js5077-professional-switched-jack-skt-20-1399>

- 6.35 mm stereo, right-angle PCB mount, solder terminals
- Rated 1 A / 12 V
- Twin isolated switched contacts
- Insulated threaded bush, chrome ring bezel, mounting nut included

Same circuit as the 3.5 mm jack: normally-closed switch contacts, opened by the
plug.
