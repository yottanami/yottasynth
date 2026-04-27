#include "control_input.h"

#include "mux_pins.h"

namespace {
constexpr uint16_t kAnalogMax = 4095;
constexpr float kSmoothing = 0.20f;
constexpr float kPotThreshold = 0.015f;
constexpr uint16_t kButtonThreshold = 1800;
constexpr unsigned long kButtonDebounceMs = 25;
}

void ControlInput::begin() {
  for (const uint8_t pin : MuxPins::kSelectPins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  pinMode(MuxPins::kSignal, INPUT);
  analogReadResolution(12);
  analogReadAveraging(8);

  for (uint8_t index = 0; index < kPotCount; ++index) {
    const uint16_t raw = readChannel(kPotChannels[index]);
    raw_values_[index] = raw;
    const float normalized = static_cast<float>(raw) / static_cast<float>(kAnalogMax);
    filtered_values_[index] = normalized;
    pending_values_[index] = normalized;
  }

  raw_button_value_ = readChannel(kButtonChannel);
  button_pressed_ = raw_button_value_ < kButtonThreshold;
}

void ControlInput::update() {
  for (uint8_t index = 0; index < kPotCount; ++index) {
    const uint16_t raw = readChannel(kPotChannels[index]);
    raw_values_[index] = raw;

    const float normalized = static_cast<float>(raw) / static_cast<float>(kAnalogMax);
    const float filtered = filtered_values_[index] + ((normalized - filtered_values_[index]) * kSmoothing);
    filtered_values_[index] = filtered;

    if (fabsf(filtered - pending_values_[index]) >= kPotThreshold) {
      pending_values_[index] = filtered;
      pending_changes_[index] = true;
    }
  }

  raw_button_value_ = readChannel(kButtonChannel);
  const bool sampled_pressed = raw_button_value_ < kButtonThreshold;
  const unsigned long now = millis();

  if (sampled_pressed != button_pressed_ && (now - last_button_change_ms_) >= kButtonDebounceMs) {
    button_pressed_ = sampled_pressed;
    last_button_change_ms_ = now;
    if (button_pressed_) {
      pending_ok_press_ = true;
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

bool ControlInput::consumeOkPress() {
  if (!pending_ok_press_) {
    return false;
  }

  pending_ok_press_ = false;
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

uint16_t ControlInput::rawButtonValue() const {
  return raw_button_value_;
}

bool ControlInput::buttonPressed() const {
  return button_pressed_;
}

void ControlInput::selectChannel(uint8_t channel) const {
  for (uint8_t bit = 0; bit < 4; ++bit) {
    digitalWrite(MuxPins::kSelectPins[bit], bitRead(channel, bit));
  }
}

uint16_t ControlInput::readChannel(uint8_t channel) const {
  selectChannel(channel);
  delayMicroseconds(50);
  analogRead(MuxPins::kSignal);
  delayMicroseconds(8);

  uint32_t total = 0;
  for (uint8_t sample = 0; sample < 4; ++sample) {
    total += analogRead(MuxPins::kSignal);
  }

  return static_cast<uint16_t>(total / 4U);
}

ControlInput control_input;
