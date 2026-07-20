#include "input_test_page.h"

#include <stdio.h>

#include "app_state.h"
#include "synth.h"

extern Synth synth;

namespace {
constexpr const char *kKnobNames[5] = {"RV1/A0", "RV2/A1", "RV3/A2", "RV4/A3", "RV5/A8"};
constexpr uint16_t kAdcMaxValue = 4095;
// Spare analog pins plus audio-shield pins, probed only while the input
// test page is active (probing shield pins disturbs a running codec).
constexpr uint8_t kProbePins[14] = {8, 18, 19, 20, 21, 23, 24, 25, 26, 27, 38, 39, 40, 41};
constexpr uint8_t kTouchIrqPin = 4;
constexpr unsigned long kKnobSampleIntervalMs = 25;
constexpr unsigned long kProbeSampleIntervalMs = 50;
constexpr unsigned long kUiRefreshIntervalMs = 200;

uint8_t rawToPercent(uint16_t value) {
  if (value >= kAdcMaxValue) {
    return 100;
  }
  return static_cast<uint8_t>((static_cast<uint32_t>(value) * 100U + (kAdcMaxValue / 2U)) / kAdcMaxValue);
}

uint16_t spanFromMinMax(uint16_t min_value, uint16_t max_value) {
  return max_value >= min_value ? static_cast<uint16_t>(max_value - min_value) : 0;
}
}

InputTestPage::InputTestPage()
  : page_(nullptr),
    controls_row_(nullptr),
    self_test_button_(nullptr),
    self_test_label_(nullptr),
    reset_button_(nullptr),
    reset_label_(nullptr),
    touch_label_(nullptr),
    warning_label_(nullptr),
    system_label_(nullptr),
    summary_label_(nullptr),
    channels_label_(nullptr),
    touch_pressed_(false),
    touch_x_(0),
    touch_y_(0),
    raw_touch_x_(0),
    raw_touch_y_(0),
    raw_touch_z_(0),
    probe_enabled_(true),
    initialized_(false),
    activity_initialized_(false),
    last_touch_pressed_(false),
    last_knob_sample_ms_(0),
    last_probe_sample_ms_(0),
    last_refresh_ms_(0) {
  memset(knob_values_, 0, sizeof(knob_values_));
  memset(knob_min_, 0, sizeof(knob_min_));
  memset(knob_max_, 0, sizeof(knob_max_));
  memset(probe_pin_values_, 0, sizeof(probe_pin_values_));
  memset(probe_pin_min_, 0, sizeof(probe_pin_min_));
  memset(probe_pin_max_, 0, sizeof(probe_pin_max_));
}

void InputTestPage::begin() {
  if (!initialized_) {
    configureAnalogPins();
    sampleKnobs();
    if (probe_enabled_) {
      sampleProbePins();
    }
    resetActivityTracking();
    initialized_ = true;
  }

  refreshLabels();
}

void InputTestPage::setProbeEnabled(bool enabled) {
  probe_enabled_ = enabled;
}

void InputTestPage::configureAnalogPins() {
  for (const uint8_t pin : KnobPins::kKnobPins) {
    pinMode(pin, INPUT_DISABLE);
  }

  analogReadResolution(12);
  analogReadAveraging(8);
}

lv_obj_t * InputTestPage::createPage(lv_obj_t * menu) {
  if (page_ != nullptr) {
    return page_;
  }

  page_ = lv_obj_create(menu);
  lv_obj_remove_style_all(page_);
  lv_obj_set_size(page_, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(page_, lv_color_hex(0x0E1726), 0);
  lv_obj_set_style_pad_all(page_, 8, 0);
  lv_obj_set_style_pad_row(page_, 6, 0);
  lv_obj_set_flex_flow(page_, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scroll_dir(page_, LV_DIR_VER);

  controls_row_ = lv_obj_create(page_);
  lv_obj_set_width(controls_row_, lv_pct(100));
  lv_obj_set_style_pad_all(controls_row_, 4, 0);
  lv_obj_set_style_pad_column(controls_row_, 8, 0);
  lv_obj_set_flex_flow(controls_row_, LV_FLEX_FLOW_ROW);

  self_test_button_ = lv_button_create(controls_row_);
  lv_obj_set_size(self_test_button_, 136, 28);
  lv_obj_add_event_cb(self_test_button_, selfTestEventHandler, LV_EVENT_CLICKED, nullptr);
  self_test_label_ = lv_label_create(self_test_button_);
  lv_obj_center(self_test_label_);

  reset_button_ = lv_button_create(controls_row_);
  lv_obj_set_size(reset_button_, 136, 28);
  lv_obj_add_event_cb(reset_button_, resetEventHandler, LV_EVENT_CLICKED, nullptr);
  reset_label_ = lv_label_create(reset_button_);
  lv_label_set_text(reset_label_, "RESET RANGES");
  lv_obj_center(reset_label_);

  lv_obj_t * cont = lv_obj_create(page_);
  lv_obj_set_width(cont, lv_pct(100));
  touch_label_ = lv_label_create(cont);
  lv_obj_set_width(touch_label_, lv_pct(100));
  lv_label_set_long_mode(touch_label_, LV_LABEL_LONG_WRAP);

  cont = lv_obj_create(page_);
  lv_obj_set_width(cont, lv_pct(100));
  warning_label_ = lv_label_create(cont);
  lv_obj_set_width(warning_label_, lv_pct(100));
  lv_label_set_long_mode(warning_label_, LV_LABEL_LONG_WRAP);

  cont = lv_obj_create(page_);
  lv_obj_set_width(cont, lv_pct(100));
  system_label_ = lv_label_create(cont);
  lv_obj_set_width(system_label_, lv_pct(100));
  lv_label_set_long_mode(system_label_, LV_LABEL_LONG_WRAP);

  cont = lv_obj_create(page_);
  lv_obj_set_width(cont, lv_pct(100));
  summary_label_ = lv_label_create(cont);
  lv_obj_set_width(summary_label_, lv_pct(100));
  lv_label_set_long_mode(summary_label_, LV_LABEL_LONG_WRAP);

  cont = lv_obj_create(page_);
  lv_obj_set_width(cont, lv_pct(100));
  channels_label_ = lv_label_create(cont);
  lv_obj_set_width(channels_label_, lv_pct(100));
  lv_label_set_long_mode(channels_label_, LV_LABEL_LONG_WRAP);

  refreshLabels();
  return page_;
}

void InputTestPage::selfTestEventHandler(lv_event_t *event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
    return;
  }

  synth.startSelfTest();
  AppState::instance().markDirty();
  input_test_page.refreshLabels();
}

void InputTestPage::resetEventHandler(lv_event_t *event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
    return;
  }

  input_test_page.resetActivityTracking();
  input_test_page.refreshLabels();
}

void InputTestPage::loop() {
  if (!initialized_) {
    configureAnalogPins();
    sampleKnobs();
    if (probe_enabled_) {
      sampleProbePins();
    }
    resetActivityTracking();
    initialized_ = true;
  }

  if (touch_pressed_ && !last_touch_pressed_) {
    resetActivityTracking();
  }

  const unsigned long now_ms = millis();
  if ((now_ms - last_knob_sample_ms_) >= kKnobSampleIntervalMs) {
    sampleKnobs();
    last_knob_sample_ms_ = now_ms;
  }

  if (probe_enabled_ && (now_ms - last_probe_sample_ms_) >= kProbeSampleIntervalMs) {
    sampleProbePins();
    last_probe_sample_ms_ = now_ms;
  }

  if ((now_ms - last_refresh_ms_) >= kUiRefreshIntervalMs) {
    last_refresh_ms_ = now_ms;
    refreshLabels();
  }

  last_touch_pressed_ = touch_pressed_;
}

void InputTestPage::updateTouch(bool pressed, int16_t x, int16_t y, int16_t raw_x, int16_t raw_y,
                                int16_t raw_z) {
  touch_pressed_ = pressed;
  touch_x_ = x;
  touch_y_ = y;
  raw_touch_x_ = raw_x;
  raw_touch_y_ = raw_y;
  raw_touch_z_ = raw_z;
}

void InputTestPage::sampleKnobs() {
  for (uint8_t index = 0; index < KNOB_COUNT; ++index) {
    const uint16_t value = readAnalogPin(KnobPins::kKnobPins[index]);
    knob_values_[index] = value;
    if (activity_initialized_) {
      if (value < knob_min_[index]) {
        knob_min_[index] = value;
      }
      if (value > knob_max_[index]) {
        knob_max_[index] = value;
      }
    }
  }
}

void InputTestPage::sampleProbePins() {
  for (uint8_t index = 0; index < PROBE_PIN_COUNT; ++index) {
    const uint16_t value = readAnalogPin(kProbePins[index]);
    probe_pin_values_[index] = value;
    if (activity_initialized_) {
      if (value < probe_pin_min_[index]) {
        probe_pin_min_[index] = value;
      }
      if (value > probe_pin_max_[index]) {
        probe_pin_max_[index] = value;
      }
    }
  }
}

void InputTestPage::resetActivityTracking() {
  memcpy(knob_min_, knob_values_, sizeof(knob_values_));
  memcpy(knob_max_, knob_values_, sizeof(knob_values_));
  memcpy(probe_pin_min_, probe_pin_values_, sizeof(probe_pin_values_));
  memcpy(probe_pin_max_, probe_pin_values_, sizeof(probe_pin_values_));
  activity_initialized_ = true;
}

void InputTestPage::refreshLabels() {
  if (touch_label_ == nullptr || warning_label_ == nullptr || system_label_ == nullptr ||
      summary_label_ == nullptr || channels_label_ == nullptr || self_test_label_ == nullptr) {
    return;
  }

  char touch_text[96];
  char warning_text[256];
  char system_text[224];
  char summary_text[448];
  char channels_text[1024];
  const AppState &state = AppState::instance();
  const int touch_irq = digitalRead(kTouchIrqPin);

  uint16_t top_span[4] = {0, 0, 0, 0};
  char top_name[4][8] = {"-", "-", "-", "-"};

  for (uint8_t index = 0; index < KNOB_COUNT; ++index) {
    const uint16_t span = spanFromMinMax(knob_min_[index], knob_max_[index]);
    for (uint8_t slot = 0; slot < 4; ++slot) {
      if (span > top_span[slot]) {
        for (uint8_t shift = 3; shift > slot; --shift) {
          top_span[shift] = top_span[shift - 1];
          memcpy(top_name[shift], top_name[shift - 1], sizeof(top_name[shift]));
        }
        top_span[slot] = span;
        snprintf(top_name[slot], sizeof(top_name[slot]), "RV%u", index + 1U);
        break;
      }
    }
  }

  for (uint8_t index = 0; index < PROBE_PIN_COUNT; ++index) {
    const uint16_t span = spanFromMinMax(probe_pin_min_[index], probe_pin_max_[index]);
    for (uint8_t slot = 0; slot < 4; ++slot) {
      if (span > top_span[slot]) {
        for (uint8_t shift = 3; shift > slot; --shift) {
          top_span[shift] = top_span[shift - 1];
          memcpy(top_name[shift], top_name[shift - 1], sizeof(top_name[shift]));
        }
        top_span[slot] = span;
        snprintf(top_name[slot], sizeof(top_name[slot]), "P%02u", kProbePins[index]);
        break;
      }
    }
  }

  snprintf(
      touch_text,
      sizeof(touch_text),
      "Touch: %s X:%d Y:%d IRQ:%d Z:%d\nRaw: X:%d Y:%d",
      touch_pressed_ ? "DOWN" : "UP",
      touch_x_,
      touch_y_,
      touch_irq,
      raw_touch_z_,
      raw_touch_x_,
      raw_touch_y_);

  snprintf(
      system_text,
      sizeof(system_text),
      "System\nAudio:%s  Self-test:%s\nMIDI:%s  Note:%u Vel:%u",
      state.audio.codec_ready ? "OK" : "FAIL",
      state.audio.self_test_active ? "ON" : "OFF",
      state.midi.connected ? "OK" : "WAIT",
      state.midi.last_note,
      state.midi.last_velocity);

  snprintf(
      warning_text,
      sizeof(warning_text),
      probe_enabled_
          ? "Knobs read directly: RV1..RV5 on pins 14 15 16 17 22.\nTap the screen once to reset activity ranges, then move one knob."
          : "Pin probing is paused outside this page.\nKnob values below stay live; spare/audio pins are only probed here.");

  snprintf(
      summary_text,
      sizeof(summary_text),
      "Knobs 0..100\n%s:%3u  %s:%3u\n%s:%3u  %s:%3u\n%s:%3u\n\nTop movers since reset\n%s d%u  %s d%u\n%s d%u  %s d%u",
      kKnobNames[0], rawToPercent(knob_values_[0]),
      kKnobNames[1], rawToPercent(knob_values_[1]),
      kKnobNames[2], rawToPercent(knob_values_[2]),
      kKnobNames[3], rawToPercent(knob_values_[3]),
      kKnobNames[4], rawToPercent(knob_values_[4]),
      top_name[0], top_span[0], top_name[1], top_span[1],
      top_name[2], top_span[2], top_name[3], top_span[3]);

  snprintf(
      channels_text,
      sizeof(channels_text),
      "Knob raw / delta\n"
      "RV1:%4u d%-4u RV2:%4u d%-4u\n"
      "RV3:%4u d%-4u RV4:%4u d%-4u\n"
      "RV5:%4u d%-4u\n\n"
      "Probe analog raw / delta\n"
      "P08:%4u d%-4u P18:%4u d%-4u\n"
      "P19:%4u d%-4u P20:%4u d%-4u\n"
      "P21:%4u d%-4u P23:%4u d%-4u\n"
      "P24:%4u d%-4u P25:%4u d%-4u\n"
      "P26:%4u d%-4u P27:%4u d%-4u\n"
      "P38:%4u d%-4u P39:%4u d%-4u\n"
      "P40:%4u d%-4u P41:%4u d%-4u",
      knob_values_[0], spanFromMinMax(knob_min_[0], knob_max_[0]),
      knob_values_[1], spanFromMinMax(knob_min_[1], knob_max_[1]),
      knob_values_[2], spanFromMinMax(knob_min_[2], knob_max_[2]),
      knob_values_[3], spanFromMinMax(knob_min_[3], knob_max_[3]),
      knob_values_[4], spanFromMinMax(knob_min_[4], knob_max_[4]),
      probe_pin_values_[0], spanFromMinMax(probe_pin_min_[0], probe_pin_max_[0]),
      probe_pin_values_[1], spanFromMinMax(probe_pin_min_[1], probe_pin_max_[1]),
      probe_pin_values_[2], spanFromMinMax(probe_pin_min_[2], probe_pin_max_[2]),
      probe_pin_values_[3], spanFromMinMax(probe_pin_min_[3], probe_pin_max_[3]),
      probe_pin_values_[4], spanFromMinMax(probe_pin_min_[4], probe_pin_max_[4]),
      probe_pin_values_[5], spanFromMinMax(probe_pin_min_[5], probe_pin_max_[5]),
      probe_pin_values_[6], spanFromMinMax(probe_pin_min_[6], probe_pin_max_[6]),
      probe_pin_values_[7], spanFromMinMax(probe_pin_min_[7], probe_pin_max_[7]),
      probe_pin_values_[8], spanFromMinMax(probe_pin_min_[8], probe_pin_max_[8]),
      probe_pin_values_[9], spanFromMinMax(probe_pin_min_[9], probe_pin_max_[9]),
      probe_pin_values_[10], spanFromMinMax(probe_pin_min_[10], probe_pin_max_[10]),
      probe_pin_values_[11], spanFromMinMax(probe_pin_min_[11], probe_pin_max_[11]),
      probe_pin_values_[12], spanFromMinMax(probe_pin_min_[12], probe_pin_max_[12]),
      probe_pin_values_[13], spanFromMinMax(probe_pin_min_[13], probe_pin_max_[13]));

  lv_label_set_text(self_test_label_, synth.isSelfTestActive() ? "TESTING" : "AUDIO TEST");
  lv_label_set_text(touch_label_, touch_text);
  lv_label_set_text(warning_label_, warning_text);
  lv_label_set_text(system_label_, system_text);
  lv_label_set_text(summary_label_, summary_text);
  lv_label_set_text(channels_label_, channels_text);
}

uint16_t InputTestPage::readAnalogPin(uint8_t pin) {
  analogRead(pin);
  delayMicroseconds(20);
  return analogRead(pin);
}

InputTestPage input_test_page;
