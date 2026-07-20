#include "control_input.h"

#include "knob_pins.h"

namespace {
constexpr uint16_t kAnalogMax = 4095;
constexpr float kSmoothing = 0.20f;
constexpr float kPotThreshold = 0.015f;
}

void ControlInput::begin() {
  for (const uint8_t pin : KnobPins::kKnobPins) {
    pinMode(pin, INPUT_DISABLE);  // pure analog input, no digital keeper
  }

  analogReadResolution(12);
  analogReadAveraging(8);

  for (uint8_t index = 0; index < kPotCount; ++index) {
    const uint16_t raw = readKnob(KnobPins::kKnobPins[index]);
    raw_values_[index] = raw;
    const float normalized = static_cast<float>(raw) / static_cast<float>(kAnalogMax);
    filtered_values_[index] = normalized;
    pending_values_[index] = normalized;
  }
}

void ControlInput::update() {
  for (uint8_t index = 0; index < kPotCount; ++index) {
    const uint16_t raw = readKnob(KnobPins::kKnobPins[index]);
    raw_values_[index] = raw;

    const float normalized = static_cast<float>(raw) / static_cast<float>(kAnalogMax);
    const float filtered = filtered_values_[index] + ((normalized - filtered_values_[index]) * kSmoothing);
    filtered_values_[index] = filtered;

    if (fabsf(filtered - pending_values_[index]) >= kPotThreshold) {
      pending_values_[index] = filtered;
      pending_changes_[index] = true;
    }
  }
}

bool ControlInput::consumePotChange(uint8_t index, float &value) {
  if (index >= kPotCount || !pending_changes_[index]) {
    return false;
  }

  pending_changes_[index] = false;
  value = pending_values_[index];
  return true;
}

float ControlInput::potValue(uint8_t index) const {
  if (index >= kPotCount) {
    return 0.0f;
  }
  return filtered_values_[index];
}

uint16_t ControlInput::rawChannelValue(uint8_t index) const {
  if (index >= kPotCount) {
    return 0;
  }
  return raw_values_[index];
}

uint16_t ControlInput::readKnob(uint8_t pin) const {
  analogRead(pin);  // discard first sample after the ADC switches channel

  uint32_t total = 0;
  for (uint8_t sample = 0; sample < 4; ++sample) {
    total += analogRead(pin);
  }

  return static_cast<uint16_t>(total / 4U);
}

ControlInput control_input;
