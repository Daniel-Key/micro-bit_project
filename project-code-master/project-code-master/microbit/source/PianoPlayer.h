#ifndef PIANOPLAYER_H_
#define PIANOPLAYER_H_

#include "MicroBit.h"
#include "Selector.h"
#include "Piano.h"

class PianoPlayer : public Selector {
    private:


    public:
        PianoPlayer(InteractiveComponent* parent);
        void controlGiven() override;
        void controlReturned() override;
};

#endif