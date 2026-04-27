#include "performance_engine.h"

#include "synth.h"

namespace {
unsigned long stepIntervalUs(const TransportState &transport, uint8_t step_index) {
  const uint16_t bpm = transport.bpm < 40 ? 40 : transport.bpm;
  const unsigned long quarter_us = 60000000UL / bpm;
  const float base = static_cast<float>(quarter_us) / 4.0f;
  const float swing = transport.swing;

  if (swing <= 0.001f) {
    return static_cast<unsigned long>(base);
  }

  const bool odd_step = (step_index % 2U) == 1U;
  const float swing_offset = 0.5f * swing;
  const float multiplier = odd_step ? (1.0f + swing_offset) : (1.0f - swing_offset);
  return static_cast<unsigned long>(base * multiplier);
}

uint8_t clampStepLength(uint8_t length) {
  if (length < 1) {
    return 1;
  }
  if (length > 16) {
    return 16;
  }
  return length;
}
}

void PerformanceEngine::begin(Synth *synth) {
  synth_ = synth;
  state_ = &AppState::instance();
  next_step_due_us_ = micros();
  gate_off_due_us_ = 0;
}

void PerformanceEngine::update() {
  if (synth_ == nullptr || state_ == nullptr) {
    return;
  }

  syncTransportState();
  releaseGeneratedNoteIfDue();

  if (!state_->transport.running) {
    return;
  }

  const unsigned long now = micros();
  uint8_t safety = 0;
  while (static_cast<long>(now - next_step_due_us_) >= 0 && safety < 4) {
    advanceTransportStep();
    ++safety;
  }
}

void PerformanceEngine::onMidiNoteOn(byte, byte note, byte velocity) {
  if (state_ == nullptr || synth_ == nullptr) {
    return;
  }

  if (synth_->isSelfTestActive()) {
    return;
  }

  if (state_->sequencer.enabled && state_->transport.running && state_->sequencer.record_armed) {
    recordStepFromMidi(note);
    synth_->noteOn(note, velocity);
    return;
  }

  addHeldNote(note);

  if (state_->arp.enabled) {
    if (!state_->transport.running) {
      synth_->noteOn(note, velocity);
    }
    return;
  }

  if (state_->sequencer.enabled && state_->transport.running) {
    return;
  }

  if (directPlayEnabled()) {
    synth_->noteOn(note, velocity);
  }
}

void PerformanceEngine::onMidiNoteOff(byte, byte note, byte) {
  if (state_ == nullptr || synth_ == nullptr) {
    return;
  }

  if (synth_->isSelfTestActive()) {
    return;
  }

  removeHeldNote(note);

  if (state_->sequencer.enabled && state_->transport.running && state_->sequencer.record_armed) {
    synth_->noteOff(note);
    return;
  }

  if (state_->arp.enabled) {
    if (!state_->transport.running && !state_->arp.latch) {
      synth_->noteOff(note);
    }
    return;
  }

  if (state_->sequencer.enabled && state_->transport.running) {
    return;
  }

  if (directPlayEnabled()) {
    synth_->noteOff(note);
  }
}

void PerformanceEngine::onMidiPitchBend(byte, int pitch) {
  if (synth_ != nullptr && !synth_->isSelfTestActive()) {
    synth_->setPitchBend(pitch);
  }
}

void PerformanceEngine::stopTransport() {
  if (state_ == nullptr) {
    return;
  }

  state_->transport.running = false;
  state_->transport.step_index = 0;
  state_->sequencer.playhead = 0;
  state_->markDirty();
  clearGeneratedNote();
}

void PerformanceEngine::clearHeldNotes() {
  held_count_ = 0;
  arp_index_ = -1;
  clearGeneratedNote();
}

void PerformanceEngine::syncTransportState() {
  if (state_ == nullptr) {
    return;
  }

  if (state_->transport.running == transport_running_cache_) {
    return;
  }

  transport_running_cache_ = state_->transport.running;

  if (transport_running_cache_) {
    next_step_due_us_ = micros();
    arp_division_counter_ = 0;
  } else {
    clearGeneratedNote();
  }
}

void PerformanceEngine::advanceTransportStep() {
  if (state_ == nullptr) {
    return;
  }

  const uint8_t current_step = state_->transport.step_index;
  const unsigned long interval = stepIntervalUs(state_->transport, current_step);
  next_step_due_us_ += interval;

  state_->sequencer.playhead = current_step % clampStepLength(state_->sequencer.length);

  if (state_->sequencer.enabled) {
    handleSequencerStep();
  } else if (state_->arp.enabled) {
    handleArpStep();
  }

  state_->transport.step_index =
      static_cast<uint8_t>((current_step + 1U) % clampStepLength(state_->sequencer.length));
  state_->markDirty();
}

void PerformanceEngine::handleSequencerStep() {
  const uint8_t step_index = state_->sequencer.playhead;
  const SequenceStep &step = state_->sequencer.steps[step_index];

  if (!step.active) {
    clearGeneratedNote();
    return;
  }

  const unsigned long interval = stepIntervalUs(state_->transport, step_index);
  const unsigned long gate_duration =
      (interval * static_cast<unsigned long>(step.gate)) / 100UL;
  triggerGeneratedNote(step.note, 108, gate_duration);
}

void PerformanceEngine::handleArpStep() {
  if (held_count_ == 0) {
    clearGeneratedNote();
    return;
  }

  arp_division_counter_ = static_cast<uint8_t>((arp_division_counter_ + 1U) % state_->arp.division);
  if (arp_division_counter_ != 0U) {
    return;
  }

  const uint8_t note = nextArpNote();
  const unsigned long interval =
      stepIntervalUs(state_->transport, state_->transport.step_index) * state_->arp.division;
  const unsigned long gate_duration =
      (interval * static_cast<unsigned long>(state_->arp.gate)) / 100UL;
  triggerGeneratedNote(note, 112, gate_duration);
}

void PerformanceEngine::triggerGeneratedNote(uint8_t note,
                                             uint8_t velocity,
                                             unsigned long duration_us) {
  if (synth_ == nullptr) {
    return;
  }

  if (generated_note_active_) {
    synth_->noteOff(generated_note_);
  }

  generated_note_ = note;
  generated_note_active_ = true;
  synth_->noteOn(note, velocity);
  if (duration_us < 1000UL) {
    duration_us = 1000UL;
  }
  gate_off_due_us_ = micros() + duration_us;
}

void PerformanceEngine::releaseGeneratedNoteIfDue() {
  if (!generated_note_active_ || synth_ == nullptr) {
    return;
  }

  const unsigned long now = micros();
  if (static_cast<long>(now - gate_off_due_us_) < 0) {
    return;
  }

  synth_->noteOff(generated_note_);
  generated_note_active_ = false;
}

void PerformanceEngine::clearGeneratedNote() {
  if (generated_note_active_ && synth_ != nullptr) {
    synth_->noteOff(generated_note_);
  }
  generated_note_active_ = false;
}

void PerformanceEngine::addHeldNote(uint8_t note) {
  for (uint8_t index = 0; index < held_count_; ++index) {
    if (held_notes_[index] == note) {
      return;
    }
  }

  if (held_count_ >= kHeldCapacity) {
    return;
  }

  held_notes_[held_count_] = note;
  ++held_count_;

  for (uint8_t outer = 0; outer + 1U < held_count_; ++outer) {
    for (uint8_t inner = outer + 1U; inner < held_count_; ++inner) {
      if (held_notes_[inner] < held_notes_[outer]) {
        const uint8_t temp = held_notes_[outer];
        held_notes_[outer] = held_notes_[inner];
        held_notes_[inner] = temp;
      }
    }
  }
}

void PerformanceEngine::removeHeldNote(uint8_t note) {
  if (state_ != nullptr && state_->arp.latch) {
    return;
  }

  for (uint8_t index = 0; index < held_count_; ++index) {
    if (held_notes_[index] == note) {
      for (uint8_t move = index; move + 1U < held_count_; ++move) {
        held_notes_[move] = held_notes_[move + 1U];
      }
      --held_count_;
      if (held_count_ == 0) {
        arp_index_ = -1;
      } else if (arp_index_ >= held_count_) {
        arp_index_ = static_cast<int8_t>(held_count_ - 1U);
      }
      return;
    }
  }
}

uint8_t PerformanceEngine::nextArpNote() {
  if (held_count_ == 0) {
    return 60;
  }

  switch (state_->arp.mode) {
    case ArpMode::DOWN:
      if (arp_index_ < 0) {
        arp_index_ = held_count_;
      }
      --arp_index_;
      if (arp_index_ < 0) {
        arp_index_ = held_count_ - 1;
      }
      break;
    case ArpMode::UP_DOWN:
      if (arp_index_ < 0) {
        arp_index_ = 0;
      } else {
        arp_index_ += arp_direction_;
        if (arp_index_ >= held_count_) {
          arp_index_ = held_count_ > 1 ? held_count_ - 2 : 0;
          arp_direction_ = -1;
        } else if (arp_index_ < 0) {
          arp_index_ = held_count_ > 1 ? 1 : 0;
          arp_direction_ = 1;
        }
      }
      break;
    case ArpMode::RANDOM:
      arp_index_ = random(held_count_);
      break;
    case ArpMode::UP:
    default:
      arp_index_ = static_cast<int8_t>((arp_index_ + 1) % held_count_);
      break;
  }

  uint8_t octave_offset = 0;
  if (state_->arp.octave_range > 1) {
    const uint8_t octave_slot =
        static_cast<uint8_t>(state_->transport.step_index % state_->arp.octave_range);
    octave_offset = octave_slot * 12U;
  }

  return static_cast<uint8_t>(min<uint16_t>(held_notes_[arp_index_] + octave_offset, 127));
}

void PerformanceEngine::recordStepFromMidi(uint8_t note) {
  const uint8_t index = state_->sequencer.playhead % 16U;
  SequenceStep &step = state_->sequencer.steps[index];
  step.active = true;
  step.note = note;
  step.gate = 78;
  state_->sequencer.selected_step = index;
  state_->markDirty();
}

bool PerformanceEngine::directPlayEnabled() const {
  return state_ != nullptr && (!state_->arp.enabled) &&
         (!state_->sequencer.enabled || !state_->transport.running);
}

PerformanceEngine performance_engine;
