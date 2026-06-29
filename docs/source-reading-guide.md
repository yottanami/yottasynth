# Source Reading Guide

This document explains the C++ and embedded-firmware patterns used in this repository.

The goal is not to teach all of C++. The goal is to make this specific codebase readable.

## The Mental Model

This project is not a desktop application. It is firmware running on a `Teensy 4.1` microcontroller.

That changes the programming model in a few important ways:

1. there is no operating-system style main event loop created for you
2. the code runs continuously after boot
3. hardware is controlled by reading and writing pins
4. many objects are created once and live for the whole lifetime of the device
5. timing is often measured with `millis()` or `micros()` instead of threads or timers

The core firmware shape is:

```cpp
void setup() {
  // Run once at boot.
}

void loop() {
  // Run forever.
}
```

That pattern comes from Arduino and is the center of this project.

## How To Read This Repository

Most active modules use the usual C++ split:

- `.h` file
  Declares types, constants, and function signatures.

- `.cpp` file
  Implements the actual logic.

Example:

- `include/synth.h` declares the `Synth` class.
- `src/synth.cpp` implements the `Synth` class.

## Important C++ Features Used Here

### `enum class`

This project uses `enum class` to represent named choices safely.

Example:

```cpp
enum class FxMode : uint8_t {
  ECHO = 0,
  REVERB,
  DRIVE
};
```

Why this is useful:

- code is easier to read than raw numbers
- the compiler prevents mixing unrelated enums by accident
- it documents the allowed states clearly

You will see this pattern for pages, LFO targets, waveforms, arpeggiator modes, effects, and tunings.

### `struct`

`struct` is used here as a lightweight data container.

Example:

```cpp
struct TransportState {
  uint16_t bpm = 120;
  bool running = false;
  float swing = 0.0f;
  uint8_t step_index = 0;
  ClockSource clock_source = ClockSource::INTERNAL;
  bool ext_clock_present = false;
};
```

In this codebase, structs usually mean:

- “these values belong together”
- “this is state, not behavior”

That is why `AppState` contains many small structs such as `PatchState`, `TransportState`, and `FxState`.

### `class`

Classes in this project usually own behavior.

Examples:

- `ControlInput`
  Reads knobs and the confirm button from the hardware multiplexer.

- `MainMenu`
  Builds and refreshes the touch UI.

- `PerformanceEngine`
  Decides what note should play and when.

- `Synth`
  Applies patch values and drives the audio objects.

In this codebase:

- `struct`
  Mostly data

- `class`
  Mostly behavior plus private internal data

This is not a strict rule in C++, but it is a useful reading shortcut for this project.

### Singleton

This project uses the singleton pattern for global shared state.

Example:

```cpp
AppState &AppState::instance() {
  static AppState state;
  return state;
}
```

Meaning:

- only one `AppState` object exists
- any module can access it through `AppState::instance()`

Why the project does this:

- the firmware has one global instrument state
- many modules need access to the same patch, transport, UI, MIDI, and audio status

You will also see an older singleton-style `Settings` class that stores the high-level mode.

### `static`

`static` appears in several different meanings in C++. In this codebase, the most common ones are:

1. static member functions
2. static local variables
3. file-local helpers in anonymous namespaces

#### 1. Static member functions

Example:

```cpp
static void tabEventHandler(lv_event_t *event);
```

This means the function belongs to the class, but does not operate on a specific object automatically through `this`.

It is useful for callback APIs such as LVGL or MIDI, where a library expects a plain function pointer.

#### 2. Static local variables

Example:

```cpp
static AppState state;
```

This object is created once and reused on later calls.

#### 3. File-local helpers

Example:

```cpp
namespace {
float clampUnit(float value) { ... }
}
```

This anonymous namespace means:

- the helper is private to that `.cpp` file
- other files cannot link to it accidentally

That is a common modern C++ way to say “internal helper for this file only”.

### `extern`

`extern` means “this object exists somewhere else; I only want to refer to it here”.

Example:

```cpp
extern MainMenu main_menu;
```

The header declares the object, and one `.cpp` file creates it:

```cpp
MainMenu main_menu;
```

This pattern is used often in embedded code because many hardware/service objects are global and long-lived.

### References and pointers

You will see both references and pointers.

Reference example:

```cpp
AppState &state = AppState::instance();
```

A reference is like another name for an existing object.

Pointer example:

```cpp
Synth *synth_ = nullptr;
```

A pointer can be empty and later point at an object. `PerformanceEngine` uses a pointer because its `Synth` dependency is attached later in `begin(Synth *synth)`.

Practical reading rule:

- `Type &name`
  This must refer to a real object.

- `Type *name`
  This may be null, so the code often checks it before use.

### Arrays

This codebase uses classic C-style arrays in many places because they are simple and efficient on embedded targets.

Examples:

- `SequenceStep steps[16];`
- `uint8_t held_notes_[kHeldCapacity] = {0};`
- `lv_obj_t *tab_buttons_[kTabCount] = {nullptr, ...};`

This is normal in microcontroller code where fixed sizes are known up front and heap allocation is often avoided.

### Callbacks

A callback is a function you give to a library so the library can call you later.

Examples in this project:

- LVGL touch/button event handlers
- USB MIDI note handlers
- display flush callback for the screen driver
- touch read callback for LVGL input

This is why some functions are `static` and have a library-defined signature.

### `constexpr`

`constexpr` means the value is known at compile time.

Example:

```cpp
static constexpr uint8_t kPotCount = 5;
```

Why it is used here:

- makes code self-documenting
- avoids magic numbers
- keeps values cheap and safe for embedded use

### `uint8_t`, `uint16_t`, `int16_t`

These types come from fixed-width integer headers and are very common in embedded code.

Examples:

- `uint8_t`
  Unsigned 8-bit integer, range `0..255`

- `uint16_t`
  Unsigned 16-bit integer, range `0..65535`

- `int16_t`
  Signed 16-bit integer

Why they matter:

- hardware protocols often expect exact widths
- memory usage is more predictable than with plain `int`

### `byte`

Arduino also defines `byte`. In practice it is similar to `uint8_t`.

This project uses both. That is common in mixed Arduino/C++ codebases.

## The “Global Object” Style

In desktop C++, people often avoid global objects. In embedded firmware, global objects are much more common.

Examples in this project:

- `control_input`
- `main_menu`
- `performance_engine`
- `input_test_page`
- `synth`

This style is used because:

- there is one hardware instrument
- the objects are needed for the whole runtime
- setup order is explicit in `setup()`

## Header Guards

You will often see this at the top of header files:

```cpp
#ifndef SYNTH_H
#define SYNTH_H
...
#endif
```

This prevents the same header from being included more than once during compilation.

## Normalized Values

Many parameters in the project are stored as `float` values between `0.0f` and `1.0f`.

Examples:

- filter cutoff control
- resonance
- effect mix
- output volume

Why this is useful:

- knob input naturally becomes a normalized value
- UI can display it in different ways
- audio code can map it to real ranges later

Example:

- UI or control input stores `0.72f`
- synth code converts it into a real filter frequency
- UI code converts it into a readable label

This separation is important to understand the code structure.

## Time In The Firmware

The project uses:

- `millis()`
  Milliseconds since boot

- `micros()`
  Microseconds since boot

Examples:

- button debounce
- transport scheduling
- status expiry
- touch confirmation
- self-test timeout

This is a common non-blocking embedded pattern:

1. read current time
2. compare it with the previous timestamp
3. run work only when enough time has passed

## Why There Are So Many Helper Functions

Functions such as:

- `clampUnit`
- `bpmFromNormalized`
- `noteName`
- `stepIntervalUs`
- `normalizedToFilterHz`

exist to keep the main logic readable.

When you read a file, treat those helpers as small translation layers:

- raw input to normalized data
- normalized data to human-readable text
- musical data to audio parameters

## How To Read A Module In This Codebase

Use this order:

1. open the header file
   Identify the class, its public functions, and private data members.

2. find the `begin()` or `setup()` function
   This usually tells you how the module is initialized.

3. find the `loop()` or `update()` function
   This usually tells you what the module does repeatedly.

4. read the event handlers
   These show what happens when MIDI arrives, a touch button is pressed, or a knob changes.

5. read helper functions last
   They usually support the main flow rather than define it.

## Project-Specific Reading Tips

### `AppState` is the center

If you are lost, start from `AppState`. It contains most of the instrument’s live state.

### `main.cpp` is the orchestration file

It does not contain the deepest logic, but it shows the order in which subsystems run every frame.

### `MainMenu` translates human actions into state changes

It is best thought of as the control panel layer, not the sound engine.

### `PerformanceEngine` decides what notes happen

It handles the arpeggiator, sequencer timing, held notes, and generated note durations.

### `Synth` turns state into sound settings

It controls oscillator frequencies, waveforms, envelopes, filter settings, pitch bend, and tuning tables.

## Legacy Code Warning

The repository also contains older code in `lib/`.

That code is still useful to study historically, but if you want to understand the current firmware behavior, focus on:

- `src/`
- `include/`

The current active firmware starts in `src/main.cpp`, not in `lib/synthesizer/synthesizer.cpp`.

## Final Reading Strategy

If you want the fastest path to understanding the project, read:

1. [`src/main.cpp`](../src/main.cpp)
2. [`include/app_state.h`](../include/app_state.h)
3. [`src/main_menu.cpp`](../src/main_menu.cpp)
4. [`src/performance_engine.cpp`](../src/performance_engine.cpp)
5. [`src/synth.cpp`](../src/synth.cpp)
6. [`src/audio_setup.cpp`](../src/audio_setup.cpp)

Then use the detailed [Codebase Guide](codebase-guide.md) as the reference while you read the source.
