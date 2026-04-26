#include <lvgl.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

//#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
//#endif

#include "main_menu.h"
#include "audio_setup.h"
#include "synth.h"
#include "play_mode.h"
// define play_mode
PlayMode play_mode;

extern "C" void _reboot_Teensyduino_(void);

// XPT2046_Touchscreen
#define CS_PIN  5
#define TIRQ_PIN  4

#define TFT_HOR_RES   320
#define TFT_VER_RES   240

XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);
TFT_eSPI tft = TFT_eSPI(TFT_HOR_RES, TFT_VER_RES);

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

namespace {

constexpr int kMuxSelectPins[] = {14, 15, 16, 17};
constexpr int kMuxSignalPin = 22;
constexpr int kMuxChannelCount = 8;
constexpr unsigned long kPotPrintIntervalMs = 200;
constexpr int kPotSamplesPerChannel = 4;
constexpr bool kEnableAudioInit = false;

bool touchIrqActive()
{
  return digitalRead(TIRQ_PIN) == LOW;
}

void selectMuxChannel(const int channel)
{
  for (int bit = 0; bit < 4; ++bit) {
    digitalWrite(kMuxSelectPins[bit], bitRead(channel, bit));
  }
}

int readMuxChannel(const int channel)
{
  selectMuxChannel(channel);
  delayMicroseconds(50);

  // Discard the first conversion after switching the mux so the ADC
  // sample-and-hold capacitor can settle on the new channel.
  analogRead(kMuxSignalPin);
  delayMicroseconds(10);

  int total = 0;
  for (int sample = 0; sample < kPotSamplesPerChannel; ++sample) {
    total += analogRead(kMuxSignalPin);
  }

  return total / kPotSamplesPerChannel;
}

}  // namespace

// initialize Synth and store it is lead_synth variable
Synth synth(&lead_waveform1, &lead_waveform2, &lead_pink, &lead_filter, &lead_envelope);

#if LV_USE_LOG != 0
void print_logs( lv_log_level_t level, const char * buf )
{
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}
#endif

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t * px_map)
{
  uint32_t w = ( area->x2 - area->x1 + 1 );
  uint32_t h = ( area->y2 - area->y1 + 1 );

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)px_map, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}


void my_touchpad_read( lv_indev_t * indev, lv_indev_data_t * data ) 
{
  LV_UNUSED(indev);

  data->state = LV_INDEV_STATE_RELEASED;

  // The XPT2046 T_IRQ line is active-low and needs a pull-up.
  // When the screen is disconnected, this keeps the pin from floating
  // and avoids bogus touch reads flooding the serial monitor.
  if (!touchIrqActive()) {
    return;
  }

  if (!ts.touched()) {
    return;
  }

  const TS_Point p = ts.getPoint();
  const int x = constrain(map(p.y, 400, 3829, 1, TFT_VER_RES), 0, TFT_VER_RES - 1);
  const int y = constrain(map(p.x, 540, 3756, 1, TFT_HOR_RES), 0, TFT_HOR_RES - 1);

  data->state = LV_INDEV_STATE_PRESSED;
  data->point.x = x;
  data->point.y = y;
}

void setup()
{
  Serial.begin( 115200 );

  // TODO: move display settings to a seperate function
  lv_init();
  tft.begin(); /* TFT init */
  tft.setRotation(3); /* Landscape orientation */
  ts.begin();
  pinMode(TIRQ_PIN, INPUT_PULLUP);
  delay(100);
  //tft.setRotation(0);
  ts.setRotation(0);
  
  /* register print function for debugging */
  #if LV_USE_LOG != 0
    lv_log_register_print_cb( print_logs );
  #endif

  lv_display_t * disp;
  #if LV_USE_TFT_ESPI
    disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
  #else
    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
  #endif

  /* Initialize the (dummy) input device driver */
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);
 
  main_menu.render();

  if (kEnableAudioInit) {
    setupAudio();
  } else {
    Serial.println("Audio init disabled for mux debug.");
  }
    
  synth.setup();
  play_mode.setup();
  
  Serial.println( "Setup done" );

  analogReadResolution(10);
  analogReadAveraging(8);

  for (const int pin : kMuxSelectPins) {
    pinMode(pin, OUTPUT);
  }
  pinMode(kMuxSignalPin, INPUT);
  selectMuxChannel(0);

}

void loop()
{
  // Poll MIDI from USB host and dispatch to Synth
  //play_mode.loop();

  if (Serial.available()) {
    const int c = Serial.read();
    if (c == '!') {
      Serial.println("Rebooting to bootloader...");
      Serial.flush();
      delay(10);
      _reboot_Teensyduino_();
    }
  }

  static unsigned long lastPotPrintMs = 0;
  const unsigned long now = millis();
  if (now - lastPotPrintMs >= kPotPrintIntervalMs) {
    lastPotPrintMs = now;

    for (int channel = 0; channel < kMuxChannelCount; ++channel) {
      if (channel > 0) {
        Serial.print(" | ");
      }

      Serial.print("C");
      Serial.print(channel);
      Serial.print(": ");
      Serial.print(readMuxChannel(channel));
    }
    Serial.println();
  }

  // Update synth internals (LFO, envelopes, etc.)
  //synth.loop();
  
  // Let the GUI do its work
  lv_task_handler();
  lv_tick_inc(5);
  //usbMIDI.read();
}
