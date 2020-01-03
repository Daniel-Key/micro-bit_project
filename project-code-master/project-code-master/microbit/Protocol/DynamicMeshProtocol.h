#ifndef DYNAMIC_MESH_PROTOCOL_H
#define DYNAMIC_MESH_PROTOCOL_H

#include "Protocol.h"

#define NEARBY_ARRAY_SIZE 4

#include "MicroBit.h"

#include <stdlib.h>

struct AddrPos {
    char x;
    char y;
    bool operator==(const AddrPos& rhs) {
        return x == rhs.x && y == rhs.y;
    }
    bool operator!=(const AddrPos& rhs) {
        return x != rhs.x || y != rhs.y;
    }
};

typedef uint16_t mb_id_t;

struct Addr {
    mb_id_t ID;
    mb_id_t network_seed;
    AddrPos addr_pos;

    // bool operator==(const Addr& lhs, const Addr& rhs) {
    //     return lhs.ID == rhs.ID && lhs.network_seed == rhs.network_seed &&
    //            lhs.addr_pos == rhs.addr_pos;
    // }
};

typedef struct {
    AddrPos addr_pos;
    int strength;
} AddrStrength;

enum MessageType { DATA, REQ, ACK, EXISTS_REQ, EXISTS_ACK, REGISTER };

struct PacketHeader {
    char type;
    char winding;
    Addr sender;
    Addr recipient;
    AddrPos last_hop;
    AddrPos next_hop;
};

class Packet {
  private:
    char* buf;
    size_t length;

  public:
    Packet(PacketBuffer _buf) {
        buf = new char[_buf.length() + 1];
        memcpy(buf, _buf.getBytes(), _buf.length());
        buf[_buf.length()] = '\0';

        length = _buf.length() - sizeof(PacketHeader);
    }
    Packet(size_t payload_size) {
        length = payload_size;
        buf = new char[payload_size + sizeof(PacketHeader)]();
    }
    ~Packet() {
        delete[] buf;
    }

    PacketHeader* getHeader() {
        return (PacketHeader*)buf;
    }

    char* getContent() {
        return buf + sizeof(PacketHeader);
    }

    size_t size() {
        return length;
    }

    char* getBytes() {
        return buf;
    }

    void send(MicroBit& uBit) {
        uBit.radio.datagram.send((uint8_t*)buf, length + sizeof(PacketHeader));
    }
};

// gets the nodes to which there is a line in a hexagonal tiling
AddrPos* adjacent(AddrPos node);

// requests that in-range microbits announce themselves
inline void send_req();

// sends an acknowlegement to announce yourself
inline void send_ack();

// swaps the values of two addr items
void swap(AddrStrength* a, AddrPos* b_pos, int* strength);

// inserts into the nearby array
inline void record_nearby(PacketBuffer& message_bytes, int signal_strength);

// checks if a node exists by sending an EXISTS message
bool exists(AddrPos node, AddrPos next_hop);

// a loop that sends heartbeat messages to reconfigure
void heartbeat();

// recieve a message addressed to this microbit
inline void receive_message_for_you(Packet& packet, void (*callback)(Message));

// forward a message on to another microbit
inline void forward_message(const Packet& packet);

void claim_address(char x, char y, mb_id_t seed);

#endif /* end of include guard: DYNAMIC_MESH_PROTOCOL_H */
