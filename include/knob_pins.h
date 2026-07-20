#ifndef KNOB_PINS_H
#define KNOB_PINS_H

#include <Arduino.h>

// The five panel knobs connect straight to Teensy analog pins
// (PCB rev without the BOB-09056 mux). Knob N wires to header RVN.
namespace KnobPins {
constexpr uint8_t kKnob1 = 14;  // A0
constexpr uint8_t kKnob2 = 15;  // A1
constexpr uint8_t kKnob3 = 16;  // A2
constexpr uint8_t kKnob4 = 17;  // A3
constexpr uint8_t kKnob5 = 22;  // A8
constexpr uint8_t kKnobPins[] = {kKnob1, kKnob2, kKnob3, kKnob4, kKnob5};
}

#endif
