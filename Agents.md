# Yottasynth Research Notes

## Goal

This repository is building a Teensy 4.1 based digital synthesizer with:

- Teensy 4.1
- Teensy Audio Shield / SGTL5000 codec
- 320x240 SPI TFT touch screen
- 5 potentiometers
- 1 joystick
- future arpeggiator and sequencer support

This document records:

- which commercial synth is the best reference for feature and UX ideas
- how the current hardware is wired
- how the current firmware works today
- what is already implemented versus what is only planned or legacy

## Recommended Reference Synth

### Primary recommendation: Korg minilogue xd

The best single reference for this project is the **Korg minilogue xd**.

Why it fits this project best:

- It combines an **analogue-style front panel** with a **display-driven workflow**, which matches this project better than a pure knob-per-function synth.
- It has a **joystick**, which maps naturally to the project hardware.
- It already includes the roadmap features this project wants: **arp/latch, sequencer, motion sequencing, effects, and visual screen feedback**.
- Its UI is simpler and more learnable than something like a Novation Summit, which matters because this firmware is still early.
- There is a lot of public documentation and community material around the minilogue family.

Relevant official minilogue xd features:

- 4 voice analogue synth with extra digital engine
- voice modes including `POLY`, `UNISON`, `CHORD`, `ARP / LATCH`
- `16-step polyphonic sequencer`
- `Motion sequence` on up to `4 parameters`
- built-in `modulation`, `delay`, and `reverb`
- `Joy stick`
- `Real-time OLED oscilloscope`

### Why not Moog Grandmother as the main reference

The **Moog Grandmother** is a very good **secondary** reference for sound-design philosophy and mono-synth behavior, but not the best primary UI model.

What is attractive about it:

- strong mono-synth signal flow
- immediate performance-oriented layout
- built-in arpeggiator / sequencer
- spring reverb

Why it is not the best main template here:

- it has **no screen-centered workflow**
- it is built around a **semi-modular patching** mindset that this hardware does not currently expose
- it is a better inspiration for **sound character and one-voice behavior** than for this repo’s touch UI

### Why not Novation Summit as the main reference

The **Novation Summit** is feature-rich, but it is too large and too deep for the current project stage.

Why it is a weaker fit:

- many more simultaneous controls than this hardware has
- much broader modulation and multitimbral scope than the current code
- current firmware is still basically one active synth voice plus placeholder UI

### Practical direction

Use:

- **Korg minilogue xd** as the main UX and feature reference
- **Moog Grandmother** as a secondary reference for mono voice behavior, performance feel, and basic subtractive synth flow

That combination is a better fit than cloning Summit directly.

## External Research Sources

- Korg minilogue xd specs: https://www.korg.com/us/products/synthesizers/minilogue_xd/specifications.php
- Korg minilogue product page: https://www.korg.com/us/products/synthesizers/minilogue/index.php
- Novation Summit user guide intro: https://userguides.novationmusic.com/hc/en-gb/articles/25003992884242-Introduction-to-the-Novation-Summit
- Moog Grandmother manual: https://api.moogmusic.com/sites/default/files/2018-08/Grandmother_Manual.pdf
- LVGL display model docs: https://docs.lvgl.io/master/main-modules/display/overview.html
- PJRC Teensy Audio System Design Tool: https://www.pjrc.com/teensy/gui/

## Current Project Hardware

### Core platform

From the repo and board files:

- MCU: `Teensy 4.1`
- firmware environment: `PlatformIO` + `Arduino`
- display stack: `ILI9341` class TFT via `TFT_eSPI`
- touch controller: `XPT2046`
- audio output codec: `AudioControlSGTL5000`
- control input expansion: `74HC4067` analog multiplexer board (`BOB-09056`)

### Display and touch wiring

From `platformio.ini` and `src/main.cpp`:

- TFT MOSI: `11`
- TFT MISO: `12`
- TFT SCLK: `13`
- TFT CS: `3`
- TFT DC: `2`
- TFT RST: `-1`
- touch CS: `5`
- touch IRQ: `4`
- resolution: `320x240`

### Potentiometers and joystick wiring

From `etc/board/board.kicad_pcb`:

- `RV1`..`RV5` are five potentiometers
- their wipers go into mux channels `C0`..`C4`
- the mux is a `74HC4067` breakout (`U2`)
- mux select pins:
  - `S0 -> A5`
  - `S1 -> A6`
  - `S2 -> A7`
  - `S3 -> A8`
- mux common output:
  - `COM -> A9`

This matches the commented-out firmware in `src/main.cpp`, which manually toggles `A5..A8` and reads `A9`.

There is also a 5-pin connector `J1`:

- pin 1 = `GND`
- pin 2 = `3V3`
- pin 3 = mux `C5`
- pin 4 = mux `C6`
- pin 5 = mux `C7`

Based on the hardware description you gave and the pin count, this is very likely intended for the joystick module. That is an inference from the board wiring and project description; it is not labeled as "joystick" in the schematic text.

## Firmware Stack

### Libraries in use

From `platformio.ini`:

- `paulstoffregen/XPT2046_Touchscreen`
- `bodmer/TFT_eSPI`
- `lvgl/lvgl@9.4.0`

### LVGL configuration

From `include/lv_conf.h`:

- `LV_COLOR_DEPTH 16`
- `LV_MEM_SIZE (64 * 1024U)`
- `LV_USE_MENU 1`
- `LV_USE_LOG 1`
- `LV_USE_TFT_ESPI 1`

This means the project is configured for:

- 16-bit RGB565 UI rendering
- LVGL menu widget support
- LVGL logging over Serial
- LVGL’s TFT_eSPI integration path

## How the UI Works Today

### LVGL model in this project

LVGL’s core model is:

- create one `display`
- create one `input device`
- create widgets on the active screen
- call the LVGL task/tick functions repeatedly

This repo follows that model in `src/main.cpp`.

### Startup sequence

Current startup flow:

1. `lv_init()`
2. `tft.begin()`
3. `tft.setRotation(3)`
4. `ts.begin()`
5. `ts.setRotation(0)`
6. register LVGL log callback to `Serial`
7. create the LVGL display
8. create one pointer input device
9. render the main menu
10. set up audio
11. set up synth
12. set up USB-host MIDI

### Display path

There are two display paths in `src/main.cpp`:

- a custom `my_disp_flush(...)`
- the LVGL TFT_eSPI path `lv_tft_espi_create(...)`

Because `LV_USE_TFT_ESPI` is enabled, the current code uses `lv_tft_espi_create(...)` rather than the custom flush callback.

### Touch path

Touch is implemented in `my_touchpad_read(...)`:

- `ts.touched()` decides pressed/released state
- `ts.getPoint()` reads raw touch coordinates
- hard-coded calibration values map raw coordinates to screen coordinates

Current calibration mapping:

- raw `p.y` mapped from `400..3829` to screen X
- raw `p.x` mapped from `540..3756` to screen Y

Important details:

- touch is the **only user input currently read by active firmware**
- the function still prints touch X values to Serial during touches
- touch calibration is hard-coded and board-specific

### Current UI screens

The active UI is only a placeholder `lv_menu` with three entries:

- `SYNTHESIZER`
- `ARPEGGIATOR`
- `SEQUENCER`

Each entry opens a placeholder page:

- `SYNTH PAGE`
- `ARPEGGIATOR PAGE`
- `SEQUENCER PAGE`

There is no real editor page, modulation page, arp page, or sequencer page yet.

### Settings / mode state

Menu presses write a mode into `Settings`:

- `Mode::SYNTHESIZER`
- `Mode::ARPEGGIATOR`
- `Mode::SEQUENCER`

But that mode is not used anywhere else in the active firmware yet. Today it is only stored, not acted on.

## How MIDI Works Today

### Input path

The project currently expects note input from **USB host MIDI**, not from local keyboard hardware.

`src/play_mode.cpp` sets up:

- `USBHost`
- two USB hubs
- one `MIDIDevice`

The main loop calls:

- `myusb.Task()`
- `midi1.read()`

### What MIDI events actually do

In the active code path:

- `Note On` is forwarded to `synth.onNoteOn(...)`
- `Note Off` is forwarded to `synth.onNoteOff(...)`

Everything else is mostly just logged to Serial:

- Control Change
- Program Change
- Aftertouch
- Pitch Change
- Clock / Start / Stop
- SysEx

Important implication:

- **MIDI CC is not currently changing synth parameters in the active firmware**
- **pitch bend is not currently applied in the active firmware path**
- **arp and sequencer clock callbacks exist, but no arp/sequencer engine uses them yet**

## How Audio Works Today

### Audio library architecture

The project uses the **Teensy Audio Library** and the audio graph in `src/audio_setup.cpp` was clearly generated from the PJRC audio design tool and then committed as C++ objects and patch cords.

The graph contains more than the currently active synth uses:

- lead path
- mid path
- bass path
- drum path
- global mixer
- global filter
- stereo I2S output

### Audio objects present

The audio graph includes:

- waveform oscillators for lead, mid, bass
- pink noise sources for lead, mid, bass
- state-variable filters
- ADSR envelopes
- mixers
- one simple drum object
- SGTL5000 codec output through I2S

### What is actually active right now

The active `Synth` instance in `src/main.cpp` only uses the **lead** lane:

- `lead_waveform1`
- `lead_waveform2`
- `lead_pink`
- `lead_filter`
- `lead_envelope`

So while the audio graph suggests future multi-part expansion, the current instrument behavior is much smaller than the graph implies.

### Audio init

`setupAudio()` currently only does:

- `sgtl5000_1.enable()`
- `sgtl5000_1.volume(0.50)`
- `AudioMemory(30)`

There is no effect block, no delay, no chorus, no reverb, and no audio input routing active yet.

## How the Synth Works Today

### Current synth voice

The active synth in `src/synth.cpp` is basically a **single monophonic subtractive voice**:

- oscillator 1: `sawtooth`
- oscillator 2: `sine`
- one filter
- one envelope
- optional LFO behavior
- no active effects

### Envelope defaults

The active voice envelope is initialized to:

- attack: `10`
- decay: `50`
- sustain: `0.7`
- release: `200`

### Note handling

Incoming notes between `24` and `107` are accepted.

The synth uses an 8-note buffer and behaves as a **last-note-priority mono synth**:

- note-on pushes into buffer and immediately plays
- note-off removes from buffer
- when the top note is released, the previous held note resumes

### Oscillator behavior

When a note plays:

- oscillator 1 frequency = played note
- oscillator 2 frequency = played note plus octave/detune/LFO pitch factor
- oscillator amplitudes follow note velocity
- pink noise amplitude is forced to `0`

That means noise exists in the graph but is effectively muted in the current active path.

### LFO behavior

There is an `LFOupdate(...)` function supporting multiple modes for:

- filter modulation
- pitch modulation
- free, retriggered, and one-shot style behavior

However, in the active firmware today:

- `LFOmodeSelect` defaults to `0`
- `LFOdepth` defaults to `0`
- there is no active UI or MIDI mapping changing these values

So the LFO engine exists in code, but it is not yet really surfaced to the user.

## Current State of Pots and Joystick

### What hardware supports

The board is clearly wired for:

- 5 front-panel potentiometers
- more analog inputs through the mux
- likely joystick access through `J1`

### What firmware actually does

Active firmware currently does **not** scan those controls.

Evidence:

- there is no active `analogRead(...)` loop for controls
- there is no mux scan routine in the live code path
- there are only commented examples in `src/main.cpp`

So the control hardware is present, but not integrated into the running synth/UI yet.

## Legacy / Prototype Code Worth Mining

### `lib/synthesizer/`

There is an older synth implementation in:

- `lib/synthesizer/synthesizer.cpp`
- `lib/synthesizer/synthesizer.h`

This older code is important because it is **more feature-complete in parameter control** than the active `src/synth.cpp`.

It includes MIDI CC mappings such as:

- `100` osc 1 mix
- `101` osc 2 mix
- `102` noise mix
- `103` octave
- `104` attack
- `105` decay
- `106` sustain
- `107` release
- `108` detune
- `109` filter frequency
- `110` resonance
- `111` bend range
- `112` LFO speed
- `113` LFO depth
- `114` LFO mode

This is not the active firmware path right now, but it is useful prior art for reconnecting the hardware controls later.

### `control.pd`

`control.pd` is a Pure Data patch that sends those same CC numbers.

That tells us an older workflow likely looked like:

- external UI in Pure Data
- MIDI CC into the synth engine
- synth parameters updated from CC values

This is useful because it already defines a compact parameter map that can be repurposed for:

- 5 pots
- touch pages
- joystick modifiers

## Important Gaps and Risks

### 1. Hardware inputs are ahead of firmware

The board already supports a muxed control surface, but current firmware only uses:

- touch screen
- USB host MIDI note events

### 2. UI is only structural

The menu exists, but the actual pages for:

- synth editing
- arp editing
- sequencer editing

do not exist yet.

### 3. Audio graph is ahead of synth behavior

The audio graph suggests multiple lanes and future layering, but the active synth uses only the lead lane.

### 4. MIDI support is incomplete in the active path

The active code logs many MIDI messages but only note on/off affect sound.

### 5. Timing for LVGL is rough

The main loop calls:

- `lv_task_handler()`
- `lv_tick_inc(5)`

without basing the tick on elapsed real time. This can make UI timing depend on loop speed.

### 6. `Settings` implementation should be cleaned up before expansion

`include/settings.h` currently has issues:

- `mode` is not initialized by default
- the singleton static is defined in the header
- the destructor deletes `instance`, which is unsafe design

This is not the biggest blocker for the synth, but it should be fixed before the UI state grows.

## Recommended Product Direction for This Repo

### UX model

Use the touch screen as a **page-based panel**, not as a generic menu system.

A good first practical model is:

- page 1: `OSC / MIX`
- page 2: `FILTER / AMP`
- page 3: `MOD`
- page 4: `FX`
- page 5: `ARP`
- page 6: `SEQ`

Then make the 5 pots context-sensitive to the current page.

That matches the real hardware better than trying to copy every physical control of a hardware synth.

### Sound engine model

For early iterations:

- keep the core voice **mono or duo**
- follow **Grandmother-style subtractive clarity**
- follow **minilogue-style page structure and display feedback**

That is a better fit than jumping directly into Summit-level complexity.

### Best next engineering steps

1. Implement mux scanning for `A5..A9` and map `RV1..RV5`.
2. Confirm joystick wiring on `J1` and read its channels through the same mux.
3. Replace placeholder menu pages with one real synth edit page.
4. Reuse the older CC parameter map as the first internal parameter model.
5. Reconnect pitch bend, CC, and modulation handling in the active synth path.
6. Add one first effect, likely delay, before building the sequencer.
7. Build arp before full sequencer, because the current code already receives MIDI clock/start/stop.

## Bottom Line

Today this repo is **not yet a finished synth UI instrument**. It is currently:

- a Teensy 4.1 firmware base
- with working TFT + touch
- working USB host MIDI note input
- working mono synth voice
- a much larger unfinished audio graph
- and hardware prepared for pots + joystick that are not integrated yet

The best commercial reference to guide the next implementation steps is **Korg minilogue xd**.
