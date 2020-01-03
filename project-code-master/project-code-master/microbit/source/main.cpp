#include "MicroBit.h"
#include <string.h>

#include "MainMenu.h"
#include "PatternLock.h"
#include "Selector.h"

#include "Piano.h"
#include "Protocol.h"
#include "Radio.h"
#include "Serial.h"
#include "config/Config.h"

MicroBit uBit;

// used to specify whether the piano is plugged in
int piano = 0;
// the main menu component
MainMenu main_menu;
// the pattern lock screen component
PatternLock* pattern_lock = NULL;

// the currently selected component
InteractiveComponent* curr_component;

void on_button_a(MicroBitEvent e) {
    (void)e; // silence unused param warning
    curr_component->on_button_a();
}

void on_button_b(MicroBitEvent e) {
    (void)e; // silence unused param warning
    curr_component->on_button_b();
}

void main_loop() {
    // detect tilt forward and tilt back, and send as events
    while (true) {
        int pitch = uBit.accelerometer.getPitch();
        if (piano == 1) {
            uBit.sleep(100);
            int note;
            if ((note = read_key_press()) == 0x100) {
                curr_component->accept();
                while (note == 0x100) {
                    uBit.sleep(100);
                    note = read_key_press();
                }
            } else if (note == 0x01) {
                curr_component->back();
                while (note == 0x01) {
                    uBit.sleep(100);
                    note = read_key_press();
                }
            }
            // TOTEST: OR IT COULD BE THE FACT THE M:B IS CONSTANTLY AT A 90
            // ANGLE WHEN SITTING IN THE PIANO! MAKING IT INSTANTLY REACT AS A
            // BACK COMMAND
        } else if (pitch > 90) {
            curr_component->back();
            while (pitch > 90) {
                uBit.sleep(100);
                pitch = uBit.accelerometer.getPitch();
            }
        } else if (pitch < -20) {
            curr_component->accept();
            while (pitch < -20) {
                uBit.sleep(100);
                pitch = uBit.accelerometer.getPitch();
            }
        }
        uBit.sleep(100);
    }
}

// program entry point
int main() {
    // initialise
    uBit.init();

    radio::init();

    uBit.display.setDisplayMode(DISPLAY_MODE_GREYSCALE);

    serial::init();

    piano = piano_main();
    if (piano == 1) {
        welcome();
        setup();
        main_menu.registerApp('P', &main_menu.player);
    }
    // if pattern lock enabled in config/Config.h
    if (PATTERN_LOCK_ENABLED) {
        // go to the pattern lock component, do not force relog
        pattern_lock = new PatternLock(&main_menu, false);
        curr_component = pattern_lock;
        // pass control to the pattern lock
        pattern_lock->controlGiven();
    } else {
        // go to the main menu
        curr_component = &main_menu;
        main_menu.controlGiven();
    }
    // listen for button presses
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK,
                           on_button_a);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK,
                           on_button_b);

    // start reading accelerometer data/piano buttons and sending tilt events
    create_fiber(main_loop);

    release_fiber();
}
