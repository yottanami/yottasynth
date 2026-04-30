#include "app_state.h"

namespace {
constexpr SequenceStep kDefaultSequence[16] = {
    {true, false, 60, 78},  {true, false, 62, 72},  {true, false, 67, 82},
    {false, false, 67, 70}, {true, false, 60, 78},  {true, false, 64, 72},
    {true, false, 69, 82},  {false, false, 69, 70}, {true, false, 60, 78},
    {true, false, 62, 72},  {true, false, 67, 82},  {false, false, 67, 70},
    {true, false, 72, 78},  {true, false, 69, 72},  {true, false, 64, 82},
    {false, false, 60, 70},
};
}

AppState::AppState() {
  for (uint8_t index = 0; index < 16; ++index) {
    sequencer.steps[index] = kDefaultSequence[index];
  }
}

AppState &AppState::instance() {
  static AppState state;
  return state;
}

namespace {
float clampUnit(float value) {
  if (value < 0.0f) {
    return 0.0f;
  }
  if (value > 1.0f) {
    return 1.0f;
  }
  return value;
}
}

void AppState::setPage(PageId page) {
  ui.page = page;
  ui.show_input_test = false;
  ui.confirm_clear_sequence = false;
  ui.dirty = true;
}

void AppState::setInputTestVisible(bool visible) {
  ui.show_input_test = visible;
  ui.confirm_clear_sequence = false;
  ui.dirty = true;
}

void AppState::markDirty() {
  ui.dirty = true;
}

Mode AppState::currentMode() const {
  if (ui.show_input_test) {
    return Mode::INPUT_TEST;
  }

  switch (ui.page) {
    case PageId::ARP:
      return Mode::ARPEGGIATOR;
    case PageId::SEQ:
      return Mode::SEQUENCER;
    case PageId::SETTINGS:
    case PageId::PLAY:
    case PageId::OSC_MIX:
    case PageId::FILTER_AMP:
    case PageId::MOD:
    default:
      return Mode::SYNTHESIZER;
  }
}

void AppState::updateMidiDevice(bool connected, uint16_t vendor_id, uint16_t product_id) {
  if (midi.connected == connected && midi.vendor_id == vendor_id &&
      midi.product_id == product_id) {
    return;
  }

  midi.connected = connected;
  midi.vendor_id = vendor_id;
  midi.product_id = product_id;
  ui.dirty = true;
}

void AppState::registerMidiNote(uint8_t note, uint8_t velocity) {
  midi.last_note = note;
  midi.last_velocity = velocity;
  midi.last_note_ms = millis();
  midi.note_recent = true;
  ui.dirty = true;
}

void AppState::refreshTransientStatus(unsigned long now_ms) {
  const bool recent = midi.connected && (now_ms - midi.last_note_ms) < 1500UL;
  if (midi.note_recent != recent) {
    midi.note_recent = recent;
    ui.dirty = true;
  }
}

void AppState::updateAudioStatus(bool codec_ready, bool self_test_active) {
  if (audio.codec_ready == codec_ready && audio.self_test_active == self_test_active) {
    return;
  }

  audio.codec_ready = codec_ready;
  audio.self_test_active = self_test_active;
  ui.dirty = true;
}

void AppState::setOutputVolume(float volume) {
  const float clamped = clampUnit(volume);
  if (audio.output_volume == clamped) {
    return;
  }

  audio.output_volume = clamped;
  ui.dirty = true;
}

const char *pageTitle(PageId page) {
  switch (page) {
    case PageId::PLAY:
      return "PLAY";
    case PageId::OSC_MIX:
      return "OSC / MIX";
    case PageId::FILTER_AMP:
      return "FILTER / AMP";
    case PageId::MOD:
      return "MOD";
    case PageId::ARP:
      return "ARPEGGIATOR";
    case PageId::SEQ:
      return "SEQUENCER";
    case PageId::SETTINGS:
      return "SETTINGS";
    default:
      return "PLAY";
  }
}

const char *arpModeLabel(ArpMode mode) {
  switch (mode) {
    case ArpMode::UP:
      return "UP";
    case ArpMode::DOWN:
      return "DOWN";
    case ArpMode::UP_DOWN:
      return "UP/DOWN";
    case ArpMode::RANDOM:
      return "RANDOM";
    default:
      return "UP";
  }
}

const char *lfoTargetLabel(LfoTarget target) {
  switch (target) {
    case LfoTarget::OFF:
      return "OFF";
    case LfoTarget::FILTER:
      return "FILTER";
    case LfoTarget::PITCH:
      return "PITCH";
    default:
      return "OFF";
  }
}
