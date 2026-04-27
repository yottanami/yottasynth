#include "synth.h"

#include <math.h>

#include "audio_setup.h"

namespace {
constexpr float kMidiNoteFrequencies[128] = {
    8.176f,   8.662f,   9.177f,   9.723f,   10.301f,  10.913f,  11.562f,
    12.250f,  12.978f,  13.750f,  14.568f,  15.434f,  16.352f,  17.324f,
    18.354f,  19.445f,  20.602f,  21.827f,  23.125f,  24.500f,  25.957f,
    27.500f,  29.135f,  30.868f,  32.703f,  34.648f,  36.708f,  38.891f,
    41.203f,  43.654f,  46.249f,  48.999f,  51.913f,  55.000f,  58.270f,
    61.735f,  65.406f,  69.296f,  73.416f,  77.782f,  82.407f,  87.307f,
    92.499f,  97.999f,  103.826f, 110.000f, 116.541f, 123.471f, 130.813f,
    138.591f, 146.832f, 155.563f, 164.814f, 174.614f, 184.997f, 195.998f,
    207.652f, 220.000f, 233.082f, 246.942f, 261.626f, 277.183f, 293.665f,
    311.127f, 329.628f, 349.228f, 369.994f, 391.995f, 415.305f, 440.000f,
    466.164f, 493.883f, 523.251f, 554.365f, 587.330f, 622.254f, 659.255f,
    698.456f, 739.989f, 783.991f, 830.609f, 880.000f, 932.328f, 987.767f,
    1046.502f, 1108.731f, 1174.659f, 1244.508f, 1318.510f, 1396.913f,
    1479.978f, 1567.982f, 1661.219f, 1760.000f, 1864.655f, 1975.533f,
    2093.005f, 2217.461f, 2349.318f, 2489.016f, 2637.020f, 2793.826f,
    2959.955f, 3135.963f, 3322.438f, 3520.000f, 3729.310f, 3951.066f,
    4186.009f, 4434.922f, 4698.636f, 4978.032f, 5274.041f, 5587.652f,
    5919.911f, 6271.927f, 6644.875f, 7040.000f, 7458.620f, 7902.133f,
    8372.018f, 8869.844f, 9397.273f, 9956.063f, 10548.080f, 11175.300f,
    11839.820f, 12543.850f};

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

Synth::Synth(AudioSynthWaveform *waveform1,
             AudioSynthWaveform *waveform2,
             AudioSynthNoisePink *pink,
             AudioFilterStateVariable *filter,
             AudioEffectEnvelope *envelope)
    : waveform1_(waveform1),
      waveform2_(waveform2),
      pink_(pink),
      envelope_(envelope),
      filter_(filter) {
}

void Synth::setup() {
  waveform1_->begin(WAVEFORM_SAWTOOTH);
  waveform1_->pulseWidth(0.50f);
  waveform2_->begin(WAVEFORM_SAWTOOTH);
  waveform2_->pulseWidth(0.35f);

  lead_mixer.gain(0, patch_.osc1_mix);
  lead_mixer.gain(1, patch_.osc2_mix);
  lead_mixer.gain(2, patch_.noise_mix);

  filter_->octaveControl(4.0f);
  filter_->resonance(0.9f);
  envelope_->attack(10.0f);
  envelope_->decay(100.0f);
  envelope_->sustain(0.7f);
  envelope_->release(250.0f);

  current_note_ = 60;
  current_velocity_ = 100;
  current_freq1_ = noteToFrequency(current_note_);
  current_freq2_ = current_freq1_;
  target_freq1_ = current_freq1_;
  target_freq2_ = current_freq2_;
  waveform1_->frequency(current_freq1_);
  waveform2_->frequency(current_freq2_);
  applyPatch(patch_);
  last_update_us_ = micros();
}

void Synth::loop() {
  if (self_test_active_) {
    if (millis() - self_test_started_ms_ >= 1200UL) {
      stopSelfTest();
    }
    return;
  }

  const unsigned long now = micros();
  float delta_seconds = 0.0f;
  if (last_update_us_ != 0) {
    const unsigned long delta_us = now - last_update_us_;
    delta_seconds = static_cast<float>(delta_us) / 1000000.0f;
  }
  last_update_us_ = now;

  updateModulation();

  if (note_count_ == 0) {
    return;
  }

  const float glide_amount = clampUnit(patch_.glide);
  if (glide_amount <= 0.01f) {
    current_freq1_ = target_freq1_;
    current_freq2_ = target_freq2_;
  } else {
    const float response = 12.0f - (glide_amount * 11.5f);
    const float alpha = clampUnit(delta_seconds * response);
    current_freq1_ += (target_freq1_ - current_freq1_) * alpha;
    current_freq2_ += (target_freq2_ - current_freq2_) * alpha;
  }

  waveform1_->frequency(current_freq1_);
  waveform2_->frequency(current_freq2_);
}

void Synth::applyPatch(const PatchState &patch) {
  patch_ = patch;

  if (self_test_active_) {
    return;
  }

  lead_mixer.gain(0, clampUnit(patch_.osc1_mix));
  lead_mixer.gain(1, clampUnit(patch_.osc2_mix));
  lead_mixer.gain(2, clampUnit(patch_.noise_mix));

  pink_->amplitude(clampUnit(patch_.noise_mix));
  filter_->frequency(normalizedToFilterHz(patch_.cutoff + filter_lfo_value_));
  filter_->resonance(0.7f + (clampUnit(patch_.resonance) * 4.3f));

  envelope_->attack(5.0f + (clampUnit(patch_.attack) * 2500.0f));
  envelope_->decay(20.0f + (clampUnit(patch_.decay) * 3000.0f));
  envelope_->sustain(clampUnit(patch_.sustain));
  envelope_->release(30.0f + (clampUnit(patch_.release) * 3200.0f));

  if (note_count_ > 0) {
    refreshVoices(false);
  }
}

void Synth::setPitchBend(int pitch) {
  const float bend_norm = static_cast<float>(pitch) / 8192.0f;
  const float bend_range_semitones = 1.0f + (clampUnit(patch_.bend_range) * 11.0f);
  const float semitone_offset = bend_norm * bend_range_semitones;
  pitch_bend_factor_ = powf(2.0f, semitone_offset / 12.0f);

  if (note_count_ > 0) {
    refreshVoices(false);
  }
}

void Synth::noteOn(byte note, byte velocity) {
  if (note < 24 || note > 107) {
    return;
  }

  current_velocity_ = velocity;

  for (byte index = 0; index < note_count_; ++index) {
    if (note_buffer_[index] == note) {
      for (byte move = index; move + 1 < note_count_; ++move) {
        note_buffer_[move] = note_buffer_[move + 1];
      }
      --note_count_;
      break;
    }
  }

  if (note_count_ < kBufferSize) {
    note_buffer_[note_count_] = note;
    ++note_count_;
  }

  current_note_ = note_buffer_[note_count_ - 1];
  retriggerCurrentNote();
}

void Synth::noteOff(byte note) {
  if (note_count_ == 0) {
    return;
  }

  for (byte index = 0; index < note_count_; ++index) {
    if (note_buffer_[index] == note) {
      for (byte move = index; move + 1 < note_count_; ++move) {
        note_buffer_[move] = note_buffer_[move + 1];
      }
      --note_count_;
      break;
    }
  }

  if (note_count_ == 0) {
    envelope_->noteOff();
    return;
  }

  current_note_ = note_buffer_[note_count_ - 1];
  refreshVoices(true);
}

void Synth::allNotesOff() {
  note_count_ = 0;
  envelope_->noteOff();
}

void Synth::startSelfTest() {
  self_test_active_ = true;
  self_test_started_ms_ = millis();
  note_count_ = 0;

  waveform1_->begin(WAVEFORM_SINE);
  waveform2_->begin(WAVEFORM_SINE);
  waveform1_->frequency(440.0f);
  waveform2_->frequency(660.0f);
  waveform1_->amplitude(0.75f);
  waveform2_->amplitude(0.18f);
  pink_->amplitude(0.0f);

  lead_mixer.gain(0, 0.90f);
  lead_mixer.gain(1, 0.25f);
  lead_mixer.gain(2, 0.0f);
  filter_->frequency(5000.0f);
  filter_->resonance(0.8f);
  envelope_->attack(5.0f);
  envelope_->decay(10.0f);
  envelope_->sustain(1.0f);
  envelope_->release(40.0f);
  envelope_->noteOn();
}

void Synth::stopSelfTest() {
  if (!self_test_active_) {
    return;
  }

  self_test_active_ = false;
  envelope_->noteOff();
  waveform1_->amplitude(0.0f);
  waveform2_->amplitude(0.0f);
  pink_->amplitude(0.0f);

  waveform1_->begin(WAVEFORM_SAWTOOTH);
  waveform2_->begin(WAVEFORM_SAWTOOTH);
  waveform1_->pulseWidth(0.50f);
  waveform2_->pulseWidth(0.35f);
  applyPatch(patch_);
}

bool Synth::isSelfTestActive() const {
  return self_test_active_;
}

void Synth::retriggerCurrentNote() {
  refreshVoices(true);
  const float amplitude = 0.20f + (static_cast<float>(current_velocity_) / 127.0f) * 0.80f;
  waveform1_->amplitude(amplitude);
  waveform2_->amplitude(amplitude * 0.95f);
  pink_->amplitude(clampUnit(patch_.noise_mix) * amplitude);
  envelope_->noteOn();
}

void Synth::refreshVoices(bool immediate) {
  target_freq1_ = noteWithBendToFrequency(current_note_);
  target_freq2_ = noteWithBendToFrequency(current_note_ + (patch_.octave_index - 2) * 12);
  const float detune_multiplier = 1.0f + ((clampUnit(patch_.detune) - 0.5f) * 0.10f);
  target_freq2_ *= detune_multiplier * pitchLfoMultiplier();

  if (patch_.lfo_target == LfoTarget::PITCH) {
    target_freq1_ *= pitchLfoMultiplier();
    target_freq2_ *= pitchLfoMultiplier();
  }

  if (immediate) {
    current_freq1_ = target_freq1_;
    current_freq2_ = target_freq2_;
    waveform1_->frequency(current_freq1_);
    waveform2_->frequency(current_freq2_);
  }
}

void Synth::updateModulation() {
  if (patch_.lfo_target == LfoTarget::OFF || clampUnit(patch_.lfo_depth) <= 0.001f) {
    filter_lfo_value_ = 0.0f;
    filter_->frequency(normalizedToFilterHz(patch_.cutoff));
    return;
  }

  const float rate_hz = 0.15f + (clampUnit(patch_.lfo_rate) * 10.0f);
  const float delta_seconds = 0.001f;
  lfo_phase_ += rate_hz * delta_seconds;
  while (lfo_phase_ >= 1.0f) {
    lfo_phase_ -= 1.0f;
  }

  const float triangle = lfo_phase_ < 0.5f ? (-1.0f + (lfo_phase_ * 4.0f))
                                            : (3.0f - (lfo_phase_ * 4.0f));
  if (patch_.lfo_target == LfoTarget::FILTER) {
    filter_lfo_value_ = triangle * clampUnit(patch_.lfo_depth) * 0.18f;
    filter_->frequency(normalizedToFilterHz(patch_.cutoff + filter_lfo_value_));
  } else {
    filter_lfo_value_ = 0.0f;
  }
}

float Synth::noteToFrequency(uint8_t note) const {
  if (note > 127) {
    note = 127;
  }
  return kMidiNoteFrequencies[note];
}

float Synth::noteWithBendToFrequency(int note_offset) const {
  if (note_offset < 0) {
    note_offset = 0;
  }
  if (note_offset > 127) {
    note_offset = 127;
  }
  return noteToFrequency(static_cast<uint8_t>(note_offset)) * pitch_bend_factor_;
}

float Synth::pitchLfoMultiplier() const {
  if (patch_.lfo_target != LfoTarget::PITCH) {
    return 1.0f;
  }

  const float triangle = lfo_phase_ < 0.5f ? (-1.0f + (lfo_phase_ * 4.0f))
                                            : (3.0f - (lfo_phase_ * 4.0f));
  const float semitone_span = clampUnit(patch_.lfo_depth) * 2.0f;
  return powf(2.0f, (triangle * semitone_span) / 12.0f);
}

float Synth::normalizedToFilterHz(float normalized) const {
  normalized = clampUnit(normalized);
  const float min_hz = 80.0f;
  const float max_hz = 9000.0f;
  return min_hz * powf(max_hz / min_hz, normalized);
}
