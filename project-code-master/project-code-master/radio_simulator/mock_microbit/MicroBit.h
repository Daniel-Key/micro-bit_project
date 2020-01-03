#ifndef MICROBIT_H
#define MICROBIT_H

#include "Type.h"

#include <thread>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

// All code here is a mock version of MicroBit.h code, make so that the same
// Protocol.cpp that runs on the microbit can run on a computer

class PacketBuffer {
  private:
    char* buf = NULL;
    int len = 0;

  public:
    PacketBuffer(const PacketBuffer& pb) {
        len = pb.len;
        buf = new char[len];
        memcpy(buf, pb.buf, len);
    }
    PacketBuffer(uint8_t* str) {
        int len = strlen((char*)str) + 1;
        buf = new char[len];
        buf[len - 1] = '\0';
        strcpy(buf, (char*)str);
    }
    PacketBuffer(uint8_t* str, int len) {
        this->len = len;
        buf = new char[len];
        memcpy(buf, (char*)str, len);
    }
    ~PacketBuffer() {
        if (buf != NULL) {
            delete[] buf;
        }
    }
    PacketBuffer(int len) {
        this->len = len;
        buf = new char[len];
    }
    void operator=(const PacketBuffer& pb) {
        len = pb.len;
        if (buf != NULL) {
            delete[] buf;
        }
        buf = new char[len];
        memcpy(buf, pb.buf, len);
    }
    PacketBuffer() : PacketBuffer(0) {
    }
    int length() {
        return len;
    }
    char* getBytes() {
        return buf;
    }

    char* toCharArray() {
        return buf;
    }
};

typedef PacketBuffer ManagedString;

class MicroBitDisplay {
  public:
    void scroll(ManagedString str) {
        (void)str;
    }
    void clear() {
    }
};

extern PacketBuffer _recv;

class MicroBitRadioDatagram {
  public:
    void send(PacketBuffer buf) {
        log(SEND, buf.length(), buf.getBytes());
    }
    void send(uint8_t* buf, size_t len) {
        PacketBuffer pb(buf, len);
        send(pb);
    }
    PacketBuffer recv() {
        return _recv;
    }
};

class MicroBitRadio {
  public:
    MicroBitRadioDatagram datagram;
    void setGroup(int group) {
        (void)group;
    }
    void enable() {
    }
};

class MicroBit {
  public:
    MicroBitDisplay display;
    MicroBitRadio radio;

    void sleep(int millis) {
        usleep(millis * 1000);
    }
    uint64_t systemTime() {
        struct timeval tp;
        gettimeofday(&tp, NULL);
        return tp.tv_sec * 1000 + tp.tv_usec / 1000;
    }
};

void create_fiber(void (*func)());

#endif /* end of include guard: MICROBIT_H */
