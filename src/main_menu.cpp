# include "Arduino.h"
#include <lvgl.h>
# include "main_menu.h"
#include "settings.h"


MainMenu::MainMenu(){
}

void MainMenu::eventHandler(lv_event_t * event){
  if (lv_event_get_code(event) == LV_EVENT_PRESSED) {
    lv_obj_t * btn = static_cast<lv_obj_t *>(lv_event_get_target(event));
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    const char * text = lv_label_get_text(label);
    Settings* settings = Settings::getInstance();

    if(strcmp(text, "SYNTHESIZER") == 0){
      settings->setMode(Mode::SYNTHESIZER);
    } else if(strcmp(text, "ARPEGGIATOR") == 0){
      settings->setMode(Mode::ARPEGGIATOR);
    } else if(strcmp(text, "SEQUENCER") == 0){
      settings->setMode(Mode::SEQUENCER);
    }                   
  }
}

void MainMenu::render(){
  /*Create a menu object*/
  lv_obj_t * menu = lv_menu_create(lv_scr_act());
  lv_obj_set_size(menu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
  lv_obj_center(menu);

  lv_obj_t * cont;
  lv_obj_t * label;

  lv_obj_t * synth_page = lv_menu_page_create(menu, NULL);

  cont = lv_menu_cont_create(synth_page);
  label = lv_label_create(cont);
  lv_label_set_text(label, "SYNTH PAGE");

  lv_obj_t * arpeggiator_page = lv_menu_page_create(menu, NULL);

  cont = lv_menu_cont_create(arpeggiator_page);
  label = lv_label_create(cont);
  lv_label_set_text(label, "ARPEGGIATOR PAGE");

  lv_obj_t * sequencer_page = lv_menu_page_create(menu, NULL);

  cont = lv_menu_cont_create(sequencer_page);
  label = lv_label_create(cont);
  lv_label_set_text(label, "SEQUENCER PAGE");
    
  /*Create a main page*/
  lv_obj_t * main_page = lv_menu_page_create(menu, NULL);

  cont = lv_menu_cont_create(main_page);
  label = lv_label_create(cont);
  lv_label_set_text(label, "SYNTHESIZER");
  lv_menu_set_load_page_event(menu, cont, synth_page);
  lv_obj_add_event_cb(cont, eventHandler, LV_EVENT_PRESSED, NULL);

  cont = lv_menu_cont_create(main_page);
  label = lv_label_create(cont);
  lv_label_set_text(label, "ARPEGGIATOR");
  lv_menu_set_load_page_event(menu, cont, arpeggiator_page);
  lv_obj_add_event_cb(cont, eventHandler, LV_EVENT_PRESSED, NULL);

  cont = lv_menu_cont_create(main_page);
  label = lv_label_create(cont);
  lv_label_set_text(label, "SEQUENCER");
  lv_menu_set_load_page_event(menu, cont, sequencer_page);
  lv_obj_add_event_cb(cont, eventHandler, LV_EVENT_PRESSED, NULL);

  lv_menu_set_page(menu, main_page);
     
}

MainMenu main_menu = MainMenu();
