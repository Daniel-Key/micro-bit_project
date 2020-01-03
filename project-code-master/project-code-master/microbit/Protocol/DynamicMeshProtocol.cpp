#include "DynamicMeshProtocol.h"
#include "Protocol.h"
#include "config/ProtocolConfig.h"

extern MicroBit uBit;

Addr addr;
mb_id_t network_seed = 0xFFFF;

char neighbours = 0;

// get the nodes to which there is a line in a hexagonal tiling
AddrPos* adjacent(AddrPos node) {
    AddrPos* ret = new AddrPos[3];

    ret[0].x = node.x + 1;
    ret[0].y = node.y;

    ret[1].x = node.x - 1;
    ret[1].y = node.y;

    int dir = ((node.x + node.y) % 2 == 0) ? 1 : -1;

    ret[2].x = node.x;
    ret[2].y = node.y + dir;

    return ret;
}

void send_req() {
    PacketBuffer buf(1);
    buf.getBytes()[0] = REQ;

    uBit.radio.datagram.send(buf);
}

void send_ack() {
    PacketBuffer buf(5);
    buf.getBytes()[0] = ACK;
    buf.getBytes()[1] = addr.addr_pos.x;
    buf.getBytes()[2] = addr.addr_pos.y;
    buf.getBytes()[3] = addr.network_seed >> 8;
    buf.getBytes()[4] = addr.network_seed & 0xFF;
    uBit.radio.datagram.send(buf);
}

void swap(AddrStrength* a, AddrPos* b_pos, int* strength) {
    *strength ^= a->strength;
    a->strength ^= *strength;
    *strength ^= a->strength;

    a->addr_pos.x ^= b_pos->x;
    b_pos->x ^= a->addr_pos.x;
    a->addr_pos.x ^= b_pos->x;

    a->addr_pos.y ^= b_pos->y;
    b_pos->y ^= a->addr_pos.y;
    a->addr_pos.y ^= b_pos->y;
}

AddrStrength* nearby = NULL;

inline void record_nearby(PacketBuffer& message_bytes, int signal_strength) {
    if (nearby == NULL) {
        return;
    }
    char* bytes = message_bytes.getBytes();

    AddrPos addr_pos = {.x = bytes[1], .y = bytes[2]};

    mb_id_t possible_seed = *(mb_id_t*)&bytes[3];

    if (possible_seed < network_seed) {
        network_seed = possible_seed;
    }

    bool insert = true;
    for (size_t i = 0; i < NEARBY_ARRAY_SIZE; i++) {
        if (nearby[i].addr_pos.x != addr_pos.x ||
            nearby[i].addr_pos.y != addr_pos.y) {
            continue;
        }
        insert = false;

        if (signal_strength == nearby[i].strength) {
            break;
        }

        if (signal_strength < nearby[i].strength) {
            break;
        }
        // if greater strength, reassign array item and move it up into the
        // new corrpossible_seedect position
        nearby[i].strength = signal_strength;

        for (int j = i - 1; j >= 0; j--) {
            if (nearby[j + 1].strength > nearby[j].strength) {
                swap(&nearby[j + 1], &nearby[j].addr_pos, &nearby[j].strength);
            } else {
                break;
            }
        }

        break;
    }
    if (!insert) {
        return;
    }
    for (size_t i = 0; i < NEARBY_ARRAY_SIZE; i++) {
        if (signal_strength > nearby[i].strength) {
            swap(&nearby[i], &addr_pos, &signal_strength);
        }
    }
}
bool exists_var;
bool exists(AddrPos node, AddrPos next_hop) {
    if (nearby) {
        // for each nearby node
        for (size_t i = 0; i < NEARBY_ARRAY_SIZE; i++) {
            // if the strength==-256, there are no more nearby nodes
            if (nearby[i].strength == -256) {
                break;
            }
            // if this node is in nearby, then it sent an announcement so it
            // exists
            if (nearby[i].addr_pos.x == node.x &&
                nearby[i].addr_pos.y == node.y) {
                return true;
            }
        }
    }
    Packet packet(0);
    PacketHeader* header = packet.getHeader();
    header->type = EXISTS_REQ;
    header->winding = 0;

    header->recipient.ID = 0;
    header->recipient.network_seed = network_seed;
    header->recipient.addr_pos = node;

    header->sender.ID = 0;
    header->sender.network_seed = network_seed;
    header->sender.addr_pos.x = 0;
    header->sender.addr_pos.y = 0;

    header->last_hop = {.x = 0, .y = 0};
    header->next_hop = next_hop;

    packet.send(uBit);

    exists_var = false;
    for (size_t i = 0; i < 10 && !exists_var; i++) {
        uBit.sleep(30);
    }

    return exists_var;
}

void heartbeat() {
    while (true) {
        // === Reconfigure ===

        // request nearby microbits to announce
        nearby = new AddrStrength[NEARBY_ARRAY_SIZE]();
        for (size_t i = 0; i < NEARBY_ARRAY_SIZE; i++) {
            nearby[i].strength = -256; // initialise array with values below min
        }
        send_req();

        // wait for replies
        uBit.sleep(500);

        // find the best spot

        // for: each nearby microbit, see if an adjacent
        // spot is available
        for (size_t i = 0; i < NEARBY_ARRAY_SIZE; i++) {
            // if strength == -256, the end of the received messages has been
            // reached
            if (nearby[i].strength == -256) {
                break;
            }
            // check if the spot has been claimed already
            AddrPos* adj = adjacent(nearby[i].addr_pos);
            for (size_t i = 0; i < 3; i++) {
                bool available = !exists(adj[i], nearby[i].addr_pos);
                // TODO: if available, check each adjacent spot to adj[i]:
                // if you can see it or it doesn't exist, claim the spot
            }
            delete[] adj;
        }
        if (nearby[0].strength == -256) {
            claim_address(0, 0, MICROBIT_ID);
        }

        // reclaim memory
        delete[] nearby;
        nearby = NULL;
        uBit.sleep(5000);
    }
}

void Protocol::init() {
    create_fiber(heartbeat);

    addr.ID = MICROBIT_ID;
    addr.addr_pos = {.x = 0, .y = 0};
}

bool Protocol::send_message(ManagedString plaintextString, char recipient) {
    (void)plaintextString;
    (void)recipient;
    return false;
}

void Protocol::receive_message(void (*callback)(Message), int signal_strength) {
    (void)signal_strength;
    PacketBuffer message_bytes = uBit.radio.datagram.recv();

    switch (message_bytes.getBytes()[0]) {
    case REQ:
        send_ack();
        break;
    case ACK:
        record_nearby(message_bytes, signal_strength);
        break;
    default:
        Packet packet(message_bytes);
        PacketHeader* header = packet.getHeader();
        if (header->next_hop != addr.addr_pos) {
            break;
        }
        if (header->last_hop == addr.addr_pos) {
            break;
        }
        if (header->recipient.ID == MICROBIT_ID) {
            // if the message is addressed to you
            receive_message_for_you(packet, callback);
        } else {
            // if you are the next hop, but it isn't addressed to you

            bool forward = true;
            if (header->type == EXISTS_REQ) {
                AddrPos* adj = adjacent(nearby[i].addr_pos);
                for (size_t i = 0; i < 3; i++) {
                    if (header->recipient == adj[i]) {
                        Packet reply(1);
                        PacketHeader* reply_h = reply.getHeader();
                        reply_h->type = EXISTS_ACK;
                        reply_h->winding = 0;
                        reply_h->sender = header->recipient;
                        reply_h->recipient = header->sender;
                        if (neighbour_exists(i)) {
                        }
                    }
                }
                delete[] adj;
            }
            if (forward) {
                forward_message(packet);
            }
        }
    }
}

void receive_message_for_you(Packet& packet, void (*callback)(Message)) {
    PacketHeader* header = packet.getHeader();

    switch (header->type) {
    case DATA: {
        // TODO
    } break;
    case EXISTS_REQ: {
        header->type = EXISTS_ACK;
        header->winding = 0;
        Addr temp_addr = header->sender;
        header->sender = header->recipient;
        header->recipient = temp_addr;
        AddrPos temp_pos = header->last_hop;
        header->last_hop = header->next_hop;
        header->next_hop = temp_pos;

        uBit.radio.datagram.send((uint8_t*)packet.getBytes(), packet.size());
    } break;
    case EXISTS_ACK: {
        // TODO
    } break;
    case REGISTER: {
        // TODO
    } break;
    default: {} // error
    }
}

void forward_message(const Packet& packet) {
}

void claim_address(char x, char y, mb_id_t seed) {
    addr.addr_pos.x = x;
    addr.addr_pos.y = y;
    addr.ID = MICROBIT_ID;
    addr.network_seed = seed;

    LOGF("claimed (%d,%d),%d\n", x, y, seed);

    // TODO: register address
}

Message::Message(PacketBuffer encodedPacketArg) {
    bytes = new char[encodedPacketArg.length() + 1];
    memcpy(bytes, encodedPacketArg.getBytes(), encodedPacketArg.length());
    bytes[encodedPacketArg.length()] = '\0';

    length = encodedPacketArg.length() - 4; // subtract header length

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
size_t Message::size() {
    return length;
}

char* Message::getContent() {
    return (char*)&bytes[4];
}
int Message::getSenderID() {
    uint16_t sender = *(uint16_t*)&bytes[0];
    return sender;
}
int Message::getRecipientID() {
    uint16_t recipient = *(uint16_t*)&bytes[2];
    return recipient;
}
char Message::getMessageType() {
    return 0;
}
