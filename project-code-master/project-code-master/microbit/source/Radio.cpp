#include "Radio.h"

extern MicroBit uBit;

Queue<Message> received;

extern bool serial_enabled;
extern Queue<struct serial_msg> serial_messages;

int PRIVATE_KEY;

void receive_message(MicroBitEvent event);

void radio::init() {
    uBit.messageBus.listen(MICROBIT_ID_RADIO, MICROBIT_RADIO_EVT_DATAGRAM,
                           receive_message, MESSAGE_BUS_LISTENER_REENTRANT);
    uBit.radio.enable();
    uBit.radio.setGroup(86);
    uBit.radio.setTransmitPower(7);
    uBit.radio.setFrequencyBand(0);
}

// sends server-addressed packets over serial if possible, else over radio
void radio::send(PacketBuffer msg, int recipient) {
    if (recipient == 0b100 && serial_enabled) {
        serial::send(SERIAL_MESH_FORWARD, msg);
    } else {
        uBit.radio.datagram.send(msg);
    }
}

// a function that is called with incoming packets addressed to this device
void radio::enqueue_message(Message msg) {
    // if the packet comes from the server, treat it as a serial message
    if (msg.getSenderID() == 0b100) {
        struct serial_msg serial_msg;
        serial_msg.type = msg.getContent()[0];
        serial_msg.msg = &msg.getContent()[1];
        serial_messages.enqueue(serial_msg);
    } else {
        // else treat it as a radio message
        received.enqueue(msg);
        // attempt to send it to the history log
        if (serial::logged_in()) {
            char buf[256];
            snprintf(buf, 256, "%d,%s", msg.getSenderID(), msg.getContent());

            bool success = serial::server_request(SERIAL_LOG_MESSAGE, buf);
            if (success) {
                serial::receive();
            }
        }
    }
    // piano notification sound
    if (piano == 1) {
        notification();
    }
}

// radio message handler - called for ALL incoming packets
void receive_message(MicroBitEvent event) {
    (void)event; // silence unused param warning

    // get message
    PacketBuffer message_bytes = uBit.radio.datagram.recv();

    // get
    Protocol::receive_message(message_bytes, radio::enqueue_message,
                              uBit.radio.getRSSI());
}

RadioReceiver::RadioReceiver(InteractiveComponent* parent)
    : InteractiveComponent(parent) {
    heart = new MicroBitImage("0,255,0,255,0\n"
                              "255,255,255,255,255\n"
                              "255,255,255,255,255\n"
                              "0,255,255,255,0\n"
                              "0,20,255,20,0\n");
}

void RadioReceiver::controlGiven() {
    if (received.empty()) {
        uBit.display.print(*heart);
        uBit.sleep(1000);
    } else {
        uBit.display.clear();
        ManagedString msg(received.dequeue().getContent());
        uBit.display.scroll(msg);

        // delete m;
    }
    toParent();
}

TwitterReceiver::TwitterReceiver(InteractiveComponent* parent)
    : InteractiveComponent(parent) {
    birb = new MicroBitImage("0,0,255,255,0\n"
                             "0,0,255,255,255\n"
                             "0,100,255,255,0\n"
                             "255,255,255,100,0\n"
                             "0,255,255,0,0\n");
}

//  When Twitter gives control to TwitterReceiver, this gets called.
//  If this is called, then the user wants to receive a Tweet.
void TwitterReceiver::controlGiven() {
    //  Because lol
    uBit.display.print(*birb);
    uBit.sleep(1000);

    //  Get the tweet.
    bool sent = serial::server_request(SERIAL_REQUEST_TWEET, "t");
    ManagedString tweetStr;
    if (sent) {
        tweetStr = serial::receive();
    } else {
        tweetStr = "no connection";
    }

    //  Display the tweet.
    uBit.display.clear();
    uBit.display.scroll(tweetStr);

    //  Give control back to Twitter
    toParent();
}
