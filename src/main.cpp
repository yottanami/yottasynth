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
constexpr uint16_t kDisplayWidth = 320;
constexpr uint16_t kDisplayHeight = 240;

XPT2046_Touchscreen ts(kTouchCsPin, kTouchIrqPin);
TFT_eSPI tft = TFT_eSPI(kDisplayWidth, kDisplayHeight);

constexpr size_t kDrawBufferSize =
    (kDisplayWidth * kDisplayHeight / 10U) * (LV_COLOR_DEPTH / 8U);
DMAMEM uint32_t draw_buffer[kDrawBufferSize / sizeof(uint32_t)];

unsigned long last_lv_tick_ms = 0;
bool input_test_started = false;
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

  const bool touched = ts.touched();
  TS_Point p = ts.getPoint();

  int x = map(p.y, 400, 3829, 0, kDisplayWidth - 1);
  int y = map(p.x, 540, 3756, 0, kDisplayHeight - 1);

  x = constrain(x, 0, kDisplayWidth - 1);
  y = constrain(y, 0, kDisplayHeight - 1);

  input_test_page.updateTouch(touched, x, y, p.x, p.y);

  if (!touched) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

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
  play_mode.setup();
  performance_engine.begin(&synth);

  last_lv_tick_ms = millis();
}

void loop() {
  AppState::instance().refreshTransientStatus(millis());
  const Mode mode = Settings::getInstance()->getMode();

  if (mode == Mode::INPUT_TEST) {
    input_test_page.setMuxEnabled(true);
    if (!input_test_started) {
      input_test_page.begin();
      input_test_started = true;
    }
    input_test_page.loop();
  } else {
    input_test_page.setMuxEnabled(false);
    input_test_started = false;

    control_input.update();
    for (uint8_t index = 0; index < ControlInput::kPotCount; ++index) {
      float value = 0.0f;
      if (control_input.consumePotChange(index, value)) {
        main_menu.handlePotChange(index, value);
      }
    }

    if (control_input.consumeOkPress()) {
      main_menu.handleOkPress();
    }

    play_mode.loop();
    performance_engine.update();
    synth.applyPatch(AppState::instance().patch);
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
