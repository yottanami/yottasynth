#ifndef MUX_PINS_H
#define MUX_PINS_H

#include <Arduino.h>

namespace MuxPins {
constexpr uint8_t kS0 = 14;
constexpr uint8_t kS1 = 15;
constexpr uint8_t kS2 = 16;
constexpr uint8_t kS3 = 17;
constexpr uint8_t kSignal = 22;
constexpr uint8_t kSelectPins[] = {kS0, kS1, kS2, kS3};
}

#endif
