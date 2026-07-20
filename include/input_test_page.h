#ifndef INPUT_TEST_PAGE_H
#define INPUT_TEST_PAGE_H

#include <Arduino.h>
#include <lvgl.h>

#include "knob_pins.h"

class InputTestPage {
public:
  InputTestPage();

  void begin();
  lv_obj_t * createPage(lv_obj_t * menu);
  void loop();
  void setProbeEnabled(bool enabled);
  void updateTouch(bool pressed, int16_t x, int16_t y, int16_t raw_x, int16_t raw_y,
                   int16_t raw_z);

private:
  static constexpr uint8_t KNOB_COUNT = 5;
  static constexpr uint8_t PROBE_PIN_COUNT = 14;

  static void selfTestEventHandler(lv_event_t *event);
  static void resetEventHandler(lv_event_t *event);

  void configureAnalogPins();
  void sampleKnobs();
  void sampleProbePins();
  void refreshLabels();
  void resetActivityTracking();
  uint16_t readAnalogPin(uint8_t pin);

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
  uint16_t knob_values_[KNOB_COUNT];
  uint16_t knob_min_[KNOB_COUNT];
  uint16_t knob_max_[KNOB_COUNT];
  uint16_t probe_pin_values_[PROBE_PIN_COUNT];
  uint16_t probe_pin_min_[PROBE_PIN_COUNT];
  uint16_t probe_pin_max_[PROBE_PIN_COUNT];
  bool touch_pressed_;
  int16_t touch_x_;
  int16_t touch_y_;
  int16_t raw_touch_x_;
  int16_t raw_touch_y_;
  int16_t raw_touch_z_;
  bool probe_enabled_;
  bool initialized_;
  bool activity_initialized_;
  bool last_touch_pressed_;
  unsigned long last_knob_sample_ms_;
  unsigned long last_probe_sample_ms_;
  unsigned long last_refresh_ms_;
};

extern InputTestPage input_test_page;

#endif
