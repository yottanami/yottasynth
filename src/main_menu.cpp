#include "main_menu.h"

#include <math.h>
#include <stdio.h>

#include "audio_setup.h"
#include "input_test_page.h"
#include "performance_engine.h"
#include "settings.h"
#include "synth.h"

extern Synth synth;

namespace {
MainMenu *g_main_menu = nullptr;
AppState &state = AppState::instance();

constexpr PageId kPages[8] = {
    PageId::PLAY, PageId::OSC_MIX, PageId::FILTER_AMP,
    PageId::MOD,  PageId::FX,      PageId::ARP, PageId::SEQ, PageId::SETTINGS,
};

constexpr const char *kTabTitles[8] = {
    "PLAY", "OSC", "FILT", "MOD", "FX", "ARP", "SEQ", "SET",
};

constexpr uint32_t kPageColors[8] = {
    0x2F855A, 0x0EA5A4, 0xD97706, 0x2563EB,
    0x7C3AED, 0x9333EA, 0xDC2626, 0x475569,
};

float clampUnit(float value) {
  if (value < 0.0f) {
    return 0.0f;
  }
  if (value > 1.0f) {
    return 1.0f;
  }
  return value;
}

uint8_t discreteIndex(float normalized, uint8_t option_count) {
  normalized = clampUnit(normalized);
  const uint8_t max_index = option_count > 0 ? option_count - 1 : 0;
  return min<uint8_t>(static_cast<uint8_t>(normalized * option_count), max_index);
}

uint16_t bpmFromNormalized(float normalized) {
  return static_cast<uint16_t>(60 + (clampUnit(normalized) * 140.0f));
}

const char *noteName(uint8_t note) {
  static char buffer[8];
  static const char *kNames[12] = {"C", "C#", "D", "D#", "E", "F",
                                   "F#", "G", "G#", "A", "A#", "B"};
  snprintf(buffer, sizeof(buffer), "%s%u", kNames[note % 12U], (note / 12U) - 1U);
  return buffer;
}

const char *yesNo(bool value) {
  return value ? "ON" : "OFF";
}

OscWave nextOscWave(OscWave wave) {
  switch (wave) {
    case OscWave::SAW:
      return OscWave::SINE;
    case OscWave::SINE:
      return OscWave::SQUARE;
    case OscWave::SQUARE:
      return OscWave::TRIANGLE;
    case OscWave::TRIANGLE:
    default:
      return OscWave::SAW;
  }
}

OscWave previousOscWave(OscWave wave) {
  switch (wave) {
    case OscWave::SAW:
      return OscWave::TRIANGLE;
    case OscWave::SINE:
      return OscWave::SAW;
    case OscWave::SQUARE:
      return OscWave::SINE;
    case OscWave::TRIANGLE:
    default:
      return OscWave::SQUARE;
  }
}

float msFromNormalized(float normalized, float min_ms, float max_ms) {
  return min_ms + (clampUnit(normalized) * (max_ms - min_ms));
}

const char *fxPotName(uint8_t index) {
  static const char *kEchoNames[5] = {"MIX", "TIME", "FDBK", "RATIO", "SMEAR"};
  static const char *kReverbNames[5] = {"MIX", "SIZE", "DAMP", "PRE", "TONE"};
  static const char *kDriveNames[5] = {"MIX", "DRIVE", "TONE", "CRUSH", "LEVEL"};

  switch (state.fx.mode) {
    case FxMode::ECHO:
      return kEchoNames[index];
    case FxMode::REVERB:
      return kReverbNames[index];
    case FxMode::DRIVE:
      return kDriveNames[index];
    default:
      return kEchoNames[index];
  }
}

void formatFxPotValue(char *buffer, size_t size, uint8_t index) {
  switch (state.fx.mode) {
    case FxMode::ECHO:
      if (index == 0) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.fx.echo.mix * 100.0f));
      if (index == 1) snprintf(buffer, size, "%ums", static_cast<unsigned>(msFromNormalized(state.fx.echo.time, 50.0f, 360.0f)));
      if (index == 2) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.fx.echo.feedback * 100.0f));
      if (index == 3) snprintf(buffer, size, "%u%%", static_cast<unsigned>((0.45f + state.fx.echo.ratio * 0.50f) * 100.0f));
      if (index == 4) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.fx.echo.smear * 100.0f));
      break;
    case FxMode::REVERB:
      if (index == 0) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.fx.reverb.mix * 100.0f));
      if (index == 1) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.fx.reverb.size * 100.0f));
      if (index == 2) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.fx.reverb.damping * 100.0f));
      if (index == 3) snprintf(buffer, size, "%ums", static_cast<unsigned>(msFromNormalized(state.fx.reverb.predelay, 8.0f, 150.0f)));
      if (index == 4) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.fx.reverb.tone * 100.0f));
      break;
    case FxMode::DRIVE:
      if (index == 0) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.fx.drive.mix * 100.0f));
      if (index == 1) snprintf(buffer, size, "x%.1f", 1.0f + (state.fx.drive.drive * 13.0f));
      if (index == 2) snprintf(buffer, size, "%.1fk", (4.5f + state.fx.drive.tone * 18.0f));
      if (index == 3) snprintf(buffer, size, "%ubit", static_cast<unsigned>(16 - roundf(state.fx.drive.crush * 10.0f)));
      if (index == 4) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.fx.drive.level * 100.0f));
      break;
    default:
      buffer[0] = '\0';
      break;
  }
}

void formatPotValue(char *buffer, size_t size, uint8_t index) {
  switch (state.ui.page) {
    case PageId::PLAY:
      if (index == 0) snprintf(buffer, size, "%u Hz", static_cast<unsigned>(state.patch.cutoff * 10000.0f));
      if (index == 1) snprintf(buffer, size, "%.1f", 0.7f + (state.patch.resonance * 4.3f));
      if (index == 2) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.patch.glide * 100.0f));
      if (index == 3) snprintf(buffer, size, "%u%%", state.arp.gate);
      if (index == 4) snprintf(buffer, size, "%u BPM", state.transport.bpm);
      break;
    case PageId::OSC_MIX:
      if (index == 0) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.patch.osc1_mix * 100.0f));
      if (index == 1) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.patch.osc2_mix * 100.0f));
      if (index == 2) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.patch.noise_mix * 100.0f));
      if (index == 3) snprintf(buffer, size, "%+d st", (state.patch.octave_index - 2) * 12);
      if (index == 4) snprintf(buffer, size, "%.2f", 0.95f + ((state.patch.detune - 0.5f) * 0.10f));
      break;
    case PageId::FILTER_AMP:
      if (index == 0) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.patch.cutoff * 100.0f));
      if (index == 1) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.patch.resonance * 100.0f));
      if (index == 2) snprintf(buffer, size, "%ums", static_cast<unsigned>(5.0f + state.patch.attack * 2500.0f));
      if (index == 3) snprintf(buffer, size, "%ums", static_cast<unsigned>(20.0f + state.patch.decay * 3000.0f));
      if (index == 4) snprintf(buffer, size, "%ums", static_cast<unsigned>(30.0f + state.patch.release * 3200.0f));
      break;
    case PageId::MOD:
      if (index == 0) snprintf(buffer, size, "%.1f Hz", 0.15f + (state.patch.lfo_rate * 10.0f));
      if (index == 1) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.patch.lfo_depth * 100.0f));
      if (index == 2) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.patch.glide * 100.0f));
      if (index == 3) snprintf(buffer, size, "%.1f st", 1.0f + (state.patch.bend_range * 11.0f));
      if (index == 4) snprintf(buffer, size, "%s", lfoTargetLabel(state.patch.lfo_target));
      break;
    case PageId::FX:
      formatFxPotValue(buffer, size, index);
      break;
    case PageId::ARP:
      if (index == 0) snprintf(buffer, size, "%u BPM", state.transport.bpm);
      if (index == 1) snprintf(buffer, size, "x%u", state.arp.division);
      if (index == 2) snprintf(buffer, size, "%u%%", state.arp.gate);
      if (index == 3) snprintf(buffer, size, "%u oct", state.arp.octave_range);
      if (index == 4) snprintf(buffer, size, "%s", arpModeLabel(state.arp.mode));
      break;
    case PageId::SEQ:
      if (index == 0) snprintf(buffer, size, "%u BPM", state.transport.bpm);
      if (index == 1) snprintf(buffer, size, "%u steps", state.sequencer.length);
      if (index == 2) snprintf(buffer, size, "%u%%", static_cast<unsigned>(state.transport.swing * 100.0f));
      if (index == 3) snprintf(buffer, size, "%s", noteName(state.sequencer.steps[state.sequencer.selected_step].note));
      if (index == 4) snprintf(buffer, size, "%u%%", state.sequencer.steps[state.sequencer.selected_step].gate);
      break;
    case PageId::SETTINGS:
      if (index == 0) {
        snprintf(buffer, size, "%u%%",
                 static_cast<unsigned>(roundf(state.audio.output_volume * 100.0f)));
      } else if (index == 1) {
        snprintf(buffer, size, "%s", tuningLabel(state.patch.tuning));
      } else {
        buffer[0] = '\0';
      }
      break;
  }
}

const char *potName(uint8_t index) {
  static const char *kPlayNames[5] = {"CUT", "RES", "GLIDE", "A-GATE", "BPM"};
  static const char *kOscNames[5] = {"OSC1", "OSC2", "NOISE", "OCT", "DETUNE"};
  static const char *kFilterNames[5] = {"CUT", "RES", "ATT", "DEC", "REL"};
  static const char *kModNames[5] = {"RATE", "DEPTH", "GLIDE", "BEND", "TARGET"};
  static const char *kArpNames[5] = {"BPM", "DIV", "GATE", "OCT", "MODE"};
  static const char *kSeqNames[5] = {"BPM", "LEN", "SWING", "NOTE", "GATE"};
  static const char *kSettingsNames[5] = {"VOL", "TUNE", "", "", ""};

  switch (state.ui.page) {
    case PageId::PLAY:
      return kPlayNames[index];
    case PageId::OSC_MIX:
      return kOscNames[index];
    case PageId::FILTER_AMP:
      return kFilterNames[index];
    case PageId::MOD:
      return kModNames[index];
    case PageId::FX:
      return fxPotName(index);
    case PageId::ARP:
      return kArpNames[index];
    case PageId::SEQ:
      return kSeqNames[index];
    case PageId::SETTINGS:
      return kSettingsNames[index];
    default:
      return "";
  }
}

void disableScroll(lv_obj_t *obj) {
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}
}

MainMenu::MainMenu() {
  g_main_menu = this;
}

void MainMenu::render() {
  if (root_ != nullptr) {
    return;
  }

  root_ = lv_obj_create(lv_scr_act());
  lv_obj_remove_style_all(root_);
  lv_obj_set_size(root_, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(root_, lv_color_hex(0x09111F), 0);
  disableScroll(root_);

  lv_obj_t *top_bar = lv_obj_create(root_);
  lv_obj_set_pos(top_bar, 0, 0);
  lv_obj_set_size(top_bar, 320, 46);
  lv_obj_set_style_radius(top_bar, 0, 0);
  lv_obj_set_style_border_width(top_bar, 0, 0);
  lv_obj_set_style_bg_color(top_bar, lv_color_hex(0x111C2E), 0);
  lv_obj_set_style_pad_all(top_bar, 0, 0);
  disableScroll(top_bar);

  status_title_label_ = lv_label_create(top_bar);
  lv_obj_set_style_text_font(status_title_label_, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(status_title_label_, lv_color_white(), 0);
  lv_obj_set_width(status_title_label_, 300);
  lv_label_set_long_mode(status_title_label_, LV_LABEL_LONG_CLIP);
  lv_obj_align(status_title_label_, LV_ALIGN_TOP_LEFT, 10, 4);

  status_label_ = lv_label_create(top_bar);
  lv_obj_set_style_text_font(status_label_, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(status_label_, lv_color_hex(0xD5E1F5), 0);
  lv_obj_set_width(status_label_, 300);
  lv_label_set_long_mode(status_label_, LV_LABEL_LONG_CLIP);
  lv_obj_align(status_label_, LV_ALIGN_BOTTOM_LEFT, 10, -4);

  content_panel_ = lv_obj_create(root_);
  lv_obj_set_pos(content_panel_, 0, 46);
  lv_obj_set_size(content_panel_, 320, 150);
  lv_obj_set_style_radius(content_panel_, 0, 0);
  lv_obj_set_style_border_width(content_panel_, 0, 0);
  lv_obj_set_style_bg_color(content_panel_, lv_color_hex(0x0D1628), 0);
  disableScroll(content_panel_);

  input_test_panel_ = lv_obj_create(root_);
  lv_obj_set_pos(input_test_panel_, 0, 46);
  lv_obj_set_size(input_test_panel_, 320, 150);
  lv_obj_set_style_radius(input_test_panel_, 0, 0);
  lv_obj_set_style_border_width(input_test_panel_, 0, 0);
  lv_obj_set_style_bg_color(input_test_panel_, lv_color_hex(0x0D1628), 0);
  disableScroll(input_test_panel_);
  input_test_page.createPage(input_test_panel_);

  pot_row_ = lv_obj_create(content_panel_);
  lv_obj_set_pos(pot_row_, -9, 8);
  lv_obj_set_size(pot_row_, 316, 56);
  lv_obj_set_style_bg_opa(pot_row_, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(pot_row_, 0, 0);
  lv_obj_set_style_pad_all(pot_row_, 0, 0);
  disableScroll(pot_row_);

  for (uint8_t index = 0; index < kPotCardCount; ++index) {
    pot_cards_[index] = lv_obj_create(pot_row_);
    lv_obj_set_size(pot_cards_[index], 60, 56);
    lv_obj_set_pos(pot_cards_[index], index * 64, 0);
    lv_obj_set_style_border_width(pot_cards_[index], 0, 0);
    lv_obj_set_style_radius(pot_cards_[index], 12, 0);
    lv_obj_set_style_pad_all(pot_cards_[index], 3, 0);

    pot_name_labels_[index] = lv_label_create(pot_cards_[index]);
    lv_obj_set_style_text_font(pot_name_labels_[index], LV_FONT_DEFAULT, 0);
    lv_obj_set_width(pot_name_labels_[index], 54);
    lv_label_set_long_mode(pot_name_labels_[index], LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(pot_name_labels_[index], LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(pot_name_labels_[index], LV_ALIGN_TOP_MID, 0, 5);

    pot_value_labels_[index] = lv_label_create(pot_cards_[index]);
    lv_obj_set_style_text_font(pot_value_labels_[index], LV_FONT_DEFAULT, 0);
    lv_obj_set_width(pot_value_labels_[index], 54);
    lv_label_set_long_mode(pot_value_labels_[index], LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(pot_value_labels_[index], LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(pot_value_labels_[index], LV_ALIGN_BOTTOM_MID, 0, -5);
  }

  seq_info_label_ = lv_label_create(content_panel_);
  lv_obj_set_pos(seq_info_label_, -6, 6);
  lv_obj_set_width(seq_info_label_, 300);
  lv_label_set_long_mode(seq_info_label_, LV_LABEL_LONG_CLIP);
  lv_obj_set_style_text_font(seq_info_label_, LV_FONT_DEFAULT, 0);
  lv_obj_set_style_text_color(seq_info_label_, lv_color_hex(0xD5E1F5), 0);

  for (uint8_t index = 0; index < kActionCount; ++index) {
    action_buttons_[index] = lv_button_create(content_panel_);
    lv_obj_set_size(action_buttons_[index], 145, 26);
    lv_obj_set_pos(action_buttons_[index], (index % 2U) ? 153 : 6, index < 2U ? 70 : 102);
    lv_obj_add_event_cb(action_buttons_[index], actionEventHandler, LV_EVENT_CLICKED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(index)));
    action_labels_[index] = lv_label_create(action_buttons_[index]);
    lv_obj_set_width(action_labels_[index], 130);
    lv_label_set_long_mode(action_labels_[index], LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(action_labels_[index], LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(action_labels_[index]);
  }

  for (uint8_t index = 0; index < kSeqButtonCount; ++index) {
    seq_step_buttons_[index] = lv_button_create(content_panel_);
    const int col = index % 4;
    const int row = index / 4;
    lv_obj_set_size(seq_step_buttons_[index], 72, 34);
    lv_obj_set_pos(seq_step_buttons_[index], -6 + col * 76, 34 + row * 36);
    lv_obj_add_event_cb(seq_step_buttons_[index], seqStepEventHandler, LV_EVENT_CLICKED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(index)));
    seq_step_labels_[index] = lv_label_create(seq_step_buttons_[index]);
    lv_obj_set_width(seq_step_labels_[index], 64);
    lv_label_set_long_mode(seq_step_labels_[index], LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(seq_step_labels_[index], LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(seq_step_labels_[index]);
  }

  lv_obj_t *tab_bar = lv_obj_create(root_);
  lv_obj_set_pos(tab_bar, 0, 196);
  lv_obj_set_size(tab_bar, 320, 44);
  lv_obj_set_style_radius(tab_bar, 0, 0);
  lv_obj_set_style_border_width(tab_bar, 0, 0);
  lv_obj_set_style_bg_color(tab_bar, lv_color_hex(0x111C2E), 0);
  lv_obj_set_style_pad_all(tab_bar, 0, 0);
  disableScroll(tab_bar);

  for (uint8_t index = 0; index < kTabCount; ++index) {
    tab_buttons_[index] = lv_button_create(tab_bar);
    lv_obj_set_size(tab_buttons_[index], 36, 32);
    lv_obj_set_pos(tab_buttons_[index], 8 + index * 38, 6);
    lv_obj_add_event_cb(tab_buttons_[index], tabEventHandler, LV_EVENT_CLICKED,
                        reinterpret_cast<void *>(static_cast<uintptr_t>(index)));
    tab_labels_[index] = lv_label_create(tab_buttons_[index]);
    lv_label_set_text(tab_labels_[index], kTabTitles[index]);
    lv_obj_center(tab_labels_[index]);
  }

  setPage(PageId::PLAY);
}

void MainMenu::loop() {
  if (state.ui.dirty) {
    refresh();
    state.ui.dirty = false;
  }
}

void MainMenu::handlePotChange(uint8_t index, float value) {
  value = clampUnit(value);

  switch (state.ui.page) {
    case PageId::PLAY:
      if (index == 0) state.patch.cutoff = value;
      if (index == 1) state.patch.resonance = value;
      if (index == 2) state.patch.glide = value;
      if (index == 3) state.arp.gate = static_cast<uint8_t>(40 + value * 50.0f);
      if (index == 4) state.transport.bpm = bpmFromNormalized(value);
      break;
    case PageId::OSC_MIX:
      if (index == 0) state.patch.osc1_mix = value;
      if (index == 1) state.patch.osc2_mix = value;
      if (index == 2) state.patch.noise_mix = value;
      if (index == 3) state.patch.octave_index = discreteIndex(value, 5);
      if (index == 4) state.patch.detune = value;
      break;
    case PageId::FILTER_AMP:
      if (index == 0) state.patch.cutoff = value;
      if (index == 1) state.patch.resonance = value;
      if (index == 2) state.patch.attack = value;
      if (index == 3) state.patch.decay = value;
      if (index == 4) state.patch.release = value;
      break;
    case PageId::MOD:
      if (index == 0) state.patch.lfo_rate = value;
      if (index == 1) state.patch.lfo_depth = value;
      if (index == 2) state.patch.glide = value;
      if (index == 3) state.patch.bend_range = value;
      if (index == 4) state.patch.lfo_target = static_cast<LfoTarget>(discreteIndex(value, 3));
      break;
    case PageId::FX:
      state.fx.enabled = true;
      switch (state.fx.mode) {
        case FxMode::ECHO:
          if (index == 0) state.fx.echo.mix = value;
          if (index == 1) state.fx.echo.time = value;
          if (index == 2) state.fx.echo.feedback = value;
          if (index == 3) state.fx.echo.ratio = value;
          if (index == 4) state.fx.echo.smear = value;
          break;
        case FxMode::REVERB:
          if (index == 0) state.fx.reverb.mix = value;
          if (index == 1) state.fx.reverb.size = value;
          if (index == 2) state.fx.reverb.damping = value;
          if (index == 3) state.fx.reverb.predelay = value;
          if (index == 4) state.fx.reverb.tone = value;
          break;
        case FxMode::DRIVE:
          if (index == 0) state.fx.drive.mix = value;
          if (index == 1) state.fx.drive.drive = value;
          if (index == 2) state.fx.drive.tone = value;
          if (index == 3) state.fx.drive.crush = value;
          if (index == 4) state.fx.drive.level = value;
          break;
        default:
          break;
      }
      break;
    case PageId::ARP:
      if (index == 0) state.transport.bpm = bpmFromNormalized(value);
      if (index == 1) state.arp.division = 1 + discreteIndex(value, 4);
      if (index == 2) state.arp.gate = static_cast<uint8_t>(30 + value * 65.0f);
      if (index == 3) state.arp.octave_range = 1 + discreteIndex(value, 3);
      if (index == 4) state.arp.mode = static_cast<ArpMode>(discreteIndex(value, 4));
      break;
    case PageId::SEQ:
      if (index == 0) state.transport.bpm = bpmFromNormalized(value);
      if (index == 1) state.sequencer.length = 1 + discreteIndex(value, 16);
      if (index == 2) state.transport.swing = value * 0.9f;
      if (index == 3) setSelectedStepValue(true, value);
      if (index == 4) setSelectedStepValue(false, value);
      break;
    case PageId::SETTINGS:
      if (index == 0) {
        state.setOutputVolume(value);
        setOutputVolume(state.audio.output_volume);
      }
      if (index == 1) {
        state.patch.tuning = static_cast<TuningId>(discreteIndex(value, tuningCount()));
      }
      break;
  }

  state.markDirty();
}

void MainMenu::handleOkPress() {
  if (!state.ui.confirm_clear_sequence) {
    return;
  }

  for (SequenceStep &step : state.sequencer.steps) {
    step.active = false;
    step.tie = false;
    step.note = 60;
    step.gate = 75;
  }

  state.ui.confirm_clear_sequence = false;
  state.markDirty();
}

void MainMenu::tabEventHandler(lv_event_t *event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED || g_main_menu == nullptr) {
    return;
  }

  const uint8_t tab_index =
      static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
  g_main_menu->setPage(kPages[tab_index]);
}

void MainMenu::actionEventHandler(lv_event_t *event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED || g_main_menu == nullptr) {
    return;
  }

  const uint8_t action_index =
      static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
  g_main_menu->handleAction(action_index);
}

void MainMenu::seqStepEventHandler(lv_event_t *event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED || g_main_menu == nullptr) {
    return;
  }

  const uint8_t local_index =
      static_cast<uint8_t>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(event)));
  const uint8_t step_index = state.sequencer.visible_bank * 8U + local_index;
  if (step_index >= 16U) {
    return;
  }

  if (state.sequencer.selected_step == step_index) {
    state.sequencer.steps[step_index].active = !state.sequencer.steps[step_index].active;
  } else {
    state.sequencer.selected_step = step_index;
  }
  state.markDirty();
}

void MainMenu::setPage(PageId page) {
  state.setPage(page);
  updateMode();
}

void MainMenu::updateMode() {
  Settings::getInstance()->setMode(state.currentMode());
}

void MainMenu::refresh() {
  refreshStatusBar();
  refreshTabs();
  refreshPotCards();
  refreshActionButtons();
  refreshSequencerButtons();
  refreshVisibility();
}

void MainMenu::refreshStatusBar() {
  if (state.ui.show_input_test) {
    lv_label_set_text(status_title_label_, "TEST PAGE");
    lv_label_set_text(status_label_, "Touch, audio, MIDI, and mux diagnostics");
    return;
  }

  lv_label_set_text(status_title_label_, pageTitle(state.ui.page));

  char status_text[96];
  if (state.ui.page == PageId::SETTINGS) {
    snprintf(status_text, sizeof(status_text), "VOL %u%%  TUNE %s",
             static_cast<unsigned>(roundf(state.audio.output_volume * 100.0f)),
             tuningLabel(state.patch.tuning));
  } else if (state.ui.page == PageId::OSC_MIX) {
    snprintf(status_text, sizeof(status_text), "W1 %s  W2 %s  N %u%%",
             oscWaveLabel(state.patch.osc1_wave), oscWaveLabel(state.patch.osc2_wave),
             static_cast<unsigned>(state.patch.noise_mix * 100.0f));
  } else if (state.ui.page == PageId::FX) {
    snprintf(status_text, sizeof(status_text), "FX %s %s", fxModeLabel(state.fx.mode),
             yesNo(state.fx.enabled));
  } else {
    snprintf(status_text, sizeof(status_text), "%uBPM %s A:%s S:%s",
             state.transport.bpm, state.transport.running ? "RUN" : "STOP",
             yesNo(state.arp.enabled), yesNo(state.sequencer.enabled));
  }
  lv_label_set_text(status_label_, status_text);
}

void MainMenu::refreshTabs() {
  const uint32_t accent = kPageColors[static_cast<uint8_t>(state.ui.page)];
  for (uint8_t index = 0; index < kTabCount; ++index) {
    const bool active = kPages[index] == state.ui.page;
    lv_obj_set_style_bg_color(tab_buttons_[index],
                              active ? lv_color_hex(accent) : lv_color_hex(0x1A2740), 0);
    lv_obj_set_style_text_color(tab_labels_[index],
                                active ? lv_color_white() : lv_color_hex(0xC2D2EA), 0);
  }
}

void MainMenu::refreshPotCards() {
  const uint32_t accent = kPageColors[static_cast<uint8_t>(state.ui.page)];
  for (uint8_t index = 0; index < kPotCardCount; ++index) {
    if (state.ui.page == PageId::SETTINGS) {
      if (index == 0) {
        lv_obj_clear_flag(pot_cards_[index], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(pot_cards_[index], 0, 0);
        lv_obj_set_size(pot_cards_[index], 68, 56);
        lv_obj_set_width(pot_name_labels_[index], 60);
        lv_obj_set_width(pot_value_labels_[index], 60);
      } else if (index == 1) {
        lv_obj_clear_flag(pot_cards_[index], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(pot_cards_[index], 72, 0);
        lv_obj_set_size(pot_cards_[index], 236, 56);
        lv_obj_set_width(pot_name_labels_[index], 228);
        lv_obj_set_width(pot_value_labels_[index], 228);
      } else {
        lv_obj_add_flag(pot_cards_[index], LV_OBJ_FLAG_HIDDEN);
        continue;
      }
    } else {
      lv_obj_clear_flag(pot_cards_[index], LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_pos(pot_cards_[index], index * 64, 0);
      lv_obj_set_size(pot_cards_[index], 60, 56);
      lv_obj_set_width(pot_name_labels_[index], 54);
      lv_obj_set_width(pot_value_labels_[index], 54);
    }

    const char *name = potName(index);
    lv_label_set_text(pot_name_labels_[index], name);
    char buffer[24];
    formatPotValue(buffer, sizeof(buffer), index);
    lv_label_set_text(pot_value_labels_[index], buffer);
    lv_obj_set_style_bg_color(pot_cards_[index], lv_color_hex(accent), 0);
    const bool active = name[0] != '\0';
    lv_obj_set_style_bg_opa(pot_cards_[index], active ? LV_OPA_30 : LV_OPA_10, 0);
    lv_obj_set_style_text_color(pot_name_labels_[index],
                                active ? lv_color_hex(0xE5EDF8) : lv_color_hex(0x7C8AA5), 0);
    lv_obj_set_style_text_color(pot_value_labels_[index],
                                active ? lv_color_white() : lv_color_hex(0x7C8AA5), 0);
  }
}

void MainMenu::refreshActionButtons() {
  const uint32_t accent = kPageColors[static_cast<uint8_t>(state.ui.page)];
  static const char *kLabels[8][4] = {
      {"RUN/STOP", "ARP TOG", "", "PANIC"},
      {"W1 <", "W1 >", "W2 <", "W2 >"},
      {"SUS -", "SUS +", "SNAP", "LONG"},
      {"LFO OFF", "FILT LFO", "PITCH LFO", "DEPTH 0"},
      {"BYPASS", "ECHO", "REVERB", "DIRT"},
      {"ENABLE", "LATCH", "RUN/STOP", "CLR HELD"},
      {"RUN/STOP", "REC ARM", "BANK", "CLEAR"},
      {"TEST PAGE", "", "", ""},
  };

  const uint8_t page_index = static_cast<uint8_t>(state.ui.page);
  for (uint8_t index = 0; index < kActionCount; ++index) {
    if (state.ui.page == PageId::SEQ) {
      lv_obj_set_size(action_buttons_[index], 72, 24);
      lv_obj_set_pos(action_buttons_[index], 10 + index * 76, 110);
      lv_obj_set_width(action_labels_[index], 64);
    } else {
      lv_obj_set_size(action_buttons_[index], 145, 26);
      lv_obj_set_pos(action_buttons_[index], (index % 2U) ? 153 : 6, index < 2U ? 70 : 102);
      lv_obj_set_width(action_labels_[index], 130);
    }
    lv_label_set_text(action_labels_[index], kLabels[page_index][index]);
    lv_obj_set_style_bg_color(action_buttons_[index], lv_color_hex(accent), 0);
    lv_obj_set_style_bg_opa(action_buttons_[index], LV_OPA_40, 0);
  }

  if (state.ui.page == PageId::SEQ && state.ui.confirm_clear_sequence) {
    lv_label_set_text(action_labels_[3], "WAIT OK");
  }
}

void MainMenu::refreshSequencerButtons() {
  char info_text[128];
  snprintf(info_text, sizeof(info_text), "STEP %02u %s G%u B%u",
           state.sequencer.selected_step + 1U,
           noteName(state.sequencer.steps[state.sequencer.selected_step].note),
           state.sequencer.steps[state.sequencer.selected_step].gate,
           state.sequencer.visible_bank + 1U);
  lv_label_set_text(seq_info_label_, info_text);

  for (uint8_t index = 0; index < kSeqButtonCount; ++index) {
    const uint8_t step_index = state.sequencer.visible_bank * 8U + index;
    const bool selected = step_index == state.sequencer.selected_step;
    const bool active = state.sequencer.steps[step_index].active;
    const bool playing = step_index == state.sequencer.playhead && state.transport.running;

    char label[24];
    snprintf(label, sizeof(label), "%02u %s", step_index + 1U,
             active ? noteName(state.sequencer.steps[step_index].note) : "--");
    lv_label_set_text(seq_step_labels_[index], label);

    uint32_t color = active ? 0xC2410C : 0x1A2740;
    if (selected) color = 0x2563EB;
    if (playing) color = 0x16A34A;
    lv_obj_set_style_bg_color(seq_step_buttons_[index], lv_color_hex(color), 0);
  }
}

void MainMenu::refreshVisibility() {
  const bool show_seq = state.ui.page == PageId::SEQ && !state.ui.show_input_test;
  const bool show_settings = state.ui.page == PageId::SETTINGS && !state.ui.show_input_test;
  const bool show_input = state.ui.show_input_test;

  if (show_input) {
    lv_obj_add_flag(content_panel_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(input_test_panel_, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(content_panel_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(input_test_panel_, LV_OBJ_FLAG_HIDDEN);
  }

  for (uint8_t index = 0; index < kActionCount; ++index) {
    if (show_input) {
      lv_obj_add_flag(action_buttons_[index], LV_OBJ_FLAG_HIDDEN);
    } else if (show_settings && index > 0) {
      lv_obj_add_flag(action_buttons_[index], LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_clear_flag(action_buttons_[index], LV_OBJ_FLAG_HIDDEN);
    }
  }

  if (show_seq) {
    lv_obj_add_flag(pot_row_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(seq_info_label_, LV_OBJ_FLAG_HIDDEN);
    for (uint8_t index = 0; index < kSeqButtonCount; ++index) {
      lv_obj_clear_flag(seq_step_buttons_[index], LV_OBJ_FLAG_HIDDEN);
    }
  } else {
    lv_obj_clear_flag(pot_row_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(seq_info_label_, LV_OBJ_FLAG_HIDDEN);
    for (uint8_t index = 0; index < kSeqButtonCount; ++index) {
      lv_obj_add_flag(seq_step_buttons_[index], LV_OBJ_FLAG_HIDDEN);
    }
  }
}

void MainMenu::handleAction(uint8_t action_index) {
  switch (state.ui.page) {
    case PageId::PLAY:
      if (action_index == 0) state.transport.running = !state.transport.running;
      if (action_index == 1) state.arp.enabled = !state.arp.enabled;
      if (action_index == 3) performance_engine.stopTransport();
      break;
    case PageId::OSC_MIX:
      if (action_index == 0) state.patch.osc1_wave = previousOscWave(state.patch.osc1_wave);
      if (action_index == 1) state.patch.osc1_wave = nextOscWave(state.patch.osc1_wave);
      if (action_index == 2) state.patch.osc2_wave = previousOscWave(state.patch.osc2_wave);
      if (action_index == 3) state.patch.osc2_wave = nextOscWave(state.patch.osc2_wave);
      break;
    case PageId::FILTER_AMP:
      if (action_index == 0) state.patch.sustain = max(0.0f, state.patch.sustain - 0.05f);
      if (action_index == 1) state.patch.sustain = min(1.0f, state.patch.sustain + 0.05f);
      if (action_index == 2) {
        state.patch.attack = 0.01f;
        state.patch.decay = 0.08f;
        state.patch.release = 0.10f;
      }
      if (action_index == 3) {
        state.patch.attack = 0.35f;
        state.patch.decay = 0.45f;
        state.patch.release = 0.55f;
      }
      break;
    case PageId::MOD:
      if (action_index == 0) state.patch.lfo_target = LfoTarget::OFF;
      if (action_index == 1) state.patch.lfo_target = LfoTarget::FILTER;
      if (action_index == 2) state.patch.lfo_target = LfoTarget::PITCH;
      if (action_index == 3) state.patch.lfo_depth = 0.0f;
      break;
    case PageId::FX:
      if (action_index == 0) state.fx.enabled = false;
      if (action_index == 1) {
        state.fx.enabled = true;
        state.fx.mode = FxMode::ECHO;
      }
      if (action_index == 2) {
        state.fx.enabled = true;
        state.fx.mode = FxMode::REVERB;
      }
      if (action_index == 3) {
        state.fx.enabled = true;
        state.fx.mode = FxMode::DRIVE;
      }
      break;
    case PageId::ARP:
      if (action_index == 0) state.arp.enabled = !state.arp.enabled;
      if (action_index == 1) state.arp.latch = !state.arp.latch;
      if (action_index == 2) state.transport.running = !state.transport.running;
      if (action_index == 3) performance_engine.clearHeldNotes();
      break;
    case PageId::SEQ:
      if (action_index == 0) {
        state.sequencer.enabled = true;
        state.transport.running = !state.transport.running;
      }
      if (action_index == 1) {
        state.sequencer.enabled = true;
        state.sequencer.record_armed = !state.sequencer.record_armed;
      }
      if (action_index == 2) state.sequencer.visible_bank ^= 1U;
      if (action_index == 3) state.ui.confirm_clear_sequence = !state.ui.confirm_clear_sequence;
      break;
    case PageId::SETTINGS:
      if (action_index == 0) {
        state.setInputTestVisible(true);
        updateMode();
      }
      break;
  }

  state.markDirty();
}

void MainMenu::setSelectedStepValue(bool set_note, float normalized) {
  SequenceStep &step = state.sequencer.steps[state.sequencer.selected_step];
  if (set_note) {
    step.note = static_cast<uint8_t>(36 + roundf(normalized * 48.0f));
    step.active = true;
  } else {
    step.gate = static_cast<uint8_t>(20 + normalized * 75.0f);
  }
}

MainMenu main_menu;
