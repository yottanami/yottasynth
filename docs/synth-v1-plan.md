# Yottasynth Synth V1

## Summary

The current firmware implements the first integrated Yottasynth instrument shell on Teensy 4.1:

- mono synth engine
- six-page touch UI
- five context-sensitive pots
- joystick push-button as `OK / Confirm`
- internal BPM transport
- arpeggiator
- 16-step monophonic sequencer with live MIDI record

This document is the practical runtime reference for the current implementation.

## Hardware Control Map

- mux select pins: `14 / 15 / 16 / 17`
- mux signal pin: `22`
- knob 1: `C2`
- knob 2: `C4`
- knob 3: `C1`
- knob 4: `C5`
- knob 5: `C0`
- joystick push button: `C7`
- touch screen: primary navigation and detailed editing

Joystick axes are intentionally unused in v1.

## UI Structure

- `PLAY`
  - pots: cutoff, resonance, glide, arp gate, BPM
  - touch actions: run/stop, arp toggle, seq toggle, panic
- `OSC / MIX`
  - pots: osc1 mix, osc2 mix, noise mix, octave, detune
  - touch actions: octave down, octave up, detune reset, noise off
- `FILTER / AMP`
  - pots: cutoff, resonance, attack, decay, release
  - touch actions: sustain down, sustain up, short envelope preset, long envelope preset
- `MOD`
  - pots: LFO rate, LFO depth, glide, bend range, LFO target
  - touch actions: LFO off, filter target, pitch target, zero depth
- `ARPEGGIATOR`
  - pots: BPM, division, gate, octave range, mode
  - touch actions: enable, latch, run/stop, clear held notes
- `SEQUENCER`
  - pots: BPM, pattern length, swing, selected-step note, selected-step gate
  - touch actions: run/stop, record arm, bank switch, clear pattern
  - touch step pads: select a step on first tap, toggle active state on second tap

The `OK` button confirms pattern clear when the sequencer page is waiting for confirmation.

## Engine Behavior

- synth voice is mono with last-note priority
- pitch bend is active from USB host MIDI
- arpeggiator and sequencer share the same internal transport
- sequencer playback takes ownership of generated notes while running
- live MIDI record writes incoming notes into the current sequencer playhead step when record is armed
- LVGL timing uses elapsed `millis()`
- the LVGL draw buffer lives in `DMAMEM` to stay within Teensy RAM1 limits

## Known Limits

- no preset save/load yet
- no joystick axis behavior yet
- no external MIDI clock sync yet
- no effects yet
- sequencer is monophonic and does not expose tie editing in the UI yet

## Validation

Current verification:

- `pio run` passes for `teensy41`

Recommended on-device checks:

- verify all five knobs follow the expected channel order
- confirm `C7` is the correct joystick push-button input on hardware
- verify touch hit targets feel comfortable on the physical panel
- listen for zipper noise or over-sensitive smoothing on fast knob moves
