#ifndef INPUT_TEST_PAGE_H
#define INPUT_TEST_PAGE_H

#include <Arduino.h>
#include <lvgl.h>

#include "mux_pins.h"

class InputTestPage {
public:
  InputTestPage();

  void begin();
  lv_obj_t * createPage(lv_obj_t * menu);
  void loop();
  void setMuxEnabled(bool enabled);
  void updateTouch(bool pressed, int16_t x, int16_t y, int16_t raw_x, int16_t raw_y,
                   int16_t raw_z);

private:
  static constexpr uint8_t MUX_TOTAL_CHANNEL_COUNT = 16;
  static constexpr uint8_t MUX_POT_CHANNEL_COUNT = 5;
  static constexpr uint8_t DIRECT_PIN_COUNT = 19;
  static constexpr uint8_t MUX_S0_PIN = MuxPins::kS0;
  static constexpr uint8_t MUX_S1_PIN = MuxPins::kS1;
  static constexpr uint8_t MUX_S2_PIN = MuxPins::kS2;
  static constexpr uint8_t MUX_S3_PIN = MuxPins::kS3;
  static constexpr uint8_t MUX_SIGNAL_PIN = MuxPins::kSignal;

  static void selfTestEventHandler(lv_event_t *event);
  static void resetEventHandler(lv_event_t *event);

  void configureMuxPins();
  void primeMuxChannels();
  void scanMuxStep();
  void sampleDirectPins();
  void refreshLabels();
  void resetActivityTracking();
  void selectChannel(uint8_t channel) const;
  uint16_t readDirectPin(uint8_t pin);

  lv_obj_t * page_;
  lv_obj_t * controls_row_;
  lv_obj_t * self_test_button_;
  lv_obj_t * self_test_label_;
  lv_obj_t * reset_button_;
  lv_obj_t * reset_label_;
  lv_obj_t * touch_label_;
  lv_obj_t * warning_label_;
  lv_obj_t * system_label_;
  lv_obj_t * summary_label_;
  lv_obj_t * channels_label_;
  uint16_t channel_values_[MUX_TOTAL_CHANNEL_COUNT];
  uint16_t channel_min_[MUX_TOTAL_CHANNEL_COUNT];
  uint16_t channel_max_[MUX_TOTAL_CHANNEL_COUNT];
  uint16_t direct_pin_values_[DIRECT_PIN_COUNT];
  uint16_t direct_pin_min_[DIRECT_PIN_COUNT];
  uint16_t direct_pin_max_[DIRECT_PIN_COUNT];
  bool touch_pressed_;
  int16_t touch_x_;
  int16_t touch_y_;
  int16_t raw_touch_x_;
  int16_t raw_touch_y_;
  int16_t raw_touch_z_;
  bool mux_enabled_;
  bool initialized_;
  bool activity_initialized_;
  bool last_touch_pressed_;
  uint8_t mux_input_;
  unsigned long last_mux_scan_us_;
  unsigned long last_direct_sample_ms_;
  unsigned long last_refresh_ms_;
};

extern InputTestPage input_test_page;

#endif
