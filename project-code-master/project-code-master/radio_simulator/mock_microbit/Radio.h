#ifndef RADIO_H
#define RADIO_H

#include "MicroBit.h"

namespace radio {
    void send(PacketBuffer msg, int recipient);
}

#endif /* end of include guard: RADIO_H */
