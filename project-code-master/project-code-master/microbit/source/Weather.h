#ifndef WEATHER_H
#define WEATHER_H

#include "MicroBit.h"
#include "Selector.h"

#include "Serial.h"

// A component for reading the current weather
class Weather : public Selector {
  public:
    Weather(InteractiveComponent* parent);

    void controlGiven() override;
};

#endif /* end of include guard: WEATHER_H */
