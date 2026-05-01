#ifndef SYNTH_H
#define SYNTH_H

#include <Audio.h>

#include "app_state.h"

class Synth {
 public:
  Synth(AudioSynthWaveform *waveform1,
        AudioSynthWaveform *waveform2,
        AudioSynthNoisePink *pink,
        AudioFilterStateVariable *filter,
        AudioEffectEnvelope *envelope);

  void setup();
  void loop();
  void applyPatch(const PatchState &patch);
  void setPitchBend(int pitch);
  void noteOn(byte note, byte velocity);
  void noteOff(byte note);
  void allNotesOff();
  void startSelfTest();
  void stopSelfTest();
  bool isSelfTestActive() const;

 private:
  static constexpr byte kBufferSize = 8;

  void retriggerCurrentNote();
  void refreshVoices(bool immediate);
  void updateModulation();
  void applyOscillatorWaveforms(bool force);
  int waveformConstant(OscWave wave) const;
  float noteToFrequency(uint8_t note) const;
  float noteWithBendToFrequency(int note_offset) const;
  float pitchLfoMultiplier() const;
  float normalizedToFilterHz(float normalized) const;

  AudioSynthWaveform *waveform1_;
  AudioSynthWaveform *waveform2_;
  AudioSynthNoisePink *pink_;
  AudioEffectEnvelope *envelope_;
  AudioFilterStateVariable *filter_;

  PatchState patch_;
  byte note_buffer_[kBufferSize] = {0};
  byte note_count_ = 0;
  byte current_note_ = 0;
  byte current_velocity_ = 100;
  float pitch_bend_factor_ = 1.0f;
  float filter_lfo_value_ = 0.0f;
  float current_freq1_ = 0.0f;
  float current_freq2_ = 0.0f;
  float target_freq1_ = 0.0f;
  float target_freq2_ = 0.0f;
  unsigned long last_update_us_ = 0;
  float lfo_phase_ = 0.0f;
  OscWave applied_waveform1_ = OscWave::SAW;
  OscWave applied_waveform2_ = OscWave::SAW;
  bool self_test_active_ = false;
  unsigned long self_test_started_ms_ = 0;
};

#endif
