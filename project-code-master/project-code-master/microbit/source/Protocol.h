#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "MicroBit.h"

#include <stdlib.h>
#include <string.h>

#define MESSAGE_TYPE_REQUEST 0b00
#define MESSAGE_TYPE_ACKNOWLEDGE 0b01
#define MESSAGE_TYPE_DATA 0b10

#define PRIORITY_VALUE_HIGH 0b00
#define PRIORITY_VALUE_NORMAL 0b01
#define PRIORITY_VALUE_LOW 0b10
#define PRIORITY_VALUE_NONE 0b11

#define MAXIMUM_HOP_COUNT 0b11

using namespace std;

// A message sent over the protocol
class Message {
  private:
    size_t length = 0;
    char* bytes = NULL;
    size_t* ref_count = NULL;

  public:
    // a constructor for a blank packet
    Message() {
    }
    // the contructor: takes the raw message received over radio
    Message(PacketBuffer encodedPacket);
    // copy a message
    Message(const Message& message);
    ~Message();
    // assign a message
    void operator=(const Message& other);

    // getters for message metadata
    int getRecipientID();
    int getSenderID();
    char getMessageType();
    char getPriority();
    char getSequenceNum();
    char getPacketID();
    char getHopCount();
    bool isEncrypted();
    // decrements the hop count
    void decrementHopCount();
    // the size of the message
    size_t size();
    // the content
    char* getContent();
    char* getEncodedMessage();
};

// the Protocol function
namespace Protocol {
    void init();
    // sends a message according to the protocol. returns true if an
    // acknowledgement was received
    bool send_message(PacketBuffer plaintextString, char recipient);
    // decodes a string into a message
    Message* decodeString(PacketBuffer encodedPacket);
    // sends an acknowledgement
    void acknowledge(char recipient);
    // receives 1 message
    void receive_message(PacketBuffer message_bytes, void (*callback)(Message),
                         int signal_strength);
} // namespace Protocol

#endif
