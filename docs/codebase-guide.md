# Yottasynth Codebase Guide

This document explains how the current firmware is structured, how the main modules work, and how the code is organized for someone who is new both to the project and to embedded C++.

It focuses on the active firmware path:

- [`src/`](../src/)
- [`include/`](../include/)

It also points out where the repository still contains older prototype code.

## 1. What This Firmware Does

At a high level, the firmware turns a Teensy-based hardware instrument into a touch-first mono synthesizer with:

- USB host MIDI input
- a five-knob control surface through a multiplexer
- a touchscreen interface built with LVGL
- a mono synth voice with two oscillators, noise, filter, envelope, glide, and LFO
- a transport shared by an arpeggiator and step sequencer
- three effect modes: echo, reverb, and drive
- selectable Persian/Iranian-inspired tuning tables

The main idea is simple:

1. read physical inputs
2. update application state
3. render the UI from that state
4. drive the synth engine from that state
5. repeat forever

## 2. Current Architecture

The active runtime looks like this:

```text
Hardware inputs
  |- touch controller
  |- five pots through 74HC4067 mux
  |- joystick push button through mux
  |- USB MIDI device
  v
Input and event layers
  |- main.cpp touch callback
  |- ControlInput
  |- PlayMode
  v
Shared state and control logic
  |- AppState
  |- Settings
  |- MainMenu
  |- PerformanceEngine
  v
Audio layers
  |- Synth
  |- audio_setup.cpp audio graph + effects
  v
Audio codec / I2S output
```

Two design choices define the whole project:

1. `AppState` is the central shared data model.
2. `main.cpp` coordinates subsystems in a single cooperative loop.

## 3. Repository Structure

### Active firmware

- [`src/main.cpp`](../src/main.cpp)
  Boot sequence, LVGL setup, touchscreen input bridge, main runtime loop.

- [`src/app_state.cpp`](../src/app_state.cpp) and [`include/app_state.h`](../include/app_state.h)
  Central application state and label helpers.

- [`src/control_input.cpp`](../src/control_input.cpp) and [`include/control_input.h`](../include/control_input.h)
  Multiplexer-based knob and confirm-button scanning.

- [`src/play_mode.cpp`](../src/play_mode.cpp) and [`include/play_mode.h`](../include/play_mode.h)
  USB host MIDI stack and MIDI event registration.

- [`src/performance_engine.cpp`](../src/performance_engine.cpp) and [`include/performance_engine.h`](../include/performance_engine.h)
  Transport, arpeggiator, sequencer, held notes, and generated note timing.

- [`src/synth.cpp`](../src/synth.cpp) and [`include/synth.h`](../include/synth.h)
  Voice logic and mapping from patch state to Teensy Audio objects.

- [`src/audio_setup.cpp`](../src/audio_setup.cpp) and [`include/audio_setup.h`](../include/audio_setup.h)
  Audio graph definition, codec initialization, and effect routing.

- [`src/main_menu.cpp`](../src/main_menu.cpp) and [`include/main_menu.h`](../include/main_menu.h)
  LVGL panel UI and page-specific control mapping.

- [`src/input_test_page.cpp`](../src/input_test_page.cpp) and [`include/input_test_page.h`](../include/input_test_page.h)
  Hardware diagnostics page for touch, mux, direct analog pins, audio self-test, and MIDI status.

- [`include/settings.h`](../include/settings.h) and [`src/settings.cpp`](../src/settings.cpp)
  Small global mode tracker.

- [`include/mux_pins.h`](../include/mux_pins.h)
  Shared multiplexer pin assignments.

- [`include/lv_conf.h`](../include/lv_conf.h)
  LVGL compile-time configuration.

### Documentation and hardware assets

- `docs/`
  Human documentation.

- `etc/`
  Board pictures and KiCad design files.

### Legacy prototype code

- [`lib/synthesizer/`](../lib/synthesizer/)
  Older synth prototype using a different architecture.

- [`lib/ui/`](../lib/ui/)
  Older menu prototype.

- [`lib/settings.h`](../lib/settings.h)
  Older settings singleton that matches the prototype code.

These `lib/` files are useful as history, but they are not the current implementation path described by `src/main.cpp`.

## 4. Build And Platform Context

The build configuration is defined in [`platformio.ini`](../platformio.ini).

Important points:

- target board: `teensy41`
- framework: `arduino`
- upload protocol: `teensy-cli`
- UI libraries: `lvgl`, `TFT_eSPI`, `XPT2046_Touchscreen`
- USB MIDI serial support enabled by `-D USB_MIDI_SERIAL`

From a code-reading perspective, this tells you:

1. the runtime model is Arduino-style
2. graphics are handled by LVGL
3. the screen is an SPI TFT
4. the touch controller is separate from the display
5. the audio path uses the Teensy Audio ecosystem

## 5. Startup Flow

The current boot sequence lives in [`src/main.cpp`](../src/main.cpp).

### 5.1 Global objects created before `setup()`

Several long-lived objects are created at file scope:

- touchscreen driver
- TFT display driver
- LVGL draw buffer
- `play_mode`
- `synth`
- other singleton/global modules in their own `.cpp` files

This is common in embedded firmware. The project assumes there is one instrument and one set of hardware services.

### 5.2 `setup()`

`setup()` performs these steps:

1. starts serial output
2. initializes LVGL
3. initializes the TFT display
4. initializes the XPT2046 touch controller
5. creates LVGL display and input devices
6. renders the main UI with `main_menu.render()`
7. starts control-input scanning with `control_input.begin()`
8. initializes the audio graph with `setupAudio()`
9. stores audio status in `AppState`
10. initializes the synth voice with `synth.setup()`
11. applies initial output volume
12. starts USB host MIDI with `play_mode.setup()`
13. gives the synth pointer to `performance_engine.begin(&synth)`

This is the system assembly phase. After `setup()`, all major services are alive.

### 5.3 `loop()`

`loop()` runs forever and does the following:

1. refreshes transient MIDI status in `AppState`
2. checks the high-level `Mode` from `Settings`
3. if input-test mode is active:
   runs the diagnostic page loop
4. otherwise:
   updates knob/button input
   sends pot changes into `MainMenu`
   sends confirm-button presses into `MainMenu`
   polls USB MIDI
   updates transport/arpeggiator/sequencer timing
   applies patch and effects to audio
   runs synth smoothing/modulation logic
   updates audio status in `AppState`
5. listens for serial `!` to reboot into bootloader
6. lets `MainMenu` refresh the UI if needed
7. advances LVGL ticks and timers

This is a cooperative system. No module owns the CPU for long. Each module does a small amount of work and returns quickly.

## 6. Shared State Model

The central state type is declared in [`include/app_state.h`](../include/app_state.h).

### 6.1 Why `AppState` exists

Instead of letting each module keep its own disconnected state, the firmware keeps most instrument state in one place:

- patch parameters
- transport state
- arpeggiator state
- effect state
- sequencer data
- UI state
- MIDI status
- audio status

This makes the UI, performance logic, and audio logic coordinate through shared data instead of directly calling each other all the time.

### 6.2 Main structs inside `AppState`

#### `PatchState`

Contains the sound-design parameters:

- oscillator mix
- oscillator waveforms
- noise mix
- oscillator octave offset
- detune
- filter cutoff and resonance
- ADSR envelope
- LFO rate and depth
- LFO target
- glide
- pitch-bend range
- tuning selection

This is the heart of the sound patch.

#### `TransportState`

Contains tempo and step timing state:

- BPM
- whether the transport is running
- swing amount
- current step index

This transport is shared by both the sequencer and arpeggiator.

#### `ArpState`

Contains:

- enabled flag
- latch flag
- arp mode
- octave range
- rhythmic division
- gate percentage

#### `FxState`

Contains:

- enabled flag
- current effect mode
- per-mode parameter blocks for echo, reverb, and drive

#### `SequencerState`

Contains:

- enabled flag
- record-arm flag
- pattern length
- current playhead
- selected step
- visible bank
- `SequenceStep steps[16]`

Each `SequenceStep` stores:

- `active`
- `tie`
- `note`
- `gate`

The `tie` field exists but is not yet actively used by the current step-playback logic.

#### `UiState`

Contains:

- current page
- dirty flag
- whether the input-test page is being shown
- whether the sequencer clear action is waiting for confirmation

The dirty flag is important. It tells the UI when it must redraw instead of constantly rebuilding labels every loop.

#### `MidiStatus`

Contains:

- whether a USB MIDI device is connected
- vendor and product IDs
- last note and velocity
- recent-note timing info for transient status display

#### `AudioStatus`

Contains:

- whether the codec started successfully
- whether audio self-test is currently active
- output volume

### 6.3 `AppState` helper methods

Important methods:

- `setPage`
  Changes page and clears transient UI flags.

- `setInputTestVisible`
  Shows or hides diagnostic mode inside the UI.

- `markDirty`
  Requests a UI refresh.

- `currentMode`
  Converts UI page/test state into a higher-level `Mode`.

- `updateMidiDevice`
  Updates MIDI connection info and marks the UI dirty when needed.

- `registerMidiNote`
  Stores the most recent incoming note for status display.

- `refreshTransientStatus`
  Clears “recent note” status after a timeout.

- `updateAudioStatus`
  Keeps codec and self-test status in sync.

- `setOutputVolume`
  Clamps and stores volume in normalized form.

### 6.4 Why there is also a `Settings` singleton

`Settings` is a much smaller singleton that only tracks the current high-level mode:

- `MENU`
- `SYNTHESIZER`
- `SEQUENCER`
- `ARPEGGIATOR`
- `INPUT_TEST`

In the current code, `MainMenu` writes this mode by asking `AppState::currentMode()`.

You can think of `Settings` as a lightweight “what subsystem loop branch should run right now?” flag.

## 7. Input System

The firmware has three main input sources:

1. touch screen
2. multiplexed analog controls
3. USB MIDI

### 7.1 Touch input in `main.cpp`

The touch handling in [`src/main.cpp`](../src/main.cpp) is more than a thin hardware wrapper.

It adds filtering and confirmation rules:

- raw touches must be inside calibrated ranges
- pressure thresholds are used
- touches must be stable for more than one sample before being accepted
- movement is also filtered before a new touch point is committed

This is done to avoid noisy touch behavior on the resistive touch controller.

Key helper functions:

- `touchPointInRange`
- `touchPointsClose`
- `storePendingTouch`
- `commitTouchSample`
- `holdLastTouch`
- `releaseTouch`

The LVGL input callback is `my_touchpad_read`. It translates a raw `TS_Point` into:

- `LV_INDEV_STATE_PRESSED` or `LV_INDEV_STATE_RELEASED`
- screen coordinates in the `320x240` display space

The same touch information is also forwarded to `input_test_page.updateTouch(...)` so the diagnostic page can display raw and mapped touch data.

### 7.2 Knobs and OK button in `ControlInput`

[`src/control_input.cpp`](../src/control_input.cpp) reads the five knobs and joystick push button through a `74HC4067` multiplexer.

Important implementation details:

- select pins come from [`include/mux_pins.h`](../include/mux_pins.h)
- ADC resolution is set to 12 bits
- each channel is oversampled and averaged
- a smoothing filter reduces jitter
- a threshold decides whether a pot change is worth reporting
- the button is debounced in software

#### Pot mapping

The active runtime mapping is:

- knob 1 -> channel `C2`
- knob 2 -> channel `C4`
- knob 3 -> channel `C1`
- knob 4 -> channel `C5`
- knob 5 -> channel `C0`
- OK button -> channel `C7`

This mapping is defined by `ControlInput::kPotChannels` and `kButtonChannel`.

#### Important methods

- `begin`
  Configures mux pins and captures initial pot values.

- `update`
  Reads all controls, filters them, and records pending changes.

- `consumePotChange`
  Returns one pot change exactly once.

- `consumeOkPress`
  Returns one debounced confirm-button press exactly once.

This “consume” pattern is useful because it decouples scanning from action handling.

### 7.3 Hardware diagnostic mode in `InputTestPage`

[`src/input_test_page.cpp`](../src/input_test_page.cpp) provides a service/debug page inside the UI.

It can show:

- touch coordinates and raw pressure
- audio self-test state
- MIDI connection state
- live multiplexer values
- live direct analog pin values
- min/max movement ranges since last reset

It uses a slower, diagnostic-oriented scanning model than the normal control path.

Important details:

- it can start a short audio self-test by calling `synth.startSelfTest()`
- it resets activity ranges when requested
- it scans mux channels periodically with a timing budget
- it samples a set of direct analog pins separately

#### Important caution

The diagnostic page still contains some older “KiCad pot guess” labels and channel assumptions that do not fully match the active runtime control mapping in `ControlInput`.

For real instrument behavior, the source of truth is:

- [`include/control_input.h`](../include/control_input.h)
- [`src/control_input.cpp`](../src/control_input.cpp)

That means the input-test page is best understood as a hardware probing tool, not as the definitive live-control map.

## 8. MIDI Input Layer

The MIDI layer is implemented by [`src/play_mode.cpp`](../src/play_mode.cpp).

Despite the name, this file is primarily a USB host MIDI service.

### 8.1 What it creates

It creates global USB host stack objects:

- `USBHost myusb`
- two `USBHub` objects
- one `MIDIDevice midi1`

### 8.2 What `setup()` does

It:

1. waits briefly before enabling USB host power
2. starts the USB host stack
3. registers callback handlers for many MIDI message types

Only a few callbacks currently have real behavior:

- note on
- note off
- pitch bend

Most others are stubbed out with unused-parameter handling.

### 8.3 Important data flow

When MIDI note-on arrives:

1. `AppState` stores recent-note status
2. `PerformanceEngine::onMidiNoteOn(...)` is called

When MIDI note-off arrives:

1. `PerformanceEngine::onMidiNoteOff(...)` is called

When pitch bend arrives:

1. `PerformanceEngine::onMidiPitchBend(...)` is called

### 8.4 Polling model

`PlayMode::loop()` must be called continuously.

It performs:

- `myusb.Task()`
- `midi1.read()`
- `AppState::updateMidiDevice(...)`

This is an important embedded pattern: the callbacks are only triggered because the code actively polls the device in the main loop.

## 9. UI Layer

The UI is implemented in [`src/main_menu.cpp`](../src/main_menu.cpp).

This is the largest file in the active firmware because it combines:

- LVGL object creation
- screen layout
- page labels
- value formatting
- control mapping
- touch-action logic

### 9.1 UI layout

The screen has three bands:

- top status bar
- content area
- bottom tab bar

Inside the content area, the main page can show:

- five pot cards and four action buttons
- sequencer overview + eight step buttons
- or the input-test page

### 9.2 Why the UI uses a dirty flag

`MainMenu::loop()` only refreshes when `state.ui.dirty` is true.

That matters because:

- embedded display updates are relatively expensive
- most loop iterations do not need a full UI refresh

This pattern helps keep the UI responsive without redrawing everything every frame.

### 9.3 Main responsibilities

#### `render()`

Creates all LVGL widgets once:

- root container
- top bar
- content panel
- input-test panel
- pot cards
- action buttons
- sequencer buttons
- tab bar

This function mostly describes structure and styling.

#### `handlePotChange(index, value)`

Maps the five physical knobs to different parameters depending on the active page.

This is one of the central project behaviors. The same five physical controls mean different things on different pages.

#### `handleAction(action_index)`

Maps touch buttons to page-specific actions.

Examples:

- waveform selection on `OSC / MIX`
- LFO target selection on `MOD`
- effect mode selection on `FX`
- transport and record actions on `SEQ`

#### `handleOkPress()`

Currently only confirms sequence clear when the UI is waiting for `OK`.

#### `refresh*()` methods

These update different parts of the screen:

- status bar
- tabs
- pot cards
- action buttons
- sequencer buttons
- visibility of page-specific panels

### 9.4 Page logic

Each page has:

- five pot labels
- five pot value formatters
- up to four touch actions

That logic is distributed through:

- `potName`
- `formatPotValue`
- `refreshActionButtons`
- `handlePotChange`
- `handleAction`

This is a practical pattern for a fixed control surface. Instead of modeling every page as its own class, the project keeps one UI object and switches behavior by `PageId`.

### 9.5 Sequencer UI behavior

The sequencer page has custom behavior:

- eight step buttons are shown at a time
- `visible_bank` selects steps `1..8` or `9..16`
- first tap selects a step
- second tap on the selected step toggles its active state
- a cooldown prevents accidental multiple touch actions

The sequencer also has a two-step clear flow:

1. touch `CLEAR`
2. press the physical OK button

This is implemented with `state.ui.confirm_clear_sequence`.

### 9.6 Important implementation detail

The button labeled `PANIC` on the `PLAY` page currently calls `performance_engine.stopTransport()`.

That stops transport-driven playback, but it is not a full “all notes off everywhere” implementation in the current code.

## 10. Performance Logic Layer

The musical event scheduler lives in [`src/performance_engine.cpp`](../src/performance_engine.cpp).

This file answers questions such as:

- if a MIDI note comes in, should it play directly?
- should the arpeggiator consume it?
- should the sequencer record it?
- when should the next step happen?
- how long should a generated note stay on?

### 10.1 Core responsibilities

- remember held MIDI notes
- schedule transport steps
- run arpeggiator note generation
- run sequencer note generation
- manage note gate durations for generated notes
- forward note events to `Synth`

### 10.2 Transport timing

`stepIntervalUs(...)` computes the duration of one transport step in microseconds.

Important details:

- BPM is clamped to a minimum of `40`
- one step is treated as a sixteenth note
- swing alternates the duration of even and odd steps

This means the transport is grid-based and lightweight rather than sample-accurate or DAW-style complex.

### 10.3 Update loop

`PerformanceEngine::update()`:

1. syncs transport state changes
2. releases generated notes whose gate time expired
3. if transport is running:
   advances one or more due steps

The code includes a small safety limit so a slow loop iteration does not spend too long catching up.

### 10.4 MIDI note handling

#### Note-on

The code path depends on current state:

- if self-test is active:
  ignore musical input

- if sequencer is running and record-armed:
  record the note into the current step and also play it

- otherwise:
  add the note to held notes

- if arpeggiator is enabled:
  direct play is mostly suppressed while arp logic owns playback

- if sequencer is running:
  direct play is suppressed

- otherwise:
  play directly through `Synth`

#### Note-off

Similarly:

- remove from held notes
- possibly stop direct playback
- or ignore release if transport-owned playback is active

### 10.5 Arpeggiator behavior

The arpeggiator uses:

- held-note list
- mode
- division
- octave range
- gate percentage

Supported modes:

- `UP`
- `DOWN`
- `UP_DOWN`
- `RANDOM`

`nextArpNote()` chooses the next note and can add octave offsets based on the current transport step.

### 10.6 Sequencer behavior

Each active sequencer step can generate:

- note number
- fixed velocity for generated playback
- gate duration derived from step gate percentage

If a step is inactive, generated playback is cleared.

If record-arm is enabled and MIDI notes arrive while running, `recordStepFromMidi(...)` writes the incoming note into the current playhead step.

### 10.7 Generated notes

The arpeggiator and sequencer produce notes that are separate from physically held input notes.

The engine tracks:

- whether a generated note is active
- which note it is
- when its gate should end

This is why there are dedicated helpers:

- `triggerGeneratedNote`
- `releaseGeneratedNoteIfDue`
- `clearGeneratedNote`

## 11. Synth Voice Layer

The synth voice logic lives in [`src/synth.cpp`](../src/synth.cpp).

This file is the main bridge between `PatchState` and the Teensy Audio objects representing the lead voice.

### 11.1 Important idea

The current playable instrument is essentially a mono lead voice, even though the audio graph declares additional mid/bass/drum objects.

The active `Synth` object is constructed with:

- `lead_waveform1`
- `lead_waveform2`
- `lead_pink`
- `lead_filter`
- `lead_envelope`

So the current musical firmware uses the lead chain only.

### 11.2 What `setup()` does

It initializes:

- oscillator pulse widths
- oscillator waveforms
- lead mixer gains
- filter defaults
- envelope defaults
- current note and frequencies
- patch application

It also captures an initial `last_update_us_` timestamp used for glide/update calculations.

### 11.3 Patch application

`applyPatch(const PatchState &patch)` updates sound parameters on the audio objects.

It handles:

- waveform changes
- oscillator/noise mix
- filter frequency and resonance
- ADSR envelope values
- live voice refresh if a note is already sounding

Important design detail:

- UI and state store normalized values
- `Synth` converts them into real audio-domain values

Examples:

- cutoff is converted with an exponential mapping to hertz
- ADSR normalized values are mapped to milliseconds
- resonance is mapped into the filter’s expected range

### 11.4 Note stack behavior

The synth keeps a small note buffer of size `8`.

When a new note-on arrives:

- duplicate entries are removed
- the note is appended if there is room
- the newest note becomes the current note

When note-off arrives:

- the released note is removed
- if other notes remain, the most recent remaining note becomes active
- if none remain, the envelope is released

This is a mono last-note-priority design.

### 11.5 Glide

If glide is low, frequency jumps directly to the target.

If glide is higher, the current frequency moves gradually toward the target frequency in `loop()`.

This happens separately from note scheduling. The scheduler decides which note to play, and the synth decides how smoothly to move to it.

### 11.6 LFO behavior

The synth supports two LFO targets:

- filter
- pitch

The modulation is generated as a triangle shape.

Important implementation detail:

`updateModulation()` advances phase using a fixed `0.001f` seconds per loop iteration rather than the measured loop delta.

That means the LFO is simple and practical, but not mathematically tied to exact real elapsed time in the most precise way.

### 11.7 Tuning system

The file defines several cent tables:

- standard equal temperament
- Shur
- Abuata
- Afshari
- Segah
- Chahargah
- Homayun
- Bayat-e Esfahan
- Mahur
- Rast-Panjgah

Some tuning IDs intentionally reuse the same cent table in the current implementation.

`noteToFrequency(...)` works like this:

1. choose the cent table for the selected tuning
2. split MIDI note into note class and octave
3. measure relative to `C4 = 261.63 Hz`
4. compute the final frequency with `powf(2.0f, octave_ratio)`

This is one of the project’s most distinctive musical features.

### 11.8 Self-test mode

The synth can temporarily enter a self-test mode:

- both oscillators become sine waves
- fixed test frequencies are used
- the envelope is forced on
- the test auto-stops after about `1200 ms`

This is triggered by the input-test page and is useful for verifying that the audio path works.

## 12. Audio Graph And Effects

The audio graph is defined in [`src/audio_setup.cpp`](../src/audio_setup.cpp) with declarations mirrored in [`include/audio_setup.h`](../include/audio_setup.h).

### 12.1 Why the file is so large

Most of the file is generated object wiring from the Teensy Audio System Design Tool.

That includes:

- waveform sources
- noise sources
- filters
- mixers
- envelopes
- delay
- reverb
- waveshaper
- bitcrusher
- final stereo output

Generated audio wiring files are usually large because every connection is explicit.

### 12.2 Important practical interpretation

Although many voices are declared:

- lead
- mid
- bass
- drum

the current firmware actively drives the lead voice path only.

The rest of the audio graph is best understood as reserved or prototype structure that could support expansion later.

### 12.3 `setupAudio()`

This function:

1. allocates Teensy audio memory
2. enables the SGTL5000 codec
3. sets codec output levels
4. builds the drive waveshaper curve
5. initializes mixer balances
6. initializes delay times
7. initializes reverb defaults
8. initializes bitcrusher defaults

It returns `true` or `false` depending on whether the audio codec could be enabled.

### 12.4 Output volume

`setOutputVolume(float volume)` is a thin wrapper around `sgtl5000_1.volume(...)`.

The normalized `AppState` value remains the UI/state representation, while this function applies the real codec setting.

### 12.5 Effect application

`applyFxState(const FxState &fx)` is the effect-routing controller.

It:

- checks whether effects are initialized
- skips work if the effect state has not changed
- handles bypass behavior
- reconfigures routing and parameters for echo, reverb, or drive

#### Echo mode

Controls:

- wet/dry mix
- delay time
- feedback
- left/right ratio
- smear

Implementation techniques:

- two delay taps for stereo feel
- feedback mixer
- some bleed into the reverb input for smear

#### Reverb mode

Controls:

- wet/dry mix
- size
- damping
- predelay
- tone

Implementation techniques:

- delay channel used for predelay
- reverb fed with a tone-dependent balance
- different left/right wet levels

#### Drive mode

Controls:

- wet/dry mix
- drive amount
- tone
- bit crush
- level

Implementation techniques:

- pre-gain into waveshaper
- bitcrusher depth
- sample-rate reduction

### 12.6 Change caching

The function stores the last applied `FxState` and compares the new one with `memcmp(...)`.

That is a performance optimization to avoid reapplying the same settings every loop.

## 13. Page-By-Page Behavior

This section explains what each page changes in code.

### `PLAY`

Knobs:

- cutoff
- resonance
- glide
- arp gate
- BPM

Actions:

- toggle transport
- toggle arpeggiator
- unused button slot
- stop transport

### `OSC / MIX`

Knobs:

- oscillator 1 mix
- oscillator 2 mix
- noise mix
- octave index
- detune

Actions:

- previous/next waveform for oscillator 1
- previous/next waveform for oscillator 2

### `FILTER / AMP`

Knobs:

- cutoff
- resonance
- attack
- decay
- release

Actions:

- sustain down
- sustain up
- short envelope preset
- long envelope preset

### `MOD`

Knobs:

- LFO rate
- LFO depth
- glide
- bend range
- LFO target

Actions:

- LFO off
- filter LFO
- pitch LFO
- depth zero

### `FX`

Knobs depend on current effect mode.

Actions:

- bypass
- select echo
- select reverb
- select drive

### `ARP`

Knobs:

- BPM
- division
- gate
- octave range
- mode

Actions:

- enable
- latch
- toggle transport
- clear held notes

### `SEQ`

Knobs:

- BPM
- length
- swing
- selected step note
- selected step gate

Actions:

- run/stop
- record arm
- bank switch
- arm/confirm clear flow

### `SETTINGS`

Knobs:

- output volume
- tuning

Action:

- open input-test page

## 14. LVGL Configuration

[`include/lv_conf.h`](../include/lv_conf.h) is the LVGL configuration header.

You do not need to read it line by line to understand the firmware logic.

The important high-level facts are:

- color depth is `16`
- the default refresh period is `33 ms`
- LVGL logging is enabled
- the default font is `Montserrat 14`
- the memory pool is configured for embedded use

Treat this file as framework configuration rather than product logic.

## 15. Legacy Code In `lib/`

The repository still contains older prototype code.

### `lib/synthesizer`

This older synth implementation:

- uses a much simpler architecture
- handles MIDI directly inside the synth module
- has its own LFO implementation
- predates the current `AppState + MainMenu + PerformanceEngine + Synth` split

It is useful if you want to see the project’s earlier direction, but it is not the main firmware path now.

### `lib/ui`

This contains an older LVGL menu prototype based on a simpler page/menu model.

The current UI no longer uses this structure.

### Why these files matter

For maintenance, it is important to know they exist so you do not confuse:

- active implementation
- historical prototype

When documenting or modifying current behavior, prefer `src/` and `include/`.

## 16. File-By-File Quick Reference

### Core orchestration

- [`src/main.cpp`](../src/main.cpp)
  Boot, loop, display flush, touch filtering, high-level orchestration.

### State

- [`include/app_state.h`](../include/app_state.h)
  Type definitions for almost all live application state.

- [`src/app_state.cpp`](../src/app_state.cpp)
  State initialization and label/helper implementations.

- [`include/settings.h`](../include/settings.h)
  Small mode singleton.

- [`src/settings.cpp`](../src/settings.cpp)
  Defines the static singleton pointer.

### Hardware input

- [`include/mux_pins.h`](../include/mux_pins.h)
  Shared mux pin numbers.

- [`include/control_input.h`](../include/control_input.h)
  Public API for normal control scanning.

- [`src/control_input.cpp`](../src/control_input.cpp)
  Normal runtime scanning, filtering, and button debounce.

- [`include/input_test_page.h`](../include/input_test_page.h)
  Public API for diagnostic page.

- [`src/input_test_page.cpp`](../src/input_test_page.cpp)
  Diagnostic scanning and UI text generation.

### MIDI and scheduling

- [`include/play_mode.h`](../include/play_mode.h)
  MIDI service declarations.

- [`src/play_mode.cpp`](../src/play_mode.cpp)
  USB host MIDI setup and event callbacks.

- [`include/performance_engine.h`](../include/performance_engine.h)
  Scheduler/arpeggiator/sequencer declarations.

- [`src/performance_engine.cpp`](../src/performance_engine.cpp)
  Musical event timing and generated note logic.

### Sound engine

- [`include/synth.h`](../include/synth.h)
  Mono voice interface.

- [`src/synth.cpp`](../src/synth.cpp)
  Voice implementation, tuning tables, modulation, glide, self-test.

- [`include/audio_setup.h`](../include/audio_setup.h)
  Audio graph declarations and public audio helper functions.

- [`src/audio_setup.cpp`](../src/audio_setup.cpp)
  Audio graph instantiation, codec setup, and effect routing.

### UI

- [`include/main_menu.h`](../include/main_menu.h)
  Main UI class declaration.

- [`src/main_menu.cpp`](../src/main_menu.cpp)
  UI creation, refresh logic, formatting, touch actions, page control mapping.

### Configuration

- [`include/lv_conf.h`](../include/lv_conf.h)
  LVGL compile-time settings.

## 17. Important Implementation Characteristics

These are not bugs by themselves, but they are important to know when working on the project:

1. The system is single-threaded and cooperative.
   Long blocking work in any module will hurt touch response, MIDI timing, and UI updates.

2. `AppState` is the main source of truth.
   If behavior looks wrong, verify whether the state is being updated correctly before blaming audio or UI code.

3. The UI often displays interpreted values, not raw internal values.
   Some labels are approximations for readability.

4. The current playable synth is mono and lead-voice focused.
   The audio graph is larger than the feature set actively exposed today.

5. The input-test page is diagnostic, not the final authority on current control mapping.

6. The repository still contains older architectures in `lib/`.
   Do not accidentally treat them as the active code path.

## 18. Recommended Reading Order For New Contributors

If you want to learn the codebase efficiently:

1. read [`src/main.cpp`](../src/main.cpp) for runtime flow
2. read [`include/app_state.h`](../include/app_state.h) for the data model
3. read [`src/main_menu.cpp`](../src/main_menu.cpp) for control mapping
4. read [`src/performance_engine.cpp`](../src/performance_engine.cpp) for note scheduling
5. read [`src/synth.cpp`](../src/synth.cpp) for sound generation logic
6. read [`src/audio_setup.cpp`](../src/audio_setup.cpp) for audio routing
7. read [`src/input_test_page.cpp`](../src/input_test_page.cpp) only after the main flow is clear

That order follows the most useful path from “what runs” to “what it means” to “how it sounds”.
