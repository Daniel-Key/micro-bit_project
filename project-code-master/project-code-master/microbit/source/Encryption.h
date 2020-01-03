#ifndef ENCRYPTION_H_
#define ENCRYPTION_H_

#include "MicroBit.h"
#include "config/ProtocolConfig.h"

namespace encryption {
    char* encrypt(char* text, int length, uint64_t public_key, uint64_t n, int* resultLength);
    char* decrypt(char* encryptedText, int length, int* resultLength);
}

#endif