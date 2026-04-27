#ifndef CONTROL_INPUT_H
#define CONTROL_INPUT_H

#include <Arduino.h>

class ControlInput {
 public:
  static constexpr uint8_t kPotCount = 5;

  void begin();
  void update();

  bool consumePotChange(uint8_t index, float &value);
  bool consumeOkPress();

  float potValue(uint8_t index) const;
  uint16_t rawChannelValue(uint8_t index) const;
  uint16_t rawButtonValue() const;
  bool buttonPressed() const;

 private:
  static constexpr uint8_t kPotChannels[kPotCount] = {2, 4, 1, 5, 0};
  static constexpr uint8_t kButtonChannel = 7;

  void selectChannel(uint8_t channel) const;
  uint16_t readChannel(uint8_t channel) const;

  float filtered_values_[kPotCount] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  float pending_values_[kPotCount] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  bool pending_changes_[kPotCount] = {false, false, false, false, false};
  uint16_t raw_values_[kPotCount] = {0, 0, 0, 0, 0};
  uint16_t raw_button_value_ = 0;
  bool button_pressed_ = false;
  bool pending_ok_press_ = false;
  unsigned long last_button_change_ms_ = 0;
};

extern ControlInput control_input;

#endif
