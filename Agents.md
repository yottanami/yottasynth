# Yottasynth Agent Notes

## Goal

This repository is building a Teensy 4.1 based digital instrument with:

- Teensy 4.1
- Teensy Audio Shield / SGTL5000 codec
- 320x240 SPI TFT touch screen
- 5 potentiometers
- 1 joystick push button used as `OK / Confirm`
- internal arpeggiator and sequencer

This document is the implementation-facing reference for how the current firmware works and how the interface and panel should be understood.

## Product Direction

Treat the instrument as a **touch-first panel with hardware assist controls**.

That means:

- the touch screen is the main navigation surface
- the five pots are always active and remap to the current page
- the joystick push button is reserved for confirm-style actions, not general navigation
- the UI should feel like one instrument panel split into focused pages, not like a generic settings menu

The current firmware already follows that model. Future work should extend it, not replace it with a desktop-style menu tree.

## Current Hardware Control Map

From the repo and active code:

- MCU: `Teensy 4.1`
- firmware environment: `PlatformIO` + `Arduino`
- display stack: `ILI9341` via `TFT_eSPI`
- touch controller: `XPT2046`
- audio codec: `AudioControlSGTL5000`
- control expansion: `74HC4067` analog multiplexer

### Display and touch wiring

- TFT MOSI: `11`
- TFT MISO: `12`
- TFT SCLK: `13`
- TFT CS: `3`
- TFT DC: `2`
- touch CS: `5`
- touch IRQ: `4`
- resolution: `320x240`

### Mux and panel controls

- mux select pins: `14 / 15 / 16 / 17`
- mux signal pin: `22`
- knob 1: `C2`
- knob 2: `C4`
- knob 3: `C1`
- knob 4: `C5`
- knob 5: `C0`
- joystick push button: `C7`

Joystick axes are intentionally unused in the current implementation.

## Interface Layout

The implemented UI is a fixed three-band layout on the `320x240` screen:

- top status bar: `46px`
- main content panel: `150px`
- bottom tab bar: `44px`

### Top status bar

The top bar should always show:

- current page title
- one compact status line relevant to the active page

Current behavior:

- most pages show BPM, transport state, arp state, and sequencer state
- `OSC / MIX` shows oscillator wave summary and noise amount
- `FX` shows effect mode and enabled/bypassed state
- `SETTINGS` shows output volume and current tuning
- input-test mode shows service text instead of musical status

### Main content panel

The default content layout is:

- one row of five pot cards at the top
- four large touch action buttons below

Each pot card shows:

- short parameter name
- current value

This is the core panel metaphor of the instrument. The screen should always make it clear what the five physical knobs currently do.

### Bottom tab bar

The implemented tabs are:

- `PLAY`
- `OSC`
- `FILT`
- `MOD`
- `FX`
- `ARP`
- `SEQ`
- `SET`

Each page has its own accent color. That color coding is part of the panel identity and should stay consistent if the UI grows.

## Page Model

The firmware currently implements eight pages.

### `PLAY`

Purpose:

- quick performance access without leaving the main play surface

Knobs:

- `CUT`
- `RES`
- `GLIDE`
- `A-GATE`
- `BPM`

Touch actions:

- `RUN/STOP`
- `ARP TOG`
- unused slot
- `PANIC`

This page should remain the fastest performance page, with no dense editing.

### `OSC / MIX`

Purpose:

- source balance and coarse oscillator shape changes

Knobs:

- `OSC1`
- `OSC2`
- `NOISE`
- `OCT`
- `DETUNE`

Touch actions:

- `W1 <`
- `W1 >`
- `W2 <`
- `W2 >`

Waveform changes are intentionally touch-based while the five knobs handle continuous values.

### `FILTER / AMP`

Purpose:

- amplitude and brightness shaping

Knobs:

- `CUT`
- `RES`
- `ATT`
- `DEC`
- `REL`

Touch actions:

- `SUS -`
- `SUS +`
- `SNAP`
- `LONG`

This page mixes direct ADSR editing with quick envelope presets, which is a good fit for the current panel size.

### `MOD`

Purpose:

- lightweight modulation editing without deep routing

Knobs:

- `RATE`
- `DEPTH`
- `GLIDE`
- `BEND`
- `TARGET`

Touch actions:

- `LFO OFF`
- `FILT LFO`
- `PITCH LFO`
- `DEPTH 0`

The modulation model is intentionally shallow and should stay easy to read from the screen.

### `FX`

Purpose:

- one focused effect page with mode-specific knob labels

Modes:

- `ECHO`
- `REVERB`
- `DRIVE`

Touch actions:

- `BYPASS`
- `ECHO`
- `REVERB`
- `DIRT`

The five pot labels change with the selected effect mode:

- echo: `MIX`, `TIME`, `FDBK`, `RATIO`, `SMEAR`
- reverb: `MIX`, `SIZE`, `DAMP`, `PRE`, `TONE`
- drive: `MIX`, `DRIVE`, `TONE`, `CRUSH`, `LEVEL`

This is the right pattern for pages that cannot fit every parameter at once: keep the page stable and retitle the five knob cards.

### `ARPEGGIATOR`

Purpose:

- transport-linked pattern performance

Knobs:

- `BPM`
- `DIV`
- `GATE`
- `OCT`
- `MODE`

Touch actions:

- `ENABLE`
- `LATCH`
- `RUN/STOP`
- `CLR HELD`

The arp is part of the main instrument flow, not a hidden utility.

### `SEQUENCER`

Purpose:

- direct step entry and transport control

Knobs:

- `BPM`
- `LEN`
- `SWING`
- `NOTE`
- `GATE`

Touch actions:

- `RUN/STOP`
- `REC ARM`
- `BANK`
- `CLEAR`

Sequencer-specific content replaces the pot-card row with:

- one step info label
- eight large step buttons for the visible bank

Interaction rules:

- first tap on a step selects it
- tapping the selected step toggles it active/inactive
- `BANK` switches between steps `1-8` and `9-16`
- `CLEAR` enters a confirmation state
- joystick `OK` confirms the clear operation

This is the most important special-case panel in the current UI. It should stay tactile and obvious rather than becoming list-driven.

### `SETTINGS`

Purpose:

- small number of global controls without turning the synth into a menu maze

Knobs:

- `VOL`
- `TUNE`

Touch actions:

- `TEST PAGE`

Unlike the other pages, settings intentionally collapses to two active pot cards and one service entry point.

### Input test overlay

The input-test screen is a service/debug overlay, not a separate product mode. It temporarily replaces the content panel and is entered from `SETTINGS`.

## Panel Behavior

### Knob model

The five knobs are active in normal runtime.

Current behavior:

- values are read through the mux
- analog reads use smoothing
- changes are only emitted after a movement threshold
- each page maps the same physical knob order to a different parameter set

This is the correct panel model for the current hardware. Documentation and future features should assume context-sensitive knobs, not fixed one-function-per-knob labeling.

### Touch model

Touch input is stabilized in firmware before LVGL receives it.

Current behavior:

- press confirmation requires repeated stable samples
- move confirmation also requires stable samples
- release uses its own threshold
- raw coordinates are calibrated into screen coordinates

This means the touch UI is designed for deliberate finger interaction, not stylus-like precision. Buttons should stay large and clearly separated.

### Confirm model

The joystick push button is currently dedicated to `OK / Confirm`.

Implemented use today:

- confirms sequencer clear after the UI shows `WAIT OK`

Future destructive actions should reuse this pattern instead of adding tiny touch-only confirmation dialogs.

## Runtime Behavior

### Startup flow

Current boot flow is:

1. initialize LVGL
2. initialize TFT and touch
3. create the LVGL display and input device
4. render the main menu
5. start control input scanning
6. initialize audio
7. initialize synth
8. initialize USB-host MIDI
9. start the performance engine

### Audio and synth behavior

The active instrument is currently:

- one monophonic synth voice with last-note priority
- dual oscillator plus noise
- filter and ADSR envelope
- LFO with filter or pitch target
- pitch bend
- output volume control
- selectable FX state for echo, reverb, and drive

The larger audio graph still contains extra lanes beyond the active lead voice, but the playable instrument is centered on the lead path.

### MIDI and transport behavior

Current live behavior:

- note input comes from `USBHost_t36`
- pitch bend is active
- arp and sequencer share the same internal transport
- sequencer can live-record MIDI notes into the active step while record is armed
- transport is currently internal-clock only

### LVGL timing

The firmware now advances LVGL using elapsed `millis()` time, not a fake fixed increment. The draw buffer lives in `DMAMEM` to fit comfortably on Teensy 4.1.

## Practical UI Rules

Use these rules when extending the instrument:

- keep the current three-band screen structure
- always show the active meaning of the five knobs
- prefer page-specific panels over deep navigation
- keep touch targets large enough for finger use
- use touch for selection and discrete actions
- use pots for continuous editing
- reserve the joystick push button for confirmations and safe commits
- keep color accents page-specific and stable
- treat the sequencer as a hands-on grid, not a text editor

## Current Gaps

The current implementation is already functional, but these limits are still real:

- no preset save/load yet
- joystick axes still unused
- no external MIDI clock sync yet
- sequencer tie state exists in data but is not surfaced in the current UI
- the broader multi-lane audio graph is not yet exposed as a multi-part instrument

## Next Engineering Steps

1. Add preset storage for patch, FX, and sequence state.
2. Expose sequencer tie and rest editing without making the step page crowded.
3. Decide whether joystick axes should get a dedicated performance role or remain unused.
4. Add external MIDI clock sync only after the internal transport workflow feels solid on hardware.
5. Expand the instrument only through the existing page-based panel model.

## Bottom Line

The current firmware is no longer a placeholder menu. It already behaves like a compact instrument panel with:

- eight touch pages
- five context-sensitive knobs
- one confirm button
- a mono synth engine
- active FX
- an arpeggiator
- a 16-step sequencer

Any future documentation or agent guidance should describe and extend that implemented panel model directly, without leaning on external synthesizer examples.
