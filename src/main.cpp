#include <lvgl.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

//#if LV_USE_TFT_ESPI
#include <TFT_eSPI.h>
//#endif

#include "main_menu.h"
#include "input_test_page.h"
#include "audio_setup.h"
#include "settings.h"
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

namespace {
bool audio_ready = false;
constexpr bool kSerialInputDebug = false;
constexpr uint8_t kDebugMuxS0Pin = A5;
constexpr uint8_t kDebugMuxS1Pin = A6;
constexpr uint8_t kDebugMuxS2Pin = A7;
constexpr uint8_t kDebugMuxS3Pin = A8;
constexpr uint8_t kDebugMuxSignalPin = A9;
constexpr uint8_t kDebugDirectPins[] = {14, 15, 16, 17, 18, 24, 25, 26, 27, 38, 39, 40, 41};
unsigned long last_debug_dump_ms = 0;

bool modeUsesAudio(Mode mode) {
  return mode == Mode::SYNTHESIZER
      || mode == Mode::SEQUENCER
      || mode == Mode::ARPEGGIATOR;
}

void ensureAudioReady() {
  if (audio_ready) {
    return;
  }

  setupAudio();
  synth.setup();
  play_mode.setup();
  audio_ready = true;
  Serial.println("Audio and MIDI initialized");
}

void configureSerialInputDebugPins() {
  pinMode(kDebugMuxS0Pin, OUTPUT);
  pinMode(kDebugMuxS1Pin, OUTPUT);
  pinMode(kDebugMuxS2Pin, OUTPUT);
  pinMode(kDebugMuxS3Pin, OUTPUT);
  pinMode(kDebugMuxSignalPin, INPUT);

  digitalWrite(kDebugMuxS0Pin, LOW);
  digitalWrite(kDebugMuxS1Pin, LOW);
  digitalWrite(kDebugMuxS2Pin, LOW);
  digitalWrite(kDebugMuxS3Pin, LOW);

  analogReadResolution(12);
  analogReadAveraging(8);
}

void selectDebugMuxChannel(uint8_t channel) {
  digitalWrite(kDebugMuxS0Pin, bitRead(channel, 0));
  digitalWrite(kDebugMuxS1Pin, bitRead(channel, 1));
  digitalWrite(kDebugMuxS2Pin, bitRead(channel, 2));
  digitalWrite(kDebugMuxS3Pin, bitRead(channel, 3));
}

uint16_t readDebugAnalogPin(uint8_t pin) {
  analogRead(pin);
  delayMicroseconds(100);
  return analogRead(pin);
}

uint16_t readDebugMuxChannel(uint8_t channel) {
  static constexpr uint8_t kDiscardCount = 8;
  static constexpr uint8_t kSampleCount = 9;
  uint16_t samples[kSampleCount];

  selectDebugMuxChannel(channel);
  delayMicroseconds(2500);

  for (uint8_t discard = 0; discard < kDiscardCount; ++discard) {
    analogRead(kDebugMuxSignalPin);
    delayMicroseconds(120);
  }

  for (uint8_t sample = 0; sample < kSampleCount; ++sample) {
    samples[sample] = analogRead(kDebugMuxSignalPin);
    delayMicroseconds(120);
  }

  for (uint8_t outer = 0; outer + 1U < kSampleCount; ++outer) {
    for (uint8_t inner = outer + 1U; inner < kSampleCount; ++inner) {
      if (samples[inner] < samples[outer]) {
        const uint16_t temp = samples[outer];
        samples[outer] = samples[inner];
        samples[inner] = temp;
      }
    }
  }

  return samples[kSampleCount / 2U];
}

void printSerialInputDebugDump() {
  static constexpr uint8_t kGuessPotChannels[5] = {0, 3, 1, 2, 4};
  static constexpr const char *kGuessPotNames[5] = {"RV1/C0", "RV2/C3", "RV3/C1", "RV4/C2", "RV5/C4"};

  Serial.println();
  Serial.print("=== serial input debug @ ");
  Serial.print(millis());
  Serial.println(" ms ===");

  Serial.print("guess ");
  for (uint8_t index = 0; index < 5; ++index) {
    Serial.print(kGuessPotNames[index]);
    Serial.print('=');
    Serial.print(readDebugMuxChannel(kGuessPotChannels[index]));
    if (index + 1U < 5) {
      Serial.print("  ");
    }
  }
  Serial.println();

  Serial.println("mux scan A5/A6/A7/A8 -> A9");
  for (uint8_t channel = 0; channel < 16; ++channel) {
    if (channel == 8) {
      Serial.println();
    }
    Serial.print('C');
    if (channel < 10) {
      Serial.print('0');
    }
    Serial.print(channel);
    Serial.print('=');
    Serial.print(readDebugMuxChannel(channel));
    Serial.print("  ");
  }
  Serial.println();

  Serial.println("direct analog pins");
  for (uint8_t index = 0; index < sizeof(kDebugDirectPins); ++index) {
    const uint8_t pin = kDebugDirectPins[index];
    Serial.print('P');
    Serial.print(pin);
    Serial.print('=');
    Serial.print(readDebugAnalogPin(pin));
    Serial.print("  ");
    if ((index + 1U) % 4U == 0U) {
      Serial.println();
    }
  }
  Serial.println();
}

void setupSerialInputDebug() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    ;
  }

  configureSerialInputDebugPins();
  Serial.println();
  Serial.println("TEMP SERIAL INPUT DEBUG");
  Serial.println("This bypasses the UI and audio.");
  Serial.println("Rotate one knob at a time and capture the output from pio device monitor.");
  Serial.println("Expected KiCad guess: RV1/C0 RV2/C3 RV3/C1 RV4/C2 RV5/C4");
}

void loopSerialInputDebug() {
  if (millis() - last_debug_dump_ms < 700) {
    return;
  }

  last_debug_dump_ms = millis();
  printSerialInputDebugDump();
}
}  // namespace

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

  bool touched = ts.touched();
  TS_Point p = ts.getPoint();
  
  int x = map(p.y, 400, 3829, 0, TFT_HOR_RES - 1);
  int y = map(p.x, 540, 3756, 0, TFT_VER_RES - 1);

  x = constrain(x, 0, TFT_HOR_RES - 1);
  y = constrain(y, 0, TFT_VER_RES - 1);

  LV_UNUSED(indev);
  input_test_page.updateTouch(touched, x, y, p.x, p.y);
  
  if(!touched) {    
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
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
  if (kSerialInputDebug) {
    setupSerialInputDebug();
    return;
  }

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
  input_test_page.setMuxEnabled(true);
  
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
  if (kSerialInputDebug) {
    loopSerialInputDebug();
    return;
  }

  static Mode last_mode = Mode::MENU;
  const Mode mode = Settings::getInstance()->getMode();

  if (modeUsesAudio(mode)) {
    ensureAudioReady();
  }

  input_test_page.setMuxEnabled(!audio_ready);

  if (mode == Mode::INPUT_TEST && last_mode != Mode::INPUT_TEST) {
    input_test_page.begin();
  }

  if (mode == Mode::INPUT_TEST) {
    input_test_page.loop();
  }

  if (audio_ready) {
    // Poll MIDI from USB host and dispatch to Synth
    play_mode.loop();
    // Update synth internals (LFO, envelopes, etc.)
    synth.loop();
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

  last_mode = mode;
}
