#ifndef CONTROL_INPUT_H
#define CONTROL_INPUT_H

#include <Arduino.h>

class ControlInput {
 public:
  static constexpr uint8_t kPotCount = 5;

  void begin();
  void update();

  bool consumePotChange(uint8_t index, float &value);

  float potValue(uint8_t index) const;
  uint16_t rawChannelValue(uint8_t index) const;

 private:
  uint16_t readKnob(uint8_t pin) const;

  float filtered_values_[kPotCount] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  float pending_values_[kPotCount] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  bool pending_changes_[kPotCount] = {false, false, false, false, false};
  uint16_t raw_values_[kPotCount] = {0, 0, 0, 0, 0};
};

extern ControlInput control_input;

#endif
