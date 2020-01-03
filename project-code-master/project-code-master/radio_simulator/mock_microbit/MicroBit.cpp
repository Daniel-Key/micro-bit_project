#include "MicroBit.h"
// most code is in MicroBit.h in classes

// create_fiber is roughly equivalent to starting in a new thread
void create_fiber(void (*func)()) {
    std::thread(func).detach();
}
