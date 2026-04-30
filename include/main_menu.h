#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include <lvgl.h>

#include "app_state.h"

class MainMenu {
 public:
  MainMenu();

  void render();
  void loop();
  void handlePotChange(uint8_t index, float value);
  void handleOkPress();

 private:
  static constexpr uint8_t kTabCount = 8;
  static constexpr uint8_t kActionCount = 4;
  static constexpr uint8_t kSeqButtonCount = 8;
  static constexpr uint8_t kPotCardCount = 5;

  static void tabEventHandler(lv_event_t *event);
  static void actionEventHandler(lv_event_t *event);
  static void seqStepEventHandler(lv_event_t *event);

  void setPage(PageId page);
  void updateMode();
  void refresh();
  void refreshStatusBar();
  void refreshTabs();
  void refreshPotCards();
  void refreshActionButtons();
  void refreshSequencerButtons();
  void refreshVisibility();
  void handleAction(uint8_t action_index);
  void setSelectedStepValue(bool set_note, float normalized);

  lv_obj_t *root_ = nullptr;
  lv_obj_t *content_panel_ = nullptr;
  lv_obj_t *input_test_panel_ = nullptr;
  lv_obj_t *status_title_label_ = nullptr;
  lv_obj_t *status_label_ = nullptr;
  lv_obj_t *pot_row_ = nullptr;
  lv_obj_t *pot_cards_[kPotCardCount] = {nullptr, nullptr, nullptr, nullptr, nullptr};
  lv_obj_t *pot_name_labels_[kPotCardCount] = {nullptr, nullptr, nullptr, nullptr, nullptr};
  lv_obj_t *pot_value_labels_[kPotCardCount] = {nullptr, nullptr, nullptr, nullptr, nullptr};
  lv_obj_t *action_buttons_[kActionCount] = {nullptr, nullptr, nullptr, nullptr};
  lv_obj_t *action_labels_[kActionCount] = {nullptr, nullptr, nullptr, nullptr};
  lv_obj_t *seq_info_label_ = nullptr;
  lv_obj_t *seq_step_buttons_[kSeqButtonCount] = {nullptr, nullptr, nullptr, nullptr,
                                                  nullptr, nullptr, nullptr, nullptr};
  lv_obj_t *seq_step_labels_[kSeqButtonCount] = {nullptr, nullptr, nullptr, nullptr,
                                                 nullptr, nullptr, nullptr, nullptr};
  lv_obj_t *tab_buttons_[kTabCount] = {nullptr, nullptr, nullptr, nullptr,
                                       nullptr, nullptr, nullptr, nullptr};
  lv_obj_t *tab_labels_[kTabCount] = {nullptr, nullptr, nullptr, nullptr,
                                      nullptr, nullptr, nullptr, nullptr};
};

extern MainMenu main_menu;

#endif
