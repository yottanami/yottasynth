#include "synth.h"

#include <math.h>

#include "audio_setup.h"

namespace {
constexpr float kReferenceC4Hz = 261.63f;
constexpr float kStandardCents[12] = {0.0f,   100.0f, 200.0f, 300.0f, 400.0f, 500.0f,
                                      600.0f, 700.0f, 800.0f, 900.0f, 1000.0f, 1100.0f};
constexpr float kShurCents[12] = {0.0f,   100.0f, 205.0f, 300.0f, 340.0f, 500.0f,
                                  600.0f, 700.0f, 800.0f, 905.0f, 995.0f, 1100.0f};
constexpr float kAbuataCents[12] = {0.0f,   100.0f, 205.0f, 300.0f, 340.0f, 500.0f,
                                    600.0f, 700.0f, 800.0f, 905.0f, 1000.0f, 1100.0f};
constexpr float kAfshariCents[12] = {0.0f,   100.0f, 205.0f, 300.0f, 340.0f, 500.0f,
                                     600.0f, 700.0f, 800.0f, 835.0f, 995.0f, 1100.0f};
constexpr float kSegahCents[12] = {0.0f,   100.0f, 200.0f, 300.0f, 340.0f, 500.0f,
                                   600.0f, 700.0f, 800.0f, 835.0f, 995.0f, 1100.0f};
constexpr float kChahargahCents[12] = {0.0f,   100.0f, 135.0f, 300.0f, 410.0f, 500.0f,
                                       600.0f, 700.0f, 800.0f, 835.0f, 1000.0f, 1110.0f};
constexpr float kHomayunCents[12] = {0.0f,   100.0f, 205.0f, 300.0f, 340.0f, 500.0f,
                                     600.0f, 700.0f, 800.0f, 835.0f, 1000.0f, 1110.0f};
constexpr float kBayatEsfahanCents[12] = {
    0.0f, 100.0f, 205.0f, 300.0f, 340.0f, 565.0f, 600.0f, 700.0f, 800.0f, 905.0f, 995.0f, 1100.0f};
constexpr float kMahurCents[12] = {0.0f,   100.0f, 205.0f, 300.0f, 410.0f, 500.0f,
                                   600.0f, 700.0f, 800.0f, 905.0f, 1000.0f, 1110.0f};
constexpr float kRastPanjgahCents[12] = {
    0.0f, 100.0f, 205.0f, 300.0f, 410.0f, 500.0f, 600.0f, 700.0f, 800.0f, 900.0f, 995.0f, 1100.0f};

const float *tuningCents(TuningId tuning) {
  switch (tuning) {
    case TuningId::STANDARD:
      return kStandardCents;
    case TuningId::SHUR:
      return kShurCents;
    case TuningId::ABUATA:
      return kAbuataCents;
    case TuningId::DASHTI:
      return kShurCents;
    case TuningId::BAYAT_E_TORK:
      return kShurCents;
    case TuningId::AFSHARI:
      return kAfshariCents;
    case TuningId::SEGAH:
      return kSegahCents;
    case TuningId::CHAHARGAH:
      return kChahargahCents;
    case TuningId::HOMAYUN:
      return kHomayunCents;
    case TuningId::BAYAT_E_ESFAHAN:
      return kBayatEsfahanCents;
    case TuningId::NAVA:
      return kShurCents;
    case TuningId::MAHUR:
      return kMahurCents;
    case TuningId::RAST_PANJGAH:
      return kRastPanjgahCents;
    default:
      return kStandardCents;
  }
}

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
  const float *cents = tuningCents(patch_.tuning);
  const uint8_t note_class = note % 12U;
  const int octave_from_c4 = static_cast<int>(note / 12U) - 5;
  const float octave_ratio = static_cast<float>(octave_from_c4) + (cents[note_class] / 1200.0f);
  return kReferenceC4Hz * powf(2.0f, octave_ratio);
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
