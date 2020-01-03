#include "Piano.h"
#include "MicroBit.h"

extern MicroBit uBit;

// Based on https://raw.githubusercontent.com/KitronikLtd/micropython-microbit-kitronik-klef-piano/master/klef-piano.py
// Horrible hacks, just for proof of principle.
// Can be built by using as 'source/main.cpp' in the microbit-samples repository.

int CHIP_ADDRESS = 0x0D << 1;   // J: 13 -> 26 = 1a hexa
// int CHIP_ADDRESS = 0x11;
bool INITIALISED = 0;
                            // J:
const int KEY_K0 = 0x100;  // up -> 256
const int KEY_K1 = 0x200;  // C# -> 512
const int KEY_K2 = 0x400;  // Eb
const int KEY_K3 = 0x800;  // F#
const int KEY_K4 = 0x1000; // Ab
const int KEY_K5 = 0x2000; // Bb -> 8192
const int KEY_K6 = 0x4000; // B
const int KEY_K7 = 0x8000; // C2
const int KEY_K8 = 0x01;   // down
const int KEY_K9 = 0x02;   // C1
const int KEY_K10 = 0x04;  // D
const int KEY_K11 = 0x08;  // E
const int KEY_K12 = 0x10;  // F
const int KEY_K13 = 0x20;  // G -> 32
const int KEY_K14 = 0x40;  // A

int keySensitivity = 20;
int keyNoiseThreshold = 1;
int keyRegValue = 0x0000;

int c4 = 3823;
int d4 = 3412;
int e4 = 3039;
int f4 = 2865;
int g4 = 2551;
int a4 = 2272;
int b4 = 2028;
int c5 = 1912;
int e5 = 1517;

int chosen = 0;

int init() {
    char buff[1] = {0};     // holds chip ID
    char buff2[2] = {0};    // holds ID number of item to change and data of what to change it to
                            // ie. 0 = key, 1 = sensitivity
    char buff3[5] = {0};

    uBit.io.P1.setPull(PullUp);
    buff[0] = 0;
    uBit.i2c.write(CHIP_ADDRESS, buff, 1, false);
    uBit.i2c.read(CHIP_ADDRESS, buff, 1, false);

    while(buff[0] != 0x11) {    // = 17
        uBit.i2c.read(CHIP_ADDRESS, buff, 1, false);
        return 0;
    }

    // Change sensitivity (burst length) of keys 0-14 to keySensitivity 8 (defined above, default)
    // J: Key mappings 0 -> 54, ... , 14 -> 68
    for(int sensitivityReg = 54; sensitivityReg < 69; sensitivityReg++) {
        buff2[0] = sensitivityReg;
        buff2[1] = keySensitivity;
        uBit.i2c.write(CHIP_ADDRESS, buff2, 0); // assigns sensitivity
    }
    // Disable key 15 as it is not used
    buff2[0] = 69;      // 15 -> 69
    buff2[1] = 0;
    uBit.i2c.write(CHIP_ADDRESS, buff2, 2);

    // Set Burst Repetition to keyNoiseThreshold (default is 5);
    buff2[0] = 13;
    buff2[1] = keyNoiseThreshold;
    uBit.i2c.write(CHIP_ADDRESS, buff2, 2);

    // Configure Adjacent Key Suppression (AKS) Groups
    // AKS Group 1: ALL KEYS
    for (int aksReg = 22; aksReg < 37; aksReg++) {
        buff2[0] = aksReg;
        buff2[1] = 1;
        uBit.i2c.write(CHIP_ADDRESS, buff2, 2);
    }

    // Send calibration command
    buff2[0] = 10;
    buff2[1] = 1;
    uBit.i2c.write(CHIP_ADDRESS, buff2, 2);

    // Read all change status address (General Status addr = 2);
    buff[0] = 2;
    uBit.i2c.write(CHIP_ADDRESS, buff, 1);
    uBit.i2c.read(CHIP_ADDRESS, buff3, 5, 0);
    // Continue reading change status address until /change pin goes high

    while (uBit.io.P1.getDigitalValue()) {
        uBit.display.scroll("Reset");
        buff[0] = 2;
        uBit.i2c.write(CHIP_ADDRESS, buff, 1);
        uBit.i2c.read(CHIP_ADDRESS, buff3, 5, 0);
        INITIALISED = 1;
        init();
    }
    uBit.display.scroll("Piano");
    return 1;
}

/* ARRAY OF VECTORS, VECTOR OF TUPLES
    ARRAY OF THE KEYS ON THE PIANO
    VECTORS REPRESENT ALL NOTES TO BE PLAYED FOR A MELODY
    TUPLE = (NOTE, LENGTH) (int, int)
*/
std::vector<tuple<int, int>> melodies[13];

void setup() {
    KeyValuePair* setting = uBit.storage.get("sound");
    if (setting == NULL) {
        uBit.storage.put("sound", (uint8_t*) &chosen, sizeof(chosen));
    }
    else {
        memcpy(&chosen, setting->value, sizeof(int));
        delete setting;
    }

    // C1
    for (int i = 0 ; i < 3; i++) {
        melodies[0].push_back(make_tuple(3823, 500));
    }

    // D
    for (int i = 0 ; i < 3; i++) {
        melodies[1].push_back(make_tuple(1500, 500));
    }

    // E
        melodies[2].push_back(make_tuple(b4, 600));
        melodies[2].push_back(make_tuple(a4, 200));
        melodies[2].push_back(make_tuple(a4, 200));
        melodies[2].push_back(make_tuple(b4, 400));
        melodies[2].push_back(make_tuple(c5, 400));
        melodies[2].push_back(make_tuple(b4, 200));
        melodies[2].push_back(make_tuple(b4, 200));
        melodies[2].push_back(make_tuple(a4, 200));
        melodies[2].push_back(make_tuple(a4, 200));
        melodies[2].push_back(make_tuple(b4, 200));
        melodies[2].push_back(make_tuple(g4, 200));
        melodies[2].push_back(make_tuple(g4, 800));
        melodies[2].push_back(make_tuple(0, 200));
        melodies[2].push_back(make_tuple(e4, 200));
        melodies[2].push_back(make_tuple(f4, 200));
        melodies[2].push_back(make_tuple(g4, 400));
        melodies[2].push_back(make_tuple(a4, 1000));

    // F
        melodies[3].push_back(make_tuple(f4, 600));
        melodies[3].push_back(make_tuple(f4, 200));
        melodies[3].push_back(make_tuple(f4, 400));
        melodies[3].push_back(make_tuple(f4, 400));
        melodies[3].push_back(make_tuple(e4, 400));
        melodies[3].push_back(make_tuple(f4, 800));
        melodies[3].push_back(make_tuple(f4, 400));
        melodies[3].push_back(make_tuple(e4, 400));
        melodies[3].push_back(make_tuple(f4, 400));
        melodies[3].push_back(make_tuple(0, 400));
        melodies[3].push_back(make_tuple(g4, 400));
        melodies[3].push_back(make_tuple(a4, 600));
        melodies[3].push_back(make_tuple(g4, 200));
        melodies[3].push_back(make_tuple(g4, 400));

    // G
        melodies[4].push_back(make_tuple(e4, 400));
        // melodies[4].push_back(make_tuple(0, 10));
        melodies[4].push_back(make_tuple(e4, 400));
        // melodies[4].push_back(make_tuple(0, 10));
        melodies[4].push_back(make_tuple(e4, 800));
        // melodies[4].push_back(make_tuple(0, 10));
        melodies[4].push_back(make_tuple(e4, 400));
        // melodies[4].push_back(make_tuple(0, 10));
        melodies[4].push_back(make_tuple(e4, 400));
        // melodies[4].push_back(make_tuple(0, 10));
        melodies[4].push_back(make_tuple(e4, 800));
        // melodies[4].push_back(make_tuple(0, 10));
        melodies[4].push_back(make_tuple(e4, 400));
        melodies[4].push_back(make_tuple(g4, 400));
        melodies[4].push_back(make_tuple(c4, 400));
        melodies[4].push_back(make_tuple(d4, 400));
        melodies[4].push_back(make_tuple(e4, 800));


    // A
    for (int i = 0 ; i < 3; i++) {
        melodies[5].push_back(make_tuple(1500, 500));
    }

    // B
    for (int i = 0 ; i < 3; i++) {
        melodies[6].push_back(make_tuple(1500, 500));
        melodies[6].push_back(make_tuple(3823, 500));
    }

    // C2
    for (int i = 0 ; i < 3; i++) {
        melodies[7].push_back(make_tuple(3823, 500));
    }

    // C#
    for (int i = 0 ; i < 3; i++) {
        melodies[8].push_back(make_tuple(1500, 500));
    }

    // Eb
    melodies[9].push_back(make_tuple(1500, 500));
    melodies[9].push_back(make_tuple(3823, 500));

    // F#
    for (int i = 0 ; i < 7; i++) {
        melodies[10].push_back(make_tuple(1500, 500));
    }

    // Ab
    for (int i = 0 ; i < 5; i++) {
        melodies[11].push_back(make_tuple(3823, 500));
    }

    // Bb
    for (int i = 0 ; i < 3; i++) {
        melodies[12].push_back(make_tuple(4000, 500));
        melodies[12].push_back(make_tuple(5000, 500));
    }
}

void play_melody(int which) {
    uBit.io.P0.setAnalogValue(2);               // Volume
    for (std::vector<tuple<int, int>>::iterator iter = melodies[which].begin(); iter != melodies[which].end(); iter++) {
        uBit.io.P0.setAnalogPeriodUs(get<0>(*iter));
        uBit.sleep(get<1>(*iter));
        uBit.io.P0.setAnalogPeriodUs(0);
        uBit.sleep(100);
    }
    uBit.io.P0.setAnalogPeriodUs(0);
}

int get_note(int key) {
    uBit.io.P0.setAnalogValue(2);               // Volume
    switch(key) {
        case KEY_K0 : uBit.display.scroll("up");
                      return -1;
        case KEY_K1 : uBit.display.scroll("C#");
                      play_melody(8);
                      return 8;
        case KEY_K2 : uBit.display.scroll("Eb");
                      play_melody(9);
                      return 9;
        case KEY_K3 : uBit.display.scroll("F#");
                      play_melody(10);
                      return 10;
        case KEY_K4 : uBit.display.scroll("Ab");
                      play_melody(11);
                      return 11;
        case KEY_K5 : uBit.display.scroll("Bb");
                      play_melody(12);
                      return 12;
        case KEY_K6 : uBit.display.scroll("B");
                      play_melody(6);
                      return 6;
        case KEY_K7 : uBit.display.scroll("C");
                      play_melody(7);
                      return 7;
        case KEY_K8 : uBit.display.scroll("down");
                      return -1;
        case KEY_K9 : uBit.display.scroll("C");
                      play_melody(0);
                      return 0;
        case KEY_K10 : uBit.display.scroll("D");
                       play_melody(1);
                       return 1;
        case KEY_K11 : uBit.display.scroll("E");
                      play_melody(2);
                      return 2;
        case KEY_K12 : uBit.display.scroll("F");
                      play_melody(3);
                      return 3;
        case KEY_K13 : uBit.display.scroll("G");
                      play_melody(4);
                      return 4;
        case KEY_K14 : uBit.display.scroll("A");
                      play_melody(5);
                      return 5;
    }
    return -1;
}

int read_key_press(){
    char buff[1] = {0};
    char buff2[2] = {0};
    char buff3[5] = {0};

    buff[0] = 2;
    uBit.i2c.write(CHIP_ADDRESS, buff, 1, false);
    uBit.i2c.read(CHIP_ADDRESS, buff3, 5, false);

    // Address 3 is the address for keys 0-7 (this will then auto move onto Address 4 for keys 8-15, both reads stored in buff2)
    buff[0] = 3;
    uBit.i2c.write(CHIP_ADDRESS, buff, 1, false);
    uBit.i2c.read(CHIP_ADDRESS, buff2, 2, false);

    // keyRegValue is a 4 byte number which shows which keys are pressed
    int keyRegValue = (buff2[1] + (buff2[0] * 256));

    return keyRegValue;
}

// welcome sound
void welcome() {
    uBit.io.P0.setAnalogValue(2);               // Volume
                                                // This is frequency in C - 3823ish is a middle C
    uBit.io.P0.setAnalogPeriodUs(3823);         // the lower the number the higher the note
    uBit.sleep(500);
    uBit.io.P0.setAnalogPeriodUs(5000);
    uBit.sleep(500);
    uBit.io.P0.setAnalogPeriodUs(2500);
    uBit.sleep(500);
    uBit.io.P0.setAnalogPeriodUs(3000);
    uBit.sleep(500);
    uBit.io.P0.setAnalogPeriodUs(0);
    return;
}

void notification() {
    play_melody(chosen);
}

// Play a note on the keyboard
int play() {
    uBit.display.scroll("Choose a tune");
    int note = 0;

    while((note = read_key_press()) == 0 && note != KEY_K8) {
        uBit.sleep(100);
        uBit.display.scroll(":)");
    }
    if (note == KEY_K8) {
        return 1;
    }
    int result = get_note(note);
    uBit.display.scroll("This one?");

    while((note = read_key_press()) != KEY_K0 && note != KEY_K8){
        uBit.sleep(100);
        uBit.display.scroll("?");
    }
    if (note == KEY_K0) {
        chosen = result;
        uBit.storage.put("sound", (uint8_t*) &chosen, sizeof(chosen));
        return 1;
    }
    else if (note == KEY_K8) {
        return 0;
    }
    return 0;
}

// Calls initialisation
int piano_main() {
    // Initialise the micro:bit runtime.
    if (init() == 0) {
        return 0;
    }
    return 1;
}
