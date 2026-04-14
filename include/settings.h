#ifndef SETTINGS_H
#define SETTINGS_H

enum class Mode {
  MENU,
  SYNTHESIZER,
  SEQUENCER,
  ARPEGGIATOR,
  INPUT_TEST
};

class Settings {
private:
  // Private constructor to prevent external instantiation
  Settings() {}

  // Static instance variable to hold the single instance
  static Settings* instance;

  // Current mode setting
  Mode mode = Mode::MENU;

public:
  // Static method to access the single instance
  static Settings* getInstance() {
    if (!instance) {
      // Create the instance if it doesn't exist
      instance = new Settings();
      // Set default mode (optional)
      //instance->mode = Mode::SYNTHESIZER;
    }
    return instance;
  }

  // Method to get the current mode
  Mode getMode() const {
    return mode;
  }

  // Method to update the mode
  void setMode(Mode newMode) {
    mode = newMode;
  }

  ~Settings() = default;
};

#endif
