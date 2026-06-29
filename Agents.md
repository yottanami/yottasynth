# Yottasynth Agent Notes

## Keep Docs In Sync (read first)

Whenever you change firmware behavior, the control surface, pages, knob/touch
mappings, or hardware/wiring, update the documentation in the **same change**.
Do not treat docs as a follow-up. The files to check every time:

- `README.md` — features, page summaries, "How It Works"
- `Agents.md` (this file) — panel/page model, status bar, control maps
- `docs/user-guide.md` — user-facing workflow and current limits
- `docs/codebase-guide.md` — module/data-model/flow descriptions
- `docs/source-reading-guide.md` — struct snippets and reading order

If a feature removes a stated limitation (e.g. "X is not exposed"), delete that
line rather than leaving it stale. After firmware edits, grep the docs for the
feature you touched and reconcile any mentions before finishing.

## Scope

Treat the instrument as a **touch-first panel with hardware assist controls**.

That means:

- the touch screen is the main navigation surface
- the five pots are always active and remap to the current page
- the joystick push button is reserved for confirm-style actions, not general navigation
- the UI should feel like one instrument panel split into focused pages, not like a generic settings menu

The current firmware already follows that model. This document should stay focused on the actual panel structure and visual interaction model.

## Platform Context

The current implementation runs on:

- `Teensy 4.1`
- `SGTL5000` audio codec
- `320x240` SPI TFT touch display
- `74HC4067` mux for panel controls

Keep these platform constraints in mind for future work:

- RAM and CPU limits are those of a Teensy 4.1 class embedded target
- the UI is built for a fixed `320x240` touch screen
- the control surface is five knobs plus one joystick push button
- hardware-facing details beyond the panel map live in the codebase and board files, not only in this document

## Current Hardware Control Map

Source of truth note:

- the active mux and panel mapping in code is the valid mapping
- board design documents may still reflect an older mux wiring layout
- if there is any conflict, follow the current firmware implementation
- end-user docs should not include mux-channel or wiring details; keep those details here and in the firmware source

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

- most pages show BPM, clock source (`INT` / `EXT` / `EXT?`), transport state, arp state, and sequencer state
- `OSC / MIX` shows oscillator wave summary and noise amount
- `FX` shows effect mode and enabled/bypassed state
- `SETTINGS` shows output volume, current tuning, and clock source
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
- `SYNC` (clock source: `INT` / `EXT`)

Touch actions:

- `TEST PAGE`

Unlike the other pages, settings intentionally collapses to a few active pot cards and one service entry point.

Clock source notes:

- `INT` runs the transport on the internal BPM; `EXT` slaves the sequencer and arpeggiator to an external MIDI clock over USB
- external clock is received on both the host-side `MIDIDevice` instances (keyboard + a device through a hub) and the device-side `usbMIDI` (a DAW over the native USB port)
- this is firmware-only; no PCB change is required because the Teensy 4.1 already exposes both a USB host port and a USB-MIDI device port

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

## Bottom Line

The current firmware behaves like a compact instrument panel with:

- eight touch pages
- five context-sensitive knobs
- one confirm button
- color-coded tabs
- per-page pot cards
- page-specific touch actions
- a dedicated sequencer grid view

Future documentation or agent guidance should keep describing that implemented panel model directly.
