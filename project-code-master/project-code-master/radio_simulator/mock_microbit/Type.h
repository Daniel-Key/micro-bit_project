#ifndef TYPE_H
#define TYPE_H

#include <stdio.h>

// like printf, but sends a print-type message over stdout - parent process can
// interpret this how it likes - useful for debugging
#define LOGF(fmt, ...)                                                         \
    {                                                                          \
        char buf[100];                                                         \
        int n = snprintf(buf, 100, fmt, ##__VA_ARGS__);                        \
        log(PRINT, n, buf);                                                    \
    }

// all message types
enum Type {
    EXIT,     // exit
    RECV,     // got somthing over radio
    SEND,     // sending over radio
    RECV_MSG, // got a message
    SEND_MSG, // send a message
    PRINT     // print to console
};

// sends bytes stdout + message type in an easy to parse format
void log(Type type, int len, const char* msg);

#endif /* end of include guard: TYPE_H */
