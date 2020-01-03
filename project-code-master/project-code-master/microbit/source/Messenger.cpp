#include "Messenger.h"

extern MicroBit uBit;
extern Keyboard keyboard;

Messenger::Messenger(InteractiveComponent* parent)
    : Selector(parent), sendIcon("0,0,255,0,0\n"
                                 "0,255,255,255,0\n"
                                 "255,0,255,0,255\n"
                                 "0,0,255,0,0\n"
                                 "0,0,255,0,0\n"),

      receiveIcon("0,0,255,0,0\n"
                  "0,0,255,0,0\n"
                  "255,0,255,0,255\n"
                  "0,255,255,255,0\n"
                  "0,0,255,0,0\n"),
      radio_receiver(this) {

    num_entered = false;

    registerApp(&sendIcon, &keyboard);
    registerApp(&receiveIcon, &radio_receiver);
}

void Messenger::controlGiven() {
    uBit.sleep(100);
    // set keyboard to return to this component upon user input completion
    keyboard.parent = this;
    show();
}

void Messenger::controlReturned() {
    if (appNum == 0) {
        // if sending a message
        if (num_entered) {
            // if a message recipient number has been entered
            num_entered = false;

            ManagedString message = keyboard.getMessage();

            keyboard.freeBuffer();
            // display S for "sending"
            uBit.display.printChar('S');
            // get recpient ID
            char recipient = (char)(num_entry->getNum());
            delete num_entry;

            // send message
            PacketBuffer msg_buffer((uint8_t*)message.toCharArray(),
                                    message.length());
            bool success = Protocol::send_message(msg_buffer, recipient);
            // display Y or N for "yes" or "no" depending on success
            uBit.display.printChar(success ? 'Y' : 'N');
            uBit.sleep(1000);

            uBit.display.clear();
            if (success) {
                uBit.sleep(100);
                // display message to confirm what the user has typed
                uBit.display.scroll(message);
            }
            show();
        } else if (keyboard.msgLength() > 0) {
            // if a message has been typed, but no recipient number yet

            // setup number keyboard in order to enter recpient number
            num_entered = true;
            num_entry = new NumberKeyboard(this);
            // go to number keyboard
            switchToComponent(num_entry);
        } else {
            show();
        }
    } else {
        // if returing from to receiving a messsage
        show();
    }
}
