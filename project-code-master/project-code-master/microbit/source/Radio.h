#ifndef RADIO_H_
#define RADIO_H_

#include "InteractiveComponent.h"
#include "MicroBit.h"
#include "Piano.h"
#include "Protocol.h"
#include "Serial.h"
#include "data-structures/Queue.h"

#include <stdlib.h>
#include <string.h>

using namespace std;
extern int piano;

// functions for accessing radio
namespace radio {
    void init();
    // encrpyts a given message with the public key of the m:b it's sending to
    void encrypt(char* text, int length);
    // decrypts received message with private key
    void decrypt(char* cypherText, int length);
    // gets a message from the queue
    Message get_message();
    // sends a raw message over radio or serial as appropriate
    void send(PacketBuffer msg, int recipient);
    // enqueue a message upon receiving it
    void enqueue_message(Message msg);
}; // namespace radio

// a component that receives a message and displays it, before returning to the
// parent
class RadioReceiver : public InteractiveComponent {
  private:
    MicroBitImage* heart;

  public:
    RadioReceiver(InteractiveComponent* parent);

    void controlGiven() override;
};

//  A component that receives a Tweet and displays it before returning to the
//  parent.
class TwitterReceiver : public InteractiveComponent {
  private:
    MicroBitImage* birb;

  public:
    TwitterReceiver(InteractiveComponent* parent);

    void controlGiven() override;
};

#endif /* end of include guard: RADIO_H_ */
