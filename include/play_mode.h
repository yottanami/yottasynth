#ifndef PLAY_MODE_H
#define PLAY_MODE_H
#include <Arduino.h>
#include <Audio.h>

class PlayMode {
 public:
  PlayMode();
  void setup();
  void loop();

  // Utility to print bytes
  static void printBytes(const uint8_t *data, unsigned int size);

 private:
  // Registers the common MIDI handlers on any device exposing the USBHost_t36 /
  // usbMIDI setHandle* API (host-side MIDIDevice instances and the device-side
  // usbMIDI object).  Defined in play_mode.cpp; instantiated there only.
  template <typename Device>
  static void registerHandlers(Device &device);

  // MIDI handlers (signatures aligned with USBHost_t36 and usbMIDI)
  static void myNoteOn(byte channel, byte note, byte velocity);
  static void myNoteOff(byte channel, byte note, byte velocity);
  static void myControlChange(byte channel, byte control, byte value);
  static void myProgramChange(byte channel, byte program);
  static void mySystemExclusive(uint8_t *data, unsigned int length);
  static void myRealTimeSystem(uint8_t realtimebyte);
  static void myStart();
  static void myStop();
  static void myContinue();
  static void myClock();
  static void myActiveSensing();
  static void mySystemReset();
  static void myAfterTouchPoly(byte channel, byte note, byte pressure);
  static void myAfterTouchChannel(byte channel, byte pressure);
  static void myPitchChange(byte channel, int pitch);
  static void mySongPosition(uint16_t beats);
  static void mySongSelect(byte songnumber);
  static void myTuneRequest();
  static void myTimeCodeQuarterFrame(byte data);

  // Teensy USBHost_t36 extended SysEx handler (chunked)
  static void mySystemExclusiveChunk(const uint8_t *data, uint16_t length, bool last);
};

#endif
