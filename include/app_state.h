#ifndef APP_STATE_H
#define APP_STATE_H

#include <Arduino.h>

#include "settings.h"

enum class PageId : uint8_t {
  PLAY = 0,
  OSC_MIX,
  FILTER_AMP,
  MOD,
  ARP,
  SEQ,
  SETTINGS
};

enum class LfoTarget : uint8_t {
  OFF = 0,
  FILTER,
  PITCH
};

enum class ArpMode : uint8_t {
  UP = 0,
  DOWN,
  UP_DOWN,
  RANDOM
};

enum class TuningId : uint8_t {
  STANDARD = 0,
  SHUR,
  ABUATA,
  DASHTI,
  BAYAT_E_TORK,
  AFSHARI,
  SEGAH,
  CHAHARGAH,
  HOMAYUN,
  BAYAT_E_ESFAHAN,
  NAVA,
  MAHUR,
  RAST_PANJGAH
};

struct PatchState {
  float osc1_mix = 0.85f;
  float osc2_mix = 0.60f;
  float noise_mix = 0.05f;
  int8_t octave_index = 2;
  float detune = 0.08f;
  float cutoff = 0.72f;
  float resonance = 0.20f;
  float attack = 0.05f;
  float decay = 0.22f;
  float sustain = 0.72f;
  float release = 0.24f;
  float lfo_rate = 0.22f;
  float lfo_depth = 0.12f;
  LfoTarget lfo_target = LfoTarget::FILTER;
  float glide = 0.08f;
  float bend_range = 0.25f;
  TuningId tuning = TuningId::STANDARD;
};

struct TransportState {
  uint16_t bpm = 120;
  bool running = false;
  float swing = 0.0f;
  uint8_t step_index = 0;
};

struct ArpState {
  bool enabled = false;
  bool latch = false;
  ArpMode mode = ArpMode::UP;
  uint8_t octave_range = 1;
  uint8_t division = 1;
  uint8_t gate = 70;
};

struct SequenceStep {
  bool active = false;
  bool tie = false;
  uint8_t note = 60;
  uint8_t gate = 75;
};

struct SequencerState {
  bool enabled = false;
  bool record_armed = false;
  uint8_t length = 16;
  uint8_t playhead = 0;
  uint8_t selected_step = 0;
  uint8_t visible_bank = 0;
  SequenceStep steps[16];
};

struct UiState {
  PageId page = PageId::PLAY;
  bool dirty = true;
  bool show_input_test = false;
  bool confirm_clear_sequence = false;
};

struct MidiStatus {
  bool connected = false;
  uint16_t vendor_id = 0;
  uint16_t product_id = 0;
  bool note_recent = false;
  uint8_t last_note = 0;
  uint8_t last_velocity = 0;
  unsigned long last_note_ms = 0;
};

struct AudioStatus {
  bool codec_ready = false;
  bool self_test_active = false;
  float output_volume = 0.50f;
};

class AppState {
 public:
  static AppState &instance();

  PatchState patch;
  TransportState transport;
  ArpState arp;
  SequencerState sequencer;
  UiState ui;
  MidiStatus midi;
  AudioStatus audio;

  void setPage(PageId page);
  void setInputTestVisible(bool visible);
  void markDirty();
  Mode currentMode() const;
  void updateMidiDevice(bool connected, uint16_t vendor_id, uint16_t product_id);
  void registerMidiNote(uint8_t note, uint8_t velocity);
  void refreshTransientStatus(unsigned long now_ms);
  void updateAudioStatus(bool codec_ready, bool self_test_active);
  void setOutputVolume(float volume);

 private:
  AppState();
};

const char *pageTitle(PageId page);
const char *arpModeLabel(ArpMode mode);
const char *lfoTargetLabel(LfoTarget target);
const char *tuningLabel(TuningId tuning);
uint8_t tuningCount();

#endif
