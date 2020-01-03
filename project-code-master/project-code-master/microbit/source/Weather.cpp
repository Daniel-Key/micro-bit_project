#include "Weather.h"

extern MicroBit uBit;

Weather::Weather(InteractiveComponent* parent) : Selector(parent) {
}

void Weather::controlGiven() {
    // request the weather
    bool sent = serial::server_request(SERIAL_REQUEST_WEATHER, " ");
    uBit.display.clear();
    ManagedString weatherStr;
    // if the request was sent get the repy and show the weather else say "no
    // connection"
    if (sent) {
        weatherStr = serial::receive();
    } else {
        weatherStr = "no connection";
    }
    uBit.display.scroll(weatherStr);

    toParent();
}
