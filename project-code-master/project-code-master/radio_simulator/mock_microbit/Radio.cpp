
#include "Radio.h"

extern MicroBit uBit;

// the only radio function used in the simulator - in the simulator all packets
// should be sent over 'radio'
void radio::send(PacketBuffer msg, int recipient) {
    uBit.radio.datagram.send(msg);
}
