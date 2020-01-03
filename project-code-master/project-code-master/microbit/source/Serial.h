#ifndef SERIAL_H
#define SERIAL_H

#include "MicroBit.h"
#include "data-structures/Queue.h"

struct serial_msg {
    int type;
    ManagedString msg;
};

enum SerialMessageType {
    SERIAL_DEBUG_PRINT,     // send a message to log to the console
    SERIAL_LOG_MESSAGE,     // send a message to log on the server
    SERIAL_LOG_RADIO,       // log the bits of the message to the console
    SERIAL_REQUEST_WEATHER, // request the weather
    SERIAL_LOGIN,           // check the username and password
    SERIAL_SEND_TWEET,      //  Send a tweet
    SERIAL_REQUEST_TWEET,   //  Request a tweet
    SERIAL_PING,            // send a message to test if connected to serial
    SERIAL_MESH_FORWARD, // send a message encoded in the serial protocol to the
                         // server
    SERIAL_DEBUG_PRINT_SERVER // send a debug message that requires login
};

// print to the serial_comms.computer_serial program - like printf
#define DEBUG_PRINTF(fmt, ...)                                                 \
    {                                                                          \
        char buf[100];                                                         \
        int n = snprintf(buf, 100, fmt, ##__VA_ARGS__);                        \
        serial::send(SERIAL_DEBUG_PRINT, buf, n);                              \
    }
#define DEBUG_PRINTF_SERVER(fmt, ...)                                          \
    {                                                                          \
        char buf[100];                                                         \
        snprintf(buf, 100, fmt, ##__VA_ARGS__);                                \
        if (serial::server_request("oas", 1254, SERIAL_DEBUG_PRINT_SERVER,     \
                                   buf)) {                                     \
            serial::receive();                                                 \
        }                                                                      \
    }

// allows sending and receiving messages over USB
namespace serial {
    // initiate
    void init();
    // send somthing on over USB
    void send(int message_type, ManagedString message);
    void send(int message_type, char* message, size_t len);
    // receive 1 item over USB, ignoring type
    ManagedString receive();
    // receive 1 item over USB storing type
    ManagedString receive(int* message_type);
    // the serial event handler
    void on_serial(MicroBitEvent e);
    // send a message to the server, including login information
    bool server_request(ManagedString username, int pattern, int message_type,
                        ManagedString msg);
    // send a message to the server using stored login information
    bool server_request(int message_type, ManagedString msg);
    // check if logged in
    bool logged_in();
} // namespace serial

#endif /* end of include guard: SERIAL_H */
