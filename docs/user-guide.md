# Yottasynth User Guide

## Overview

Yottasynth is designed as a compact instrument panel rather than a menu-heavy device. The touch screen handles navigation and discrete actions, while the five panel knobs always edit the five values shown on the current page.

The current playing workflow is:

1. connect a MIDI controller through USB host MIDI
2. choose the page you want from the bottom tab bar
3. use the five knobs for the currently visible parameters
4. use the touch buttons for page actions
5. use the `OK` button when the interface asks for confirmation

## Main Controls

### Touch screen

The touch screen is the main navigation surface.

Use it to:

- switch pages with the bottom tabs
- press action buttons
- select sequencer steps

### Five knobs

The five knobs are always active, but their meaning changes with the current page.

### `OK` button

Use this button only when the instrument asks you to confirm an action.

- `OK / Confirm`

Example:

- confirm sequencer clear when the screen is waiting for confirmation

## Screen Layout

The screen is split into three parts:

- top bar: page name and current status
- center panel: parameter cards, action buttons, or sequencer steps
- bottom bar: eight page tabs

The tabs are:

- `PLAY`
- `OSC`
- `FILT`
- `MOD`
- `FX`
- `ARP`
- `SEQ`
- `SET`

## Pages

### `PLAY`

Use this page for quick sound and transport changes while performing.

Knobs:

- `CUT`
- `RES`
- `GLIDE`
- `A-GATE`
- `BPM`

Touch buttons:

- `RUN/STOP`
- `ARP TOG`
- `PANIC`

## `OSC`

Use this page to shape the source sound.

Knobs:

- `OSC1`
- `OSC2`
- `NOISE`
- `OCT`
- `DETUNE`

Touch buttons:

- `W1 <`
- `W1 >`
- `W2 <`
- `W2 >`

This page balances the oscillators and noise, sets octave and detune, and changes waveforms through the touch controls.

## `FILT`

Use this page for brightness and envelope shape.

Knobs:

- `CUT`
- `RES`
- `ATT`
- `DEC`
- `REL`

Touch buttons:

- `SUS -`
- `SUS +`
- `SNAP`
- `LONG`

`SNAP` and `LONG` act like quick envelope presets.

## `MOD`

Use this page to control the LFO and expressive movement.

Knobs:

- `RATE`
- `DEPTH`
- `GLIDE`
- `BEND`
- `TARGET`

Touch buttons:

- `LFO OFF`
- `FILT LFO`
- `PITCH LFO`
- `DEPTH 0`

## `FX`

Use this page to choose and adjust the active effect.

Touch buttons:

- `BYPASS`
- `ECHO`
- `REVERB`
- `DIRT`

The knobs change meaning depending on the selected effect mode.

### Echo mode

Knobs:

- `MIX`
- `TIME`
- `FDBK`
- `RATIO`
- `SMEAR`

### Reverb mode

Knobs:

- `MIX`
- `SIZE`
- `DAMP`
- `PRE`
- `TONE`

### Drive mode

Knobs:

- `MIX`
- `DRIVE`
- `TONE`
- `CRUSH`
- `LEVEL`

## `ARP`

Use this page to turn held notes into repeating rhythmic patterns.

Knobs:

- `BPM`
- `DIV`
- `GATE`
- `OCT`
- `MODE`

Touch buttons:

- `ENABLE`
- `LATCH`
- `RUN/STOP`
- `CLR HELD`

Typical flow:

1. enable the arp
2. set BPM, division, gate, and mode
3. hold notes on the MIDI controller
4. use latch if you want the pattern to keep running after note release

## `SEQ`

Use this page to create and edit a 16-step monophonic sequence.

Knobs:

- `BPM`
- `LEN`
- `SWING`
- `NOTE`
- `GATE`

Touch buttons:

- `RUN/STOP`
- `REC ARM`
- `BANK`
- `CLEAR`

The sequencer page replaces the normal pot-card view with a step grid:

- eight visible step buttons at a time
- bank 1 shows steps `1-8`
- bank 2 shows steps `9-16`

Step interaction:

1. tap a step once to select it
2. tap the same selected step again to toggle it on or off
3. turn the `NOTE` knob to set its pitch
4. turn the `GATE` knob to set its gate length
5. use `BANK` to reach the other half of the pattern

### Recording into the sequencer

When `REC ARM` is active, incoming MIDI notes can be written into the currently playing step while the transport runs.

### Clearing the sequence

`CLEAR` does not erase immediately.

Current flow:

1. press `CLEAR`
2. the UI waits for confirmation
3. press the `OK` button to confirm

## `SET`

Use this page for global settings.

Knobs:

- `VOL`
- `TUNE`

This page is also where you choose the tuning system.

## Persian Scales and Tunings

Yottasynth includes alternate tunings with Persian music inspired scales.

Available tuning names:

- `Standard`
- `Shur`
- `Abuata`
- `Dashti`
- `Bayat-e Tork`
- `Afshari`
- `Segah`
- `Chahargah`
- `Homayun`
- `Bayat-e Esfahan`
- `Nava`
- `Mahur`
- `Rast-Panjgah`

To change tuning:

1. open `SET`
2. turn the `TUNE` knob
3. stop on the tuning you want

## Syncing to an External Clock

By default the transport runs on its own internal BPM. To slave the sequencer and
arpeggiator to an external MIDI clock instead:

1. open `SET`
2. turn the `SYNC` knob to `EXT`
3. connect the clock source over USB:
   - a standalone device (e.g. Ableton Move) into the USB host port, through a USB
     hub alongside your keyboard, or
   - a computer / DAW (e.g. Ableton Live) over the same USB cable used to upload firmware
4. start playback on the external device

When synced, the top status bar shows `EXT` (or `EXT?` while no clock is arriving),
the displayed BPM follows the incoming clock, and the external device's start /
stop / continue controls drive the transport. Set `SYNC` back to `INT` to return
to the internal clock.

## Quick Start

1. power the instrument
2. connect a MIDI controller through USB host MIDI
3. go to `PLAY` and confirm you have sound
4. adjust oscillator, filter, and modulation pages
5. add echo, reverb, or drive from `FX`
6. use `ARP` for repeating patterns
7. use `SEQ` to build a 16-step phrase
8. switch to `SET` if you want a Persian tuning or a different output volume

## Current Limits

From a user perspective, these are the main current limits:

- the synth is monophonic
- the sequencer is monophonic
- sequence clear uses confirm, but broader preset management is not implemented yet
