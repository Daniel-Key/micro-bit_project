#ifndef MESSENGER_H_
#define MESSENGER_H_

#include "Keyboard.h"
#include "NumberKeyboard.h"
#include "Protocol.h"
#include "Selector.h"

#include "MicroBit.h"
#include "Radio.h"

// the UI component that allow the user to send and receive messages
class Messenger : public Selector {
  private:
    // icons
    MicroBitImage sendIcon;
    MicroBitImage receiveIcon;
    // child components
    RadioReceiver radio_receiver;
    NumberKeyboard* num_entry; //  For entering the ID of the m:b to send to

    // used to determine if the child that is returning control is the number
    // entry or the keyboard
    bool num_entered;

  public:
    Messenger(InteractiveComponent* parent);

    // see InteractiveComponent.h for overidden method docs
    void controlGiven() override;
    void controlReturned() override;
};

#endif /* end of include guard: MESSENGER_H_ */
