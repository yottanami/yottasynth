# Yottasynth Synth V1

## Summary

This document is the practical reference for the current panel structure and touch UI.

The implemented interface provides:

- eight tabs
- five context-sensitive pots
- joystick push-button as `OK / Confirm`
- one main panel area that changes by page
- a dedicated sequencer grid view

## Hardware Control Map

Source of truth note:

- the active mux and panel mapping in code is the valid mapping
- board design documents may still reflect an older mux wiring layout
- if there is any conflict, follow the current firmware implementation

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
  - touch actions: run/stop, arp toggle, unused slot, panic
- `OSC / MIX`
  - pots: osc1 mix, osc2 mix, noise mix, octave, detune
  - touch actions: osc1 previous wave, osc1 next wave, osc2 previous wave, osc2 next wave
- `FILTER / AMP`
  - pots: cutoff, resonance, attack, decay, release
  - touch actions: sustain down, sustain up, short envelope preset, long envelope preset
- `MOD`
  - pots: LFO rate, LFO depth, glide, bend range, LFO target
  - touch actions: LFO off, filter target, pitch target, zero depth
- `FX`
  - pots: mode-dependent effect parameters
  - touch actions: bypass, echo, reverb, drive
- `ARPEGGIATOR`
  - pots: BPM, division, gate, octave range, mode
  - touch actions: enable, latch, run/stop, clear held notes
- `SEQUENCER`
  - pots: BPM, pattern length, swing, selected-step note, selected-step gate
  - touch actions: run/stop, record arm, bank switch, clear pattern
  - touch step pads: select a step on first tap, toggle active state on second tap
- `SETTINGS`
  - pots: output volume, tuning
  - touch actions: input test page

The `OK` button confirms pattern clear when the sequencer page is waiting for confirmation.

## Interaction Rules

- the five pots stay active and remap by page
- the touch screen is the main navigation and action surface
- the joystick `OK` button is reserved for confirmations such as sequence clear
- sequencer step buttons use first tap to select and second tap to toggle
- page accent colors should stay stable across the whole interface
