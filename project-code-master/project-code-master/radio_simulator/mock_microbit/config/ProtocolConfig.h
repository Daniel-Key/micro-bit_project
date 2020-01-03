#ifndef PROTOCOL_H
#define PROTOCOL_H

class Constants {
  public:
    static int _MICROBIT_ID;
    static uint64_t _PRIVATE_KEY;
    static uint64_t _PUBLIC_KEY;
    static uint64_t _RSA_N;
};

#define MICROBIT_ID Constants::_MICROBIT_ID
#define PRIVATE_KEY Constants::_PRIVATE_KEY
#define PUBLIC_KEY Constants::_PUBLIC_KEY
#define RSA_N Constants::_RSA_N

#endif /* end of include guard: PROTOCOL_H */
