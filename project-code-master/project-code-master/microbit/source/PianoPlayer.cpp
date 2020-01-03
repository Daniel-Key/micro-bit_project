#include "PianoPlayer.h"

extern MicroBit uBit;

PianoPlayer::PianoPlayer(InteractiveComponent* parent) 
    : Selector(parent) {
}

void PianoPlayer::controlGiven() {
    int result = play();
    while (result == 0) {
        result = play();
    }
    toParent();
}

void PianoPlayer::controlReturned() {

}