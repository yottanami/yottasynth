#include "play_mode.h"
#include <Audio.h>
#include <USBHost_t36.h>
#include <EEPROM.h>
#include "app_state.h"
#include "performance_engine.h"

PlayMode::PlayMode() {
  // Constructor
}

// USB Host stack objects
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
MIDIDevice midi1(myusb);

void PlayMode::setup() {
  Serial.begin(115200);

  // Wait 1.5 seconds before turning on USB Host.  If connected USB devices
  // use too much power, Teensy at least completes USB enumeration, which
  // makes isolating the power issue easier.
  delay(1500);
  Serial.println("USB Host InputFunctions example");
  delay(10);
  myusb.begin();

  // Register handlers - these names resolve to PlayMode:: static methods
  midi1.setHandleNoteOn(myNoteOn);
  midi1.setHandleNoteOff(myNoteOff);
  midi1.setHandleAfterTouchPoly(myAfterTouchPoly);
  midi1.setHandleControlChange(myControlChange);
  midi1.setHandleProgramChange(myProgramChange);
  midi1.setHandleAfterTouchChannel(myAfterTouchChannel);
  midi1.setHandlePitchChange(myPitchChange);

  // Only one of these System Exclusive handlers will actually be used.
  // If both are set, the 3-argument chunked version is called.
  midi1.setHandleSystemExclusive(mySystemExclusiveChunk);
  midi1.setHandleSystemExclusive(mySystemExclusive);

  midi1.setHandleTimeCodeQuarterFrame(myTimeCodeQuarterFrame);
  midi1.setHandleSongPosition(mySongPosition);
  midi1.setHandleSongSelect(mySongSelect);
  midi1.setHandleTuneRequest(myTuneRequest);
  midi1.setHandleClock(myClock);
  midi1.setHandleStart(myStart);
  midi1.setHandleContinue(myContinue);
  midi1.setHandleStop(myStop);
  midi1.setHandleActiveSensing(myActiveSensing);
  midi1.setHandleSystemReset(mySystemReset);

  // This generic System Real Time handler is only used if the
  // more specific ones are not set.
  midi1.setHandleRealTimeSystem(myRealTimeSystem);
}

void PlayMode::loop() {
  // The handler functions are called when midi1 reads data.  They
  // will not be called automatically.  You must call midi1.read()
  // regularly from loop() for midi1 to actually read incoming
  // data and run the handler functions as messages arrive.
  myusb.Task();
  midi1.read();
  AppState::instance().updateMidiDevice(static_cast<bool>(midi1), midi1.idVendor(), midi1.idProduct());
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
  Serial.print("AfterTouch Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", note=");
  Serial.print(note, DEC);
  Serial.print(", velocity=");
  Serial.println(velocity, DEC);
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
  Serial.print("SysEx Message: ");
  PlayMode::printBytes(data, length);
  if (last) {
    Serial.println(" (end)");
  } else {
    Serial.println(" (to be continued)");
  }
}

void PlayMode::mySystemExclusive(uint8_t *data, unsigned int length) {
  Serial.print("SysEx Message: ");
  PlayMode::printBytes(data, length);
  Serial.println();
}

void PlayMode::myTimeCodeQuarterFrame(byte data) {
  static char SMPTE[8]={'0','0','0','0','0','0','0','0'};
  static byte fps=0;
  byte index = data >> 4;
  byte number = data & 15;
  if (index == 7) {
    fps = (number >> 1) & 3;
    number = number & 1;
  }
  if (index < 8 || number < 10) {
    SMPTE[index] = number + '0';
    Serial.print("TimeCode: ");  // perhaps only print when index == 7
    Serial.print(SMPTE[7]);
    Serial.print(SMPTE[6]);
    Serial.print(':');
    Serial.print(SMPTE[5]);
    Serial.print(SMPTE[4]);
    Serial.print(':');
    Serial.print(SMPTE[3]);
    Serial.print(SMPTE[2]);
    Serial.print('.');
    Serial.print(SMPTE[1]);  // perhaps add 2 to compensate for MIDI latency?
    Serial.print(SMPTE[0]);
    switch (fps) {
      case 0: Serial.println(" 24 fps"); break;
      case 1: Serial.println(" 25 fps"); break;
      case 2: Serial.println(" 29.97 fps"); break;
      case 3: Serial.println(" 30 fps"); break;
    }
  } else {
    Serial.print("TimeCode: invalid data = ");
    Serial.println(data, HEX);
  }
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
}

void PlayMode::myStart() {
}

void PlayMode::myContinue() {
}

void PlayMode::myStop() {
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
