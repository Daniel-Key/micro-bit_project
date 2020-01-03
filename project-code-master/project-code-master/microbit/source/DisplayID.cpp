#include "DisplayID.h"

extern MicroBit uBit;

DisplayID::DisplayID(InteractiveComponent* parent)
    : InteractiveComponent(parent) {
}
void DisplayID::show() {

    uBit.display.clear();
    char buf[10];
    snprintf(buf, 10, "ID:%d", MICROBIT_ID);

    uBit.display.scroll(buf);
    toParent();
}
