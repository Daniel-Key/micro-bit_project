#include <memory>
#include <stdio.h>

#include <thread>

#include "Protocol.h"
#include "Type.h"
#include "config/ProtocolConfig.h"

int MICROBIT_ID = 0;
uint64_t PRIVATE_KEY = 0;
uint64_t PUBLIC_KEY = 0;
uint64_t RSA_N = 0;

MicroBit uBit;

void on_message(Message msg) {
    // when a message is received, report it on stdout
    int sender = msg.getSenderID();
    char* buf = new char[msg.size() + sizeof(sender)];
    memcpy(buf, &sender, sizeof(sender));
    memcpy(buf + sizeof(sender), msg.getContent(), msg.size());

    log(RECV_MSG, msg.size() + sizeof(sender), buf);

    delete[] buf;
}

PacketBuffer _recv;

int main() {
    // get your keys
    scanf("%d", &MICROBIT_ID);
    scanf("%lu", &PRIVATE_KEY);
    scanf("%lu", &PUBLIC_KEY);
    scanf("%lu", &RSA_N);
    Protocol::init();

    while (true) {
        // get a message from stdin
        int type;
        size_t len;
        scanf("%d", &type);
        scanf("%zu", &len);
        getchar();
        char* buf = new char[len * 4];
        for (size_t i = 0; i < len; i++) {
            scanf("%d", (int*)&buf[i]);
            getchar();
        }
        if (type == EXIT) {
            // if the message is an exit message, then exit
            log(EXIT, 0, NULL);
            break;
        } else if (type == RECV) {
            // if something has been sent over radio, call
            // Protocol::receive_message in a new thread
            PacketBuffer recv((uint8_t*)&buf[1], len - 1);
            int strength = -(buf[0] & 0xFF);
            _recv = recv;

            std::thread(Protocol::receive_message, uBit.radio.datagram.recv(),
                        on_message, strength)
                .detach();
        } else if (type == SEND_MSG) {
            // if a request to send a message has come, call
            // Protocol::send_message
            ManagedString msg((uint8_t*)&buf[1], len - 1);
            char recipient = buf[0];
            std::thread(Protocol::send_message, msg, recipient).detach();
        }
    }

    return 0;
}
