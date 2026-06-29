#ifndef PERFORMANCE_ENGINE_H
#define PERFORMANCE_ENGINE_H

#include <Arduino.h>

#include "app_state.h"

class Synth;

class PerformanceEngine {
 public:
  void begin(Synth *synth);
  void update();
  void onMidiNoteOn(byte channel, byte note, byte velocity);
  void onMidiNoteOff(byte channel, byte note, byte velocity);
  void onMidiPitchBend(byte channel, int pitch);
  void onMidiClockTick();
  void onMidiStart();
  void onMidiContinue();
  void onMidiStop();
  void stopTransport();
  void clearHeldNotes();

 private:
  static constexpr uint8_t kHeldCapacity = 12;
  static constexpr uint8_t kClocksPerStep = 6;  // 24 PPQN / 4 (16th-note step)

  bool externalClock() const;
  void syncTransportState();
  void fireStep();
  void advanceTransportStep();
  void handleSequencerStep();
  void handleArpStep();
  void triggerGeneratedNote(uint8_t note, uint8_t velocity, unsigned long duration_us);
  void releaseGeneratedNoteIfDue();
  void clearGeneratedNote();
  void addHeldNote(uint8_t note);
  void removeHeldNote(uint8_t note);
  uint8_t nextArpNote();
  void recordStepFromMidi(uint8_t note);
  bool directPlayEnabled() const;

  Synth *synth_ = nullptr;
  AppState *state_ = nullptr;
  unsigned long next_step_due_us_ = 0;
  unsigned long gate_off_due_us_ = 0;
  bool generated_note_active_ = false;
  uint8_t generated_note_ = 0;
  uint8_t held_notes_[kHeldCapacity] = {0};
  uint8_t held_count_ = 0;
  int8_t arp_index_ = -1;
  int8_t arp_direction_ = 1;
  uint8_t arp_division_counter_ = 0;
  bool transport_running_cache_ = false;
  uint8_t clock_tick_count_ = 0;
  unsigned long last_clock_us_ = 0;
  bool clock_timing_valid_ = false;
};

extern PerformanceEngine performance_engine;

#endif
