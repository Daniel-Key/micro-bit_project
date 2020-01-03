#include "Protocol.h"
#include "config/ProtocolConfig.h"

Message* ack;
extern MicroBit uBit;

void Protocol::init() {
}

PacketBuffer Protocol::encodeString(char* plaintextCharArray, char length,
                                    char message_type, char recipient) {
    PacketBuffer buf(4 + length);
    char* encodedCharArray = (char*)buf.getBytes();

    // Add recipient ID
    encodedCharArray[0] = recipient;
    // Add sender ID
    encodedCharArray[1] = MICROBIT_ID;
    // Add message type- data type in this case
    encodedCharArray[2] = message_type;
    // Add priority flag- assuming highest priority
    int priority = PRIORITY_VALUE_HIGH;
    encodedCharArray[3] = priority << 6;
    // Add sequence flag- assuming only one packet being sent for now
    int sequence = 0b00;
    encodedCharArray[3] |= sequence << 4;
    // Calculate checksum bits and add checksum flag
    encodedCharArray[3] |= checksum(plaintextCharArray, length);

    // Add data
    for (int i = 0; i < length; i++) {
        encodedCharArray[i + 4] = plaintextCharArray[i];
    }

    // Add zero byte
    encodedCharArray[length + 4] = '\0';

    return buf;
}

char Protocol::checksum(char* plaintextCharArray, size_t len) {
    char checksum = 0;
    for (size_t i = 0; i < len; i++) {
        checksum ^=
            plaintextCharArray[i] & 0b00001111; // xor with lower half of byte
        checksum ^= plaintextCharArray[i] >> 4; // xor with upper half of byte
    }
    return checksum;
}

bool Protocol::send_message(ManagedString plaintextString, char recipient) {
    ack = NULL;

    PacketBuffer encoded;
    // char length = (char)plaintextString.length();
    char length = 1;

    encoded =
        Protocol::encodeString(&length, 1, MESSAGE_TYPE_REQUEST, recipient);

    uBit.radio.datagram.send(encoded);
    // wait 2 seconds for an acknowledgement
    for (size_t i = 0; i < 10 && !ack; i++) {
        uBit.sleep(200);
    }
    if (!ack) {
        return false;
    } else {
        delete ack;
        ack = NULL;
        length = plaintextString.length();
        encoded = Protocol::encodeString((char*)plaintextString.toCharArray(),
                                         length, MESSAGE_TYPE_DATA, recipient);
        uBit.radio.datagram.send(encoded);
        return true;
    }
}

void Protocol::acknowledge(char recipient) {
    PacketBuffer encoded =
        Protocol::encodeString(NULL, 0, MESSAGE_TYPE_ACKNOWLEDGE, recipient);
    uBit.radio.datagram.send(encoded);
}

void Protocol::receive_message(void (*callback)(Message), int signal_strength) {

    (void)signal_strength;

    PacketBuffer message_bytes = uBit.radio.datagram.recv();

    Message msg(message_bytes);

    if (msg.getRecipientID() != MICROBIT_ID && msg.getRecipientID() != 0) {
        return;
    }
    switch (msg.getMessageType()) {
    case MESSAGE_TYPE_ACKNOWLEDGE:
        ack = new Message(msg);
        break;
    case MESSAGE_TYPE_DATA:
        (*callback)(msg);
        break;
    case MESSAGE_TYPE_REQUEST:
        Protocol::acknowledge(msg.getSenderID());
        break;
    }
}

// Message
Message::Message(PacketBuffer encodedPacketArg) {
    bytes = new char[encodedPacketArg.length() + 1];
    memcpy(bytes, encodedPacketArg.getBytes(), encodedPacketArg.length());
    bytes[encodedPacketArg.length()] = '\0';
    // if (bytes[2] == MESSAGE_TYPE_DATA)
    //     display_bytes(bytes, encodedPacketArg.length() + 1);

    length = encodedPacketArg.length() - 4;
    ref_count = new size_t(1);
}

Message::Message(const Message& message) {
    ref_count = message.ref_count;
    (*ref_count)++;
    bytes = message.bytes;
    length = message.length;
}
void Message::operator=(const Message& other) {
    if (this == &other) {
        return;
    }
    ref_count = other.ref_count;
    (*ref_count)++;
    bytes = other.bytes;
    length = other.length;
}

Message::~Message() {
    (*ref_count)--;
    if (*ref_count == 0) {
        delete[] bytes;
    }
}
int Message::getRecipientID() {
    return (int)bytes[0];
}
int Message::getSenderID() {
    return (int)bytes[1];
}
char Message::getMessageType() {
    return bytes[2];
}

char Message::getPriority() {
    return bytes[3] >> 6;
}
char Message::getSequenceNum() {
    return (bytes[3] >> 4) & 0b11;
}
bool Message::checksumValid() {
    char checksum = bytes[3] & 0b00001111;
    return (Protocol::checksum(getContent(), size()) ^ checksum) == 0b0000;
}
size_t Message::size() {
    return length;
}

char* Message::getContent() {
    return &bytes[4];
}
