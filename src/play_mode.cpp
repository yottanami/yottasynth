#include "play_mode.h"
#include <Audio.h>
#include <USBHost_t36.h>
#include <EEPROM.h>
#include "app_state.h"
#include "performance_engine.h"

PlayMode::PlayMode() {
  // Constructor
}

// USB Host stack objects.  Two MIDIDevice instances let a keyboard and a
// clock-sending device (e.g. Ableton Move) coexist through a USB hub on the
// single host port; either can carry notes or clock.
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
MIDIDevice midi1(myusb);
MIDIDevice midi2(myusb);

template <typename Device>
void PlayMode::registerHandlers(Device &device) {
  device.setHandleNoteOn(myNoteOn);
  device.setHandleNoteOff(myNoteOff);
  device.setHandleAfterTouchPoly(myAfterTouchPoly);
  device.setHandleControlChange(myControlChange);
  device.setHandleProgramChange(myProgramChange);
  device.setHandleAfterTouchChannel(myAfterTouchChannel);
  device.setHandlePitchChange(myPitchChange);

  // External clock / transport sync.
  device.setHandleClock(myClock);
  device.setHandleStart(myStart);
  device.setHandleContinue(myContinue);
  device.setHandleStop(myStop);
}

void PlayMode::setup() {
  Serial.begin(115200);

  // Wait 1.5 seconds before turning on USB Host.  If connected USB devices
  // use too much power, Teensy at least completes USB enumeration, which
  // makes isolating the power issue easier.
  delay(1500);
  Serial.println("USB Host InputFunctions example");
  delay(10);
  myusb.begin();

  // Host-side devices (MIDI keyboard, and a second device through a hub).
  registerHandlers(midi1);
  registerHandlers(midi2);

  // Device-side: Teensy enumerates as a USB-MIDI device on the native port
  // (USB_MIDI_SERIAL), so a computer / DAW such as Ableton Live can send clock
  // and notes over the same cable used to upload code.
  registerHandlers(usbMIDI);

  // Extra host-side handlers kept on midi1 for diagnostics / completeness.
  midi1.setHandleSystemExclusive(mySystemExclusiveChunk);
  midi1.setHandleSystemExclusive(mySystemExclusive);
  midi1.setHandleTimeCodeQuarterFrame(myTimeCodeQuarterFrame);
  midi1.setHandleSongPosition(mySongPosition);
  midi1.setHandleSongSelect(mySongSelect);
  midi1.setHandleTuneRequest(myTuneRequest);
  midi1.setHandleActiveSensing(myActiveSensing);
  midi1.setHandleSystemReset(mySystemReset);

  // This generic System Real Time handler is only used if the
  // more specific ones are not set.
  midi1.setHandleRealTimeSystem(myRealTimeSystem);
}

void PlayMode::loop() {
  // The handler functions are called when the MIDI sources read data.  They
  // will not be called automatically; we must poll each source regularly from
  // loop() for handlers to run as messages arrive.
  myusb.Task();
  midi1.read();
  midi2.read();
  usbMIDI.read();

  const bool connected = static_cast<bool>(midi1) || static_cast<bool>(midi2);
  if (static_cast<bool>(midi1)) {
    AppState::instance().updateMidiDevice(true, midi1.idVendor(), midi1.idProduct());
  } else if (static_cast<bool>(midi2)) {
    AppState::instance().updateMidiDevice(true, midi2.idVendor(), midi2.idProduct());
  } else {
    AppState::instance().updateMidiDevice(connected, 0, 0);
  }
}

// Handlers implemented as PlayMode:: static methods

void PlayMode::myNoteOn(byte channel, byte note, byte velocity) {
  AppState::instance().registerMidiNote(note, velocity);
  performance_engine.onMidiNoteOn(channel, note, velocity);
}

void PlayMode::myNoteOff(byte channel, byte note, byte velocity) {
  performance_engine.onMidiNoteOff(channel, note, velocity);
}

void PlayMode::myAfterTouchPoly(byte channel, byte note, byte velocity) {
  (void)channel;
  (void)note;
  (void)velocity;
}

void PlayMode::myControlChange(byte channel, byte control, byte value) {
  (void)channel;
  (void)control;
  (void)value;
}

void PlayMode::myProgramChange(byte channel, byte program) {
  (void)channel;
  (void)program;
}

void PlayMode::myAfterTouchChannel(byte channel, byte pressure) {
  (void)channel;
  (void)pressure;
}

void PlayMode::myPitchChange(byte channel, int pitch) {
  performance_engine.onMidiPitchBend(channel, pitch);
}

void PlayMode::mySystemExclusiveChunk(const uint8_t *data, uint16_t length, bool last) {
  (void)data;
  (void)length;
  (void)last;
}

void PlayMode::mySystemExclusive(uint8_t *data, unsigned int length) {
  (void)data;
  (void)length;
}

void PlayMode::myTimeCodeQuarterFrame(byte data) {
  (void)data;
}

void PlayMode::mySongPosition(uint16_t beats) {
  (void)beats;
}

void PlayMode::mySongSelect(byte songNumber) {
  (void)songNumber;
}

void PlayMode::myTuneRequest() {
}

void PlayMode::myClock() {
  performance_engine.onMidiClockTick();
}

void PlayMode::myStart() {
  performance_engine.onMidiStart();
}

void PlayMode::myContinue() {
  performance_engine.onMidiContinue();
}

void PlayMode::myStop() {
  performance_engine.onMidiStop();
}

void PlayMode::myActiveSensing() {
}

void PlayMode::mySystemReset() {
}

void PlayMode::myRealTimeSystem(uint8_t realtimebyte) {
  (void)realtimebyte;
}

void PlayMode::printBytes(const uint8_t *data, unsigned int size) {
  while (size > 0) {
    uint8_t b = *data++;
    if (b < 16) Serial.print('0');
    Serial.print(b, HEX);
    if (size > 1) Serial.print(' ');
    size = size - 1;
  }
}
