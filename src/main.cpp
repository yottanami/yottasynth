#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>

#include "app_state.h"
#include "audio_setup.h"
#include "control_input.h"
#include "input_test_page.h"
#include "main_menu.h"
#include "performance_engine.h"
#include "play_mode.h"
#include "settings.h"
#include "synth.h"

extern "C" void _reboot_Teensyduino_(void);

namespace {
constexpr uint8_t kTouchCsPin = 5;
constexpr uint8_t kTouchIrqPin = 4;
constexpr uint16_t kDisplayWidth = 480;
constexpr uint16_t kDisplayHeight = 320;
constexpr int16_t kTouchRawXMin = 540;
constexpr int16_t kTouchRawXMax = 3756;
constexpr int16_t kTouchRawYMin = 400;
constexpr int16_t kTouchRawYMax = 3829;
constexpr int16_t kTouchRawMargin = 100;

XPT2046_Touchscreen ts(kTouchCsPin, kTouchIrqPin);
TFT_eSPI tft = TFT_eSPI(kDisplayWidth, kDisplayHeight);

constexpr size_t kDrawBufferSize =
    (kDisplayWidth * kDisplayHeight / 10U) * (LV_COLOR_DEPTH / 8U);
DMAMEM uint32_t draw_buffer[kDrawBufferSize / sizeof(uint32_t)];

unsigned long last_lv_tick_ms = 0;
bool input_test_started = false;
uint8_t touch_press_streak = 0;
uint8_t touch_release_streak = 0;
uint8_t touch_move_streak = 0;
bool touch_confirmed = false;
int16_t last_touch_x = 0;
int16_t last_touch_y = 0;
int16_t last_raw_touch_x = 0;
int16_t last_raw_touch_y = 0;
int16_t last_raw_touch_z = 0;
int16_t pending_touch_x = 0;
int16_t pending_touch_y = 0;
int16_t pending_raw_touch_x = 0;
int16_t pending_raw_touch_y = 0;
int16_t pending_raw_touch_z = 0;

constexpr uint8_t kTouchConfirmCount = 2;
constexpr uint8_t kTouchReleaseCount = 1;
constexpr int16_t kTouchPressurePress = 380;
constexpr int16_t kTouchPressureRelease = 180;
constexpr uint8_t kTouchMoveConfirmCount = 2;
constexpr int16_t kTouchConfirmWindowPx = 18;
constexpr int16_t kTouchMoveWindowPx = 40;

bool touchPointInRange(const TS_Point &point) {
  return point.x >= (kTouchRawXMin - kTouchRawMargin) &&
         point.x <= (kTouchRawXMax + kTouchRawMargin) &&
         point.y >= (kTouchRawYMin - kTouchRawMargin) &&
         point.y <= (kTouchRawYMax + kTouchRawMargin);
}

bool touchPointsClose(int16_t ax, int16_t ay, int16_t bx, int16_t by, int16_t max_delta) {
  return abs(ax - bx) <= max_delta && abs(ay - by) <= max_delta;
}

void storePendingTouch(int16_t x, int16_t y, const TS_Point &point) {
  pending_touch_x = x;
  pending_touch_y = y;
  pending_raw_touch_x = point.x;
  pending_raw_touch_y = point.y;
  pending_raw_touch_z = point.z;
}

void commitTouchSample(int16_t x, int16_t y, const TS_Point &point) {
  last_touch_x = x;
  last_touch_y = y;
  last_raw_touch_x = point.x;
  last_raw_touch_y = point.y;
  last_raw_touch_z = point.z;
  touch_move_streak = 0;
}

void holdLastTouch(lv_indev_data_t *data) {
  input_test_page.updateTouch(true, last_touch_x, last_touch_y, last_raw_touch_x,
                              last_raw_touch_y, last_raw_touch_z);
  data->state = LV_INDEV_STATE_PRESSED;
  data->point.x = last_touch_x;
  data->point.y = last_touch_y;
}

void releaseTouch(lv_indev_data_t *data, bool clear_debug_touch) {
  touch_press_streak = 0;
  touch_release_streak = 0;
  touch_move_streak = 0;
  touch_confirmed = false;
  last_raw_touch_z = 0;
  pending_raw_touch_z = 0;
  if (clear_debug_touch) {
    input_test_page.updateTouch(false, last_touch_x, last_touch_y, last_raw_touch_x,
                                last_raw_touch_y, 0);
  }
  data->state = LV_INDEV_STATE_RELEASED;
}
}

PlayMode play_mode;
Synth synth(&lead_waveform1, &lead_waveform2, &lead_pink, &lead_filter, &lead_envelope);

#if LV_USE_LOG != 0
void print_logs(lv_log_level_t level, const char *buffer) {
  LV_UNUSED(level);
  Serial.println(buffer);
  Serial.flush();
}
#endif

void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  LV_UNUSED(indev);

  const bool irq_active = ts.tirqTouched();
  if (!irq_active) {
    if (touch_confirmed && touch_release_streak + 1U < kTouchReleaseCount) {
      ++touch_release_streak;
      holdLastTouch(data);
      return;
    }

    releaseTouch(data, true);
    return;
  }

  if (!touch_confirmed && !ts.touched()) {
    releaseTouch(data, true);
    return;
  }

  const TS_Point p = ts.getPoint();
  const int16_t pressure_threshold =
      touch_confirmed ? kTouchPressureRelease : kTouchPressurePress;
  const bool valid_touch =
      touchPointInRange(p) && p.z >= pressure_threshold;
  if (!valid_touch) {
    if (touch_confirmed && touch_release_streak + 1U < kTouchReleaseCount) {
      ++touch_release_streak;
      holdLastTouch(data);
      return;
    }

    releaseTouch(data, true);
    return;
  }

  touch_release_streak = 0;

  int x = map(p.y, kTouchRawYMin, kTouchRawYMax, 0, kDisplayWidth - 1);
  int y = map(p.x, kTouchRawXMin, kTouchRawXMax, 0, kDisplayHeight - 1);

  x = constrain(x, 0, kDisplayWidth - 1);
  y = constrain(y, 0, kDisplayHeight - 1);

  if (!touch_confirmed) {
    if (touch_press_streak == 0 ||
        touchPointsClose(x, y, pending_touch_x, pending_touch_y, kTouchConfirmWindowPx)) {
      if (touch_press_streak < kTouchConfirmCount) {
        ++touch_press_streak;
      }
    } else {
      touch_press_streak = 1;
    }

    storePendingTouch(x, y, p);
    input_test_page.updateTouch(false, x, y, p.x, p.y, p.z);
    if (touch_press_streak < kTouchConfirmCount) {
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }

    touch_confirmed = true;
  } else if (!touchPointsClose(x, y, last_touch_x, last_touch_y, kTouchMoveWindowPx)) {
    if (touch_move_streak == 0 ||
        touchPointsClose(x, y, pending_touch_x, pending_touch_y, kTouchConfirmWindowPx)) {
      if (touch_move_streak < kTouchMoveConfirmCount) {
        ++touch_move_streak;
      }
    } else {
      touch_move_streak = 1;
    }

    storePendingTouch(x, y, p);
    input_test_page.updateTouch(true, x, y, p.x, p.y, p.z);
    if (touch_move_streak < kTouchMoveConfirmCount) {
      holdLastTouch(data);
      return;
    }
  } else {
    touch_move_streak = 0;
  }

  commitTouchSample(x, y, p);
  input_test_page.updateTouch(true, x, y, p.x, p.y, p.z);
  data->state = LV_INDEV_STATE_PRESSED;
  data->point.x = x;
  data->point.y = y;
}

void my_disp_flush(lv_display_t *display, const lv_area_t *area, uint8_t *px_map) {
  LV_UNUSED(display);

  const uint32_t width = area->x2 - area->x1 + 1U;
  const uint32_t height = area->y2 - area->y1 + 1U;

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, width, height);
  tft.pushColors(reinterpret_cast<uint16_t *>(px_map), width * height, true);
  tft.endWrite();

  lv_display_flush_ready(display);
}

void setup() {
  Serial.begin(115200);

  lv_init();
  tft.begin();
  tft.setRotation(3);
  ts.begin();
  ts.setRotation(0);
  pinMode(kTouchIrqPin, INPUT_PULLUP);

#if LV_USE_LOG != 0
  lv_log_register_print_cb(print_logs);
#endif

  lv_display_t *display = lv_display_create(kDisplayWidth, kDisplayHeight);
  lv_display_set_flush_cb(display, my_disp_flush);
  lv_display_set_buffers(display, draw_buffer, nullptr, sizeof(draw_buffer),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);

  main_menu.render();

  control_input.begin();
  const bool audio_codec_ready = setupAudio();
  AppState::instance().updateAudioStatus(audio_codec_ready, false);
  synth.setup();
  setOutputVolume(AppState::instance().audio.output_volume);
  play_mode.setup();
  performance_engine.begin(&synth);

  last_lv_tick_ms = millis();
}

void loop() {
  AppState::instance().refreshTransientStatus(millis());
  const Mode mode = Settings::getInstance()->getMode();

  if (mode == Mode::INPUT_TEST) {
    input_test_page.setProbeEnabled(true);
    if (!input_test_started) {
      input_test_page.begin();
      input_test_started = true;
    }
    input_test_page.loop();
  } else {
    input_test_page.setProbeEnabled(false);
    input_test_started = false;

    control_input.update();
    for (uint8_t index = 0; index < ControlInput::kPotCount; ++index) {
      float value = 0.0f;
      if (control_input.consumePotChange(index, value)) {
        main_menu.handlePotChange(index, value);
      }
    }

    play_mode.loop();
    performance_engine.update();
    synth.applyPatch(AppState::instance().patch);
    applyFxState(AppState::instance().fx);
    synth.loop();
    AppState::instance().updateAudioStatus(AppState::instance().audio.codec_ready,
                                           synth.isSelfTestActive());
  }

  if (Serial.available()) {
    const int c = Serial.read();
    if (c == '!') {
      Serial.println("Rebooting to bootloader...");
      Serial.flush();
      delay(10);
      _reboot_Teensyduino_();
    }
  }

  main_menu.loop();

  const unsigned long now_ms = millis();
  const unsigned long elapsed_ms = now_ms - last_lv_tick_ms;
  if (elapsed_ms > 0) {
    lv_tick_inc(elapsed_ms);
    last_lv_tick_ms = now_ms;
  }
  lv_timer_handler();
}
