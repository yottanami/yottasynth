#include "input_test_page.h"

#include <stdio.h>

namespace {
constexpr uint8_t kDirectPinCount = 19;
constexpr uint8_t kPotChannels[5] = {0, 3, 1, 2, 4};
constexpr const char *kPotNames[5] = {"RV1/C0", "RV2/C3", "RV3/C1", "RV4/C2", "RV5/C4"};
constexpr uint16_t kAdcMaxValue = 4095;
constexpr uint8_t kDirectPins[kDirectPinCount] = {8, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 38, 39, 40, 41};
constexpr unsigned long kMuxScanIntervalUs = 1000;
constexpr unsigned long kDirectSampleIntervalMs = 50;
constexpr unsigned long kUiRefreshIntervalMs = 200;
constexpr uint16_t kMuxChangeThreshold = 7;

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
    touch_label_(nullptr),
    warning_label_(nullptr),
    summary_label_(nullptr),
    channels_label_(nullptr),
    touch_pressed_(false),
    touch_x_(0),
    touch_y_(0),
    raw_touch_x_(0),
    raw_touch_y_(0),
    mux_enabled_(true),
    initialized_(false),
    activity_initialized_(false),
    last_touch_pressed_(false),
    mux_input_(0),
    last_mux_scan_us_(0),
    last_direct_sample_ms_(0),
    last_refresh_ms_(0) {
  memset(channel_values_, 0, sizeof(channel_values_));
  memset(channel_min_, 0, sizeof(channel_min_));
  memset(channel_max_, 0, sizeof(channel_max_));
  memset(direct_pin_values_, 0, sizeof(direct_pin_values_));
  memset(direct_pin_min_, 0, sizeof(direct_pin_min_));
  memset(direct_pin_max_, 0, sizeof(direct_pin_max_));
}

void InputTestPage::begin() {
  if (mux_enabled_ && !initialized_) {
    configureMuxPins();
    primeMuxChannels();
    sampleDirectPins();
    resetActivityTracking();
    initialized_ = true;
  }

  refreshLabels();
}

void InputTestPage::setMuxEnabled(bool enabled) {
  mux_enabled_ = enabled;
}

void InputTestPage::configureMuxPins() {
  pinMode(MUX_S0_PIN, OUTPUT);
  pinMode(MUX_S1_PIN, OUTPUT);
  pinMode(MUX_S2_PIN, OUTPUT);
  pinMode(MUX_S3_PIN, OUTPUT);
  pinMode(MUX_SIGNAL_PIN, INPUT);

  digitalWrite(MUX_S0_PIN, LOW);
  digitalWrite(MUX_S1_PIN, LOW);
  digitalWrite(MUX_S2_PIN, LOW);
  digitalWrite(MUX_S3_PIN, LOW);

  analogReadResolution(12);
  analogReadAveraging(8);
}

lv_obj_t * InputTestPage::createPage(lv_obj_t * menu) {
  if (page_ != nullptr) {
    return page_;
  }

  page_ = lv_menu_page_create(menu, NULL);

  lv_obj_t * cont = lv_menu_cont_create(page_);
  touch_label_ = lv_label_create(cont);
  lv_obj_set_width(touch_label_, lv_pct(100));
  lv_label_set_long_mode(touch_label_, LV_LABEL_LONG_WRAP);

  cont = lv_menu_cont_create(page_);
  warning_label_ = lv_label_create(cont);
  lv_obj_set_width(warning_label_, lv_pct(100));
  lv_label_set_long_mode(warning_label_, LV_LABEL_LONG_WRAP);

  cont = lv_menu_cont_create(page_);
  summary_label_ = lv_label_create(cont);
  lv_obj_set_width(summary_label_, lv_pct(100));
  lv_label_set_long_mode(summary_label_, LV_LABEL_LONG_WRAP);

  cont = lv_menu_cont_create(page_);
  channels_label_ = lv_label_create(cont);
  lv_obj_set_width(channels_label_, lv_pct(100));
  lv_label_set_long_mode(channels_label_, LV_LABEL_LONG_WRAP);

  refreshLabels();
  return page_;
}

void InputTestPage::loop() {
  if (mux_enabled_) {
    if (!initialized_) {
      configureMuxPins();
      primeMuxChannels();
      sampleDirectPins();
      resetActivityTracking();
      initialized_ = true;
    }

    if (touch_pressed_ && !last_touch_pressed_) {
      resetActivityTracking();
    }

    uint8_t scan_budget = 0;
    while ((micros() - last_mux_scan_us_) >= kMuxScanIntervalUs && scan_budget < 8) {
      scanMuxStep();
      last_mux_scan_us_ += kMuxScanIntervalUs;
      ++scan_budget;
    }

    const unsigned long now_ms = millis();
    if ((now_ms - last_direct_sample_ms_) >= kDirectSampleIntervalMs) {
      sampleDirectPins();
      last_direct_sample_ms_ = now_ms;
    }
  }

  const unsigned long now = millis();
  if ((now - last_refresh_ms_) >= kUiRefreshIntervalMs) {
    last_refresh_ms_ = now;
    refreshLabels();
  }

  last_touch_pressed_ = touch_pressed_;
}

void InputTestPage::updateTouch(bool pressed, int16_t x, int16_t y, int16_t raw_x, int16_t raw_y) {
  touch_pressed_ = pressed;
  touch_x_ = x;
  touch_y_ = y;
  raw_touch_x_ = raw_x;
  raw_touch_y_ = raw_y;
}

void InputTestPage::primeMuxChannels() {
  mux_input_ = 0;
  selectChannel(mux_input_);
  delayMicroseconds(400);

  for (uint8_t channel = 0; channel < MUX_TOTAL_CHANNEL_COUNT; ++channel) {
    analogRead(MUX_SIGNAL_PIN);
    delayMicroseconds(120);
    channel_values_[channel] = analogRead(MUX_SIGNAL_PIN);

    if (channel + 1U < MUX_TOTAL_CHANNEL_COUNT) {
      mux_input_ = channel + 1U;
      selectChannel(mux_input_);
      delayMicroseconds(400);
    }
  }

  mux_input_ = 0;
  selectChannel(mux_input_);
  last_mux_scan_us_ = micros();
  last_direct_sample_ms_ = millis();
}

void InputTestPage::scanMuxStep() {
  const uint16_t mux_read = analogRead(MUX_SIGNAL_PIN);
  const uint8_t channel = mux_input_;
  const uint16_t previous = channel_values_[channel];

  if ((mux_read > previous && (mux_read - previous) > kMuxChangeThreshold)
      || (previous > mux_read && (previous - mux_read) > kMuxChangeThreshold)) {
    channel_values_[channel] = mux_read;
    if (activity_initialized_) {
      if (mux_read < channel_min_[channel]) {
        channel_min_[channel] = mux_read;
      }
      if (mux_read > channel_max_[channel]) {
        channel_max_[channel] = mux_read;
      }
    }
  }

  mux_input_ = static_cast<uint8_t>((mux_input_ + 1U) % MUX_TOTAL_CHANNEL_COUNT);
  selectChannel(mux_input_);
}

void InputTestPage::sampleDirectPins() {
  for (uint8_t index = 0; index < DIRECT_PIN_COUNT; ++index) {
    const uint16_t value = readDirectPin(kDirectPins[index]);
    direct_pin_values_[index] = value;
    if (activity_initialized_) {
      if (value < direct_pin_min_[index]) {
        direct_pin_min_[index] = value;
      }
      if (value > direct_pin_max_[index]) {
        direct_pin_max_[index] = value;
      }
    }
  }

  configureMuxPins();
  selectChannel(mux_input_);
}

void InputTestPage::resetActivityTracking() {
  memcpy(channel_min_, channel_values_, sizeof(channel_values_));
  memcpy(channel_max_, channel_values_, sizeof(channel_values_));
  memcpy(direct_pin_min_, direct_pin_values_, sizeof(direct_pin_values_));
  memcpy(direct_pin_max_, direct_pin_values_, sizeof(direct_pin_values_));
  activity_initialized_ = true;
}

void InputTestPage::refreshLabels() {
  if (touch_label_ == nullptr || warning_label_ == nullptr || summary_label_ == nullptr || channels_label_ == nullptr) {
    return;
  }

  char touch_text[96];
  char warning_text[256];
  char summary_text[448];
  char channels_text[1536];

  uint16_t top_span[4] = {0, 0, 0, 0};
  char top_name[4][8] = {"-", "-", "-", "-"};

  for (uint8_t channel = 0; channel < MUX_TOTAL_CHANNEL_COUNT; ++channel) {
    const uint16_t span = spanFromMinMax(channel_min_[channel], channel_max_[channel]);
    for (uint8_t slot = 0; slot < 4; ++slot) {
      if (span > top_span[slot]) {
        for (uint8_t shift = 3; shift > slot; --shift) {
          top_span[shift] = top_span[shift - 1];
          memcpy(top_name[shift], top_name[shift - 1], sizeof(top_name[shift]));
        }
        top_span[slot] = span;
        snprintf(top_name[slot], sizeof(top_name[slot]), "C%02u", channel);
        break;
      }
    }
  }

  for (uint8_t index = 0; index < DIRECT_PIN_COUNT; ++index) {
    const uint16_t span = spanFromMinMax(direct_pin_min_[index], direct_pin_max_[index]);
    for (uint8_t slot = 0; slot < 4; ++slot) {
      if (span > top_span[slot]) {
        for (uint8_t shift = 3; shift > slot; --shift) {
          top_span[shift] = top_span[shift - 1];
          memcpy(top_name[shift], top_name[shift - 1], sizeof(top_name[shift]));
        }
        top_span[slot] = span;
        snprintf(top_name[slot], sizeof(top_name[slot]), "P%02u", kDirectPins[index]);
        break;
      }
    }
  }

  snprintf(
      touch_text,
      sizeof(touch_text),
      "Touch: %s X:%d Y:%d\nRaw: X:%d Y:%d",
      touch_pressed_ ? "DOWN" : "UP",
      touch_x_,
      touch_y_,
      raw_touch_x_,
      raw_touch_y_);

  snprintf(
      warning_text,
      sizeof(warning_text),
      mux_enabled_
          ? "Probe mode is active.\nUsing checkMux-style scanning on KiCad pins: COM=A9 S0=A5 S1=A6 S2=A7 S3=A8.\nTap the screen once to reset activity ranges, then move one knob."
          : "Mux test is disabled because audio has already started.\nPins A6, A7 and A9 overlap Teensy audio pins.\nReboot and open INPUT TEST before entering SYNTHESIZER.");

  if (mux_enabled_) {
    snprintf(
        summary_text,
        sizeof(summary_text),
        "KiCad pot guess 0..100\n%s:%3u  %s:%3u\n%s:%3u  %s:%3u\n%s:%3u\n\nTop movers since reset\n%s d%u  %s d%u\n%s d%u  %s d%u",
        kPotNames[0], rawToPercent(channel_values_[kPotChannels[0]]),
        kPotNames[1], rawToPercent(channel_values_[kPotChannels[1]]),
        kPotNames[2], rawToPercent(channel_values_[kPotChannels[2]]),
        kPotNames[3], rawToPercent(channel_values_[kPotChannels[3]]),
        kPotNames[4], rawToPercent(channel_values_[kPotChannels[4]]),
        top_name[0], top_span[0], top_name[1], top_span[1],
        top_name[2], top_span[2], top_name[3], top_span[3]);

    snprintf(
        channels_text,
        sizeof(channels_text),
        "MUX raw / delta\n"
        "C00:%4u d%-4u C01:%4u d%-4u\n"
        "C02:%4u d%-4u C03:%4u d%-4u\n"
        "C04:%4u d%-4u C05:%4u d%-4u\n"
        "C06:%4u d%-4u C07:%4u d%-4u\n"
        "C08:%4u d%-4u C09:%4u d%-4u\n"
        "C10:%4u d%-4u C11:%4u d%-4u\n"
        "C12:%4u d%-4u C13:%4u d%-4u\n"
        "C14:%4u d%-4u C15:%4u d%-4u\n\n"
        "Direct analog raw / delta\n"
        "P08:%4u d%-4u P14:%4u d%-4u\n"
        "P15:%4u d%-4u P16:%4u d%-4u\n"
        "P17:%4u d%-4u P18:%4u d%-4u\n"
        "P19:%4u d%-4u P20:%4u d%-4u\n"
        "P21:%4u d%-4u P22:%4u d%-4u\n"
        "P23:%4u d%-4u P24:%4u d%-4u\n"
        "P25:%4u d%-4u P26:%4u d%-4u\n"
        "P27:%4u d%-4u P38:%4u d%-4u\n"
        "P39:%4u d%-4u P40:%4u d%-4u\n"
        "P41:%4u d%-4u",
        channel_values_[0], spanFromMinMax(channel_min_[0], channel_max_[0]),
        channel_values_[1], spanFromMinMax(channel_min_[1], channel_max_[1]),
        channel_values_[2], spanFromMinMax(channel_min_[2], channel_max_[2]),
        channel_values_[3], spanFromMinMax(channel_min_[3], channel_max_[3]),
        channel_values_[4], spanFromMinMax(channel_min_[4], channel_max_[4]),
        channel_values_[5], spanFromMinMax(channel_min_[5], channel_max_[5]),
        channel_values_[6], spanFromMinMax(channel_min_[6], channel_max_[6]),
        channel_values_[7], spanFromMinMax(channel_min_[7], channel_max_[7]),
        channel_values_[8], spanFromMinMax(channel_min_[8], channel_max_[8]),
        channel_values_[9], spanFromMinMax(channel_min_[9], channel_max_[9]),
        channel_values_[10], spanFromMinMax(channel_min_[10], channel_max_[10]),
        channel_values_[11], spanFromMinMax(channel_min_[11], channel_max_[11]),
        channel_values_[12], spanFromMinMax(channel_min_[12], channel_max_[12]),
        channel_values_[13], spanFromMinMax(channel_min_[13], channel_max_[13]),
        channel_values_[14], spanFromMinMax(channel_min_[14], channel_max_[14]),
        channel_values_[15], spanFromMinMax(channel_min_[15], channel_max_[15]),
        direct_pin_values_[0], spanFromMinMax(direct_pin_min_[0], direct_pin_max_[0]),
        direct_pin_values_[1], spanFromMinMax(direct_pin_min_[1], direct_pin_max_[1]),
        direct_pin_values_[2], spanFromMinMax(direct_pin_min_[2], direct_pin_max_[2]),
        direct_pin_values_[3], spanFromMinMax(direct_pin_min_[3], direct_pin_max_[3]),
        direct_pin_values_[4], spanFromMinMax(direct_pin_min_[4], direct_pin_max_[4]),
        direct_pin_values_[5], spanFromMinMax(direct_pin_min_[5], direct_pin_max_[5]),
        direct_pin_values_[6], spanFromMinMax(direct_pin_min_[6], direct_pin_max_[6]),
        direct_pin_values_[7], spanFromMinMax(direct_pin_min_[7], direct_pin_max_[7]),
        direct_pin_values_[8], spanFromMinMax(direct_pin_min_[8], direct_pin_max_[8]),
        direct_pin_values_[9], spanFromMinMax(direct_pin_min_[9], direct_pin_max_[9]),
        direct_pin_values_[10], spanFromMinMax(direct_pin_min_[10], direct_pin_max_[10]),
        direct_pin_values_[11], spanFromMinMax(direct_pin_min_[11], direct_pin_max_[11]),
        direct_pin_values_[12], spanFromMinMax(direct_pin_min_[12], direct_pin_max_[12]),
        direct_pin_values_[13], spanFromMinMax(direct_pin_min_[13], direct_pin_max_[13]),
        direct_pin_values_[14], spanFromMinMax(direct_pin_min_[14], direct_pin_max_[14]),
        direct_pin_values_[15], spanFromMinMax(direct_pin_min_[15], direct_pin_max_[15]),
        direct_pin_values_[16], spanFromMinMax(direct_pin_min_[16], direct_pin_max_[16]),
        direct_pin_values_[17], spanFromMinMax(direct_pin_min_[17], direct_pin_max_[17]),
        direct_pin_values_[18], spanFromMinMax(direct_pin_min_[18], direct_pin_max_[18]));
  } else {
    snprintf(
        summary_text,
        sizeof(summary_text),
        "Probe values are hidden in this state\nbecause the mux pins are owned by the audio subsystem.");

    snprintf(
        channels_text,
        sizeof(channels_text),
        "Use this flow:\n1. Reboot the Teensy\n2. Open INPUT TEST first\n3. Move the knobs\n4. Enter SYNTHESIZER after testing");
  }

  lv_label_set_text(touch_label_, touch_text);
  lv_label_set_text(warning_label_, warning_text);
  lv_label_set_text(summary_label_, summary_text);
  lv_label_set_text(channels_label_, channels_text);
}

void InputTestPage::selectChannel(uint8_t channel) const {
  digitalWrite(MUX_S0_PIN, bitRead(channel, 0));
  digitalWrite(MUX_S1_PIN, bitRead(channel, 1));
  digitalWrite(MUX_S2_PIN, bitRead(channel, 2));
  digitalWrite(MUX_S3_PIN, bitRead(channel, 3));
}

uint16_t InputTestPage::readDirectPin(uint8_t pin) {
  pinMode(pin, INPUT);
  analogRead(pin);
  delayMicroseconds(80);
  return analogRead(pin);
}

InputTestPage input_test_page;
