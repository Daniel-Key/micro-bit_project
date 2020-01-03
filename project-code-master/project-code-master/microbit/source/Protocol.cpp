#include "Protocol.h"
#include "config/Config.h"
#include "config/ProtocolConfig.h"

#include "Radio.h"

#include "Encryption.h"

Message* ack;
extern MicroBit uBit;
// Initial packet ID flag is 000
int packetID = 0b000;

// The cache is used to store messages recently received to prevent forwarding
// or processing them again The messages are identified by the sender ID and
// packet ID
typedef struct {
    char sender_id;
    char packet_id;
    uint64_t timestamp;
} CacheEntry;

// Up to 4 recently received messages are stored
#define CACHE_SIZE 4
// The messages are stored for up to half a second
#define CACHE_TIMEOUT 500

CacheEntry cache[CACHE_SIZE];

// Add message to cache if there's a free space, return true if it was already
// there
bool add_to_cache(char sender_id, char packet_id) {
    uint64_t timestamp = uBit.systemTime();
    for (size_t i = 0; i < CACHE_SIZE; i++) {
        if (cache[i].timestamp != 0 && cache[i].sender_id == sender_id &&
            cache[i].packet_id == packet_id) {

            cache[i].timestamp = timestamp;

            return true;
        }
    }

    for (size_t i = 0; i < CACHE_SIZE; i++) {
        if (cache[i].timestamp == 0) {
            cache[i].sender_id = sender_id;
            cache[i].packet_id = packet_id;
            cache[i].timestamp = timestamp;
            break;
        }
    }

    return false;
}

// Method to remove all messages from the cache which have exceeded the storage
// timeout
void purge_cache() {
    uint64_t timestamp = uBit.systemTime();

    for (size_t i = 0; i < CACHE_SIZE; i++) {
        if (timestamp - cache[i].timestamp > CACHE_TIMEOUT) {
            cache[i].timestamp = 0;
        }
    }
}

void Protocol::init() {
    for (size_t i = 0; i < CACHE_SIZE; i++) {
        cache[i].timestamp = 0;
    }
}

// Method to encode a message string with the header required by the supergroup
// protocol
PacketBuffer encodeString(char* plaintextCharArray, uint16_t length,
                          char message_type, char recipient, bool encrypt) {
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
    // Add packetID
    // Add packetID flag and increment it
    encodedCharArray[3] |= packetID << 3;
    packetID++;
    packetID &= 0b111;

    // Add encrypt flag
    encodedCharArray[3] |= encrypt << 2;

    // Add hop count flag- setting to maximum hop count of 3
    int hopCount = MAXIMUM_HOP_COUNT;
    encodedCharArray[3] |= hopCount;

    // Add data
    memcpy(&encodedCharArray[4], plaintextCharArray, length);

    return buf;
}

Message* Protocol::decodeString(PacketBuffer encodedPacket) {
    return new Message(encodedPacket);
}

// Method to send a message over radio using the supergroup protocol
bool Protocol::send_message(PacketBuffer plaintextString, char recipient) {
    ack = NULL;

    PacketBuffer encoded;
    // char length = (char)plaintextString.length();

    encoded = encodeString(NULL, 0, MESSAGE_TYPE_REQUEST, recipient,
                           ENCRYPTION_ENABLED);

    radio::send(encoded, recipient);

    // wait 2 seconds for an acknowledgement
    for (size_t i = 0; i < 10 && !ack; i++) {
        uBit.sleep(200);
    }
    if (!ack) {
        return false;
    } else {
        add_to_cache(MICROBIT_ID, packetID);

        uint64_t* ack_content = (uint64_t*)ack->getContent();

        uint16_t length = plaintextString.length();
        char* plain_chars = (char*)plaintextString.getBytes();

        int encrypted_length;
        if (ack->isEncrypted() && ack->size() == 16) {
            char* encrypted =
                encryption::encrypt(plain_chars, length, ack_content[0],
                                    ack_content[1], &encrypted_length);

            encoded = encodeString(encrypted, encrypted_length,
                                   MESSAGE_TYPE_DATA, recipient, true);

            delete[] encrypted;
        } else {
            encoded = encodeString(plain_chars, length, MESSAGE_TYPE_DATA,
                                   recipient, false);
        }
        delete ack;
        ack = NULL;

        radio::send(encoded, recipient);

        return true;
    }
}

void Protocol::receive_message(PacketBuffer message_bytes,
                               void (*callback)(Message), int signal_strength) {
    (void)signal_strength;

    Message msg(message_bytes);

    purge_cache();

    bool in_cache = add_to_cache(msg.getSenderID(), msg.getPacketID());

    if (in_cache) {
        return;
    }

    if (msg.getRecipientID() != MICROBIT_ID && msg.getRecipientID() != 0) {
        // Mesh network stuff

        // Check if hop count will be 0 when decremented
        if (msg.getHopCount() == 0) {
            // Drops message
            return;
        }
        // Create edited message with decremented hop count
        msg.decrementHopCount();

        // Resend message
        char* messageBytes = msg.getEncodedMessage();

        PacketBuffer buf((uint8_t*)messageBytes, msg.size() + 4);

        radio::send(buf, msg.getRecipientID());

        return;
    }

    switch (msg.getMessageType()) {
    case MESSAGE_TYPE_ACKNOWLEDGE:
        ack = new Message(msg);
        break;
    case MESSAGE_TYPE_DATA: {
        if (msg.isEncrypted()) {
            int decrypted_length;
            char* decrypted = encryption::decrypt(msg.getContent(), msg.size(),
                                                  &decrypted_length);

            PacketBuffer new_msg_bytes(4 + decrypted_length);
            memcpy(new_msg_bytes.getBytes(), msg.getEncodedMessage(), 4);
            memcpy(new_msg_bytes.getBytes() + 4, decrypted, decrypted_length);

            Message new_msg(new_msg_bytes);

            delete[] decrypted;

            (*callback)(new_msg);
        } else {
            (*callback)(msg);
        }
        break;
    }
    case MESSAGE_TYPE_REQUEST: {
        uint64_t* buf = new uint64_t[2];
        buf[0] = PUBLIC_KEY;
        buf[1] = RSA_N;
        PacketBuffer encoded =
            encodeString((char*)buf, 16, MESSAGE_TYPE_ACKNOWLEDGE,
                         msg.getSenderID(), msg.isEncrypted());
        radio::send(encoded, msg.getSenderID());
        break;
    }
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

Message::Message(const Message& other) {
    *this = other;
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
        delete ref_count;
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

char Message::getPacketID() {
    return (bytes[3] >> 3) & 0b111;
}

bool Message::isEncrypted() {
    return (bytes[3] >> 2) & 0b1;
}

char Message::getHopCount() {
    return (bytes[3]) & 0b11;
}
void Message::decrementHopCount() {
    int currentHopCount = getHopCount();

    bytes[3] &= 0b11111100;
    bytes[3] |= currentHopCount - 1;
}

size_t Message::size() {
    return length;
}

char* Message::getContent() {
    return &bytes[4];
}
char* Message::getEncodedMessage() {
    return bytes;
}
