#ifndef TWITTER_H_
#define TWITTER_H_

#include "Keyboard.h"
#include "NumberKeyboard.h"
#include "Selector.h"

#include "MicroBit.h"
#include "Radio.h"
#include "Serial.h"

// A selector component for sending and receiving tweets
class Twitter : public Selector {
  private:
    //  Icons
    MicroBitImage sendIcon;
    MicroBitImage receiveIcon;

    //  Child components
    TwitterReceiver twitter_receiver;

  public:
    Twitter(InteractiveComponent* parent);

    void controlGiven() override;
    void controlReturned() override;
};

#endif
