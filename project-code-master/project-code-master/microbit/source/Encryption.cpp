#include "Encryption.h"

#include "MicroBit.h"

extern MicroBit uBit;

/*
    Encryption influenced by:
    http://www.trytoprogram.com/cpp-examples/cplusplus-program-encrypt-decrypt-string/

    Modular exponentiation based on pseudocode from:
    https://en.wikipedia.org/wiki/Modular_exponentiation
*/

uint64_t mod_exp(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    while (exp > 0) {
        if ((exp & 1) == 1) {
            res = (res * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    return res;
}

char* encryption::encrypt(char* text, int length, uint64_t public_key,
                          uint64_t n, int* resultLength) {
    uint32_t* cypherText = new uint32_t[(length - 1) / 3 + 1];

    // general idea
    // for (int i = 0; i < length; i++) {
    //     cypherText[i] = (text[i] ^ n) % PUBLIC_KEY;
    // }

    uint64_t pt; // BIG BYTES FOR THE MODULAR EXPONENTIATION
    int i = 0;
    int l = 0;

    while (i < length) {
        if (length - i == 1) {
            pt = text[i] << 16 | 0 | 0;
        } else if (length - i == 2) {
            pt = text[i] << 16 | text[i + 1] << 8 | 0;
        } else {
            pt = text[i] << 16 | text[i + 1] << 8 | text[i + 2];
        }

        // serial::send(SERIAL_LOG_RADIO, (char*)&pt, 8);

        cypherText[l] = mod_exp(pt, public_key, n);
        i += 3;
        l++;
    }

    *resultLength = 4 * ((length - 1) / 3 + 1);
    return (char*)cypherText;
}

// REVERSE IDEA OF ENCRYPT
char* encryption::decrypt(char* encryptedText, int length, int* resultLength) {
    uint32_t* enc = (uint32_t*)encryptedText;
    uint8_t* plainText = new uint8_t[length / 4 * 3];
    *resultLength = length / 4 * 3;

    // general idea
    // for (int i = 0; i < length; i++) {
    //     plainText[i] = (encryptedText[i] ^ PRIVATE_KEY) % n;
    // }

    int i = 0;
    int l = 0;

    while (l < *resultLength) {
        // serial::send(SERIAL_LOG_RADIO, (char*)&enc[i], 4);

        uint64_t k = mod_exp(enc[i++], PRIVATE_KEY, RSA_N);

        // serial::send(SERIAL_LOG_RADIO, (char*)&k, 8);

        plainText[l + 2] = k & 0xFF;
        plainText[l + 1] = (k >> 8) & 0xFF;
        plainText[l] = (k >> 16) & 0xFF;

        l += 3;
    }

    return (char*)plainText;
}
