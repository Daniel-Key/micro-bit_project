#include "Serial.h"

#include "Protocol.h"
#include "Radio.h"

extern MicroBit uBit;

#define SERIAL_BUFFER_SIZE 8

// the queue of serial messages
Queue<struct serial_msg> serial_messages;

void serial::send(int message_type, ManagedString message) {
    serial::send(message_type, (char*)message.toCharArray(), message.length());
}

// if connected to the computer over serial:
bool serial_enabled = false;

void serial::init() {
    uBit.serial.setRxBufferSize(127);
    uBit.serial.baud(9600);
    uBit.messageBus.listen(MICROBIT_ID_SERIAL, MICROBIT_SERIAL_EVT_DELIM_MATCH,
                           serial::on_serial);
    uBit.serial.eventOn("\n");

    // send a ping to determine if connected to serial
    serial::send(SERIAL_PING, " ");
    uBit.sleep(200);
    if (!serial_messages.empty()) {
        serial_messages.dequeue();
    }
}

void serial::send(int message_type, char* message, size_t len) {
    // send in the format "[type]\n[length]\n[data]"
    char buf[12];
    snprintf(buf, 12, "%d\n%d\n", message_type, len);
    uBit.serial.send(buf);
    uBit.serial.send((uint8_t*)message, (int)len);
}

ManagedString serial::receive(int* message_type) {
    // sleep up to 5 seconds to wait for a message to arrive
    for (size_t i = 0; i < 10 && serial_messages.empty(); i++) {
        uBit.sleep(500);
    }

    while (serial_messages.empty()) {
        uBit.display.scroll("error");
    }

    struct serial_msg msg = serial_messages.dequeue();

    *message_type = msg.type;

    return msg.msg;
}

ManagedString serial::receive() {
    int message_type;
    return serial::receive(&message_type);
}

bool serial_receive_toggle = true;
void serial::on_serial(MicroBitEvent e) {
    (void)e; // silence unused param warning

    // if a serial message is received, serial must be enabled
    serial_enabled = true;
    // stop calls overlapping
    if (!serial_receive_toggle) {
        return;
    }
    serial_receive_toggle = false;

    struct serial_msg msg;

    // get message
    int message_length;
    {
        ManagedString message_type_str = uBit.serial.readUntil("\n");
        ManagedString message_length_str = uBit.serial.readUntil("\n");
        sscanf(message_type_str.toCharArray(), "%d", &msg.type);
        sscanf(message_length_str.toCharArray(), "%d", &message_length);
    }
    if (msg.type == SERIAL_MESH_FORWARD) {
        // if the message is a mesh message, then treat it as radio
        uint8_t* buf = new uint8_t[message_length];

        uBit.serial.read(buf, message_length);
        PacketBuffer pb(buf, message_length);
        delete[] buf;

        Protocol::receive_message(pb, radio::enqueue_message, 0);
    } else {
        // if it is not mesh, treat it as serial
        msg.msg = uBit.serial.read(message_length);

        serial_messages.enqueue(msg);
    }

    serial_receive_toggle = true;
}

bool serial::server_request(ManagedString username, int pattern,
                            int message_type, ManagedString msg) {
    // encode the login data, message type and message into a buffer
    size_t len = username.length() + 1 + sizeof(pattern) +
                 sizeof(message_type) + msg.length();

    PacketBuffer buf(len);
    char* bytes = (char*)buf.getBytes();

    // copy username
    memcpy(bytes, username.toCharArray(), username.length());
    bytes += username.length();

    // add new line
    bytes[0] = '\n';
    bytes++;

    // copy pattern
    memcpy(bytes, (char*)&pattern, sizeof(pattern));
    bytes += sizeof(pattern);

    // copy message_type
    memcpy(bytes, (char*)&message_type, sizeof(message_type));
    bytes += sizeof(message_type);

    // copy msg
    memcpy(bytes, msg.toCharArray(), msg.length());

    // send to server address
    return Protocol::send_message(buf, 0b100);
}

bool serial::server_request(int message_type, ManagedString msg) {
    // get values from storage
    KeyValuePair* username_stored = uBit.storage.get("user");
    KeyValuePair* pattern_stored = uBit.storage.get("lock");

    ManagedString username;
    int pattern;

    char username_buf[32];
    memcpy(&username_buf, username_stored->value, 32);
    delete username_stored;
    username = username_buf;

    memcpy((char*)&pattern, (char*)pattern_stored->value, sizeof(pattern));
    delete pattern_stored;

    return serial::server_request(username, pattern, message_type, msg);
}

bool serial::logged_in() {
    // if username has been stored, then logged in
    KeyValuePair* username_stored = uBit.storage.get("user");
    delete username_stored;
    return username_stored != NULL;
}
