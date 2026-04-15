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
  bool touched = ts.touched();
  TS_Point p = ts.getPoint();
  
  int x = map(p.y, 400, 3829, 1, TFT_VER_RES);
  int y = map(p.x, 540, 3756, 1, TFT_HOR_RES);

  
  if(!touched) {    
    data->state = LV_INDEV_STATE_RELEASED;
  } else {
    data->state = LV_INDEV_STATE_PRESSED;

    Serial.print("X: ");
    Serial.print(x);

  
    data->point.x = x;
    data->point.y = y;
  }
}

void setup()
{
  Serial.begin( 115200 );

  // TODO: move display settings to a seperate function
  lv_init();
  tft.begin(); /* TFT init */
  tft.setRotation(3); /* Landscape orientation */
  ts.begin();
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

  setupAudio();    
    
  synth.setup();
  play_mode.setup();
  
  Serial.println( "Setup done" );

  pinMode(10, OUTPUT);
  pinMode(20, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(A9, INPUT);

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

  int channel = 1;
  digitalWrite(A5, bitRead(channel, 0));
  digitalWrite(A6, bitRead(channel, 1));
  digitalWrite(A7, bitRead(channel, 2));
  digitalWrite(A8, bitRead(channel, 3));

  int potValue = analogRead(A9);

  //   Print the value to the serial monitor
  Serial.print("Channel ");
  Serial.print(channel);
  Serial.print(": ");
  Serial.println(potValue);

  // Update synth internals (LFO, envelopes, etc.)
  //synth.loop();
  
  // Let the GUI do its work
  lv_task_handler();
  lv_tick_inc(5);
  //usbMIDI.read();
}
