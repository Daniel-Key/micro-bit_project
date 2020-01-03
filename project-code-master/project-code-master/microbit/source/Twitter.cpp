//  Taken from Messenger.cpp
#include "Twitter.h"

extern MicroBit uBit;
extern Keyboard keyboard;

Twitter::Twitter(InteractiveComponent* parent)
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
      twitter_receiver(this) {

    registerApp(&sendIcon, &keyboard);
    registerApp(&receiveIcon, &twitter_receiver);
}

//  Gets called when Selector passes control down to Twitter
void Twitter::controlGiven() {
    keyboard.parent = this;
    show();
}

//  Gets called when Keyboard or TwitterReceiver are finished
void Twitter::controlReturned() {
    //  If sending a Tweet
    if (appNum == 0) {
        //  Get the body of the Tweet
        ManagedString message = keyboard.getMessage();

        keyboard.freeBuffer();

        uBit.display.printChar('S');

        //  Send the Tweet
        bool sent = serial::server_request(SERIAL_SEND_TWEET, message);
        // display 'Y' or 'N' to report success
        if (sent) {
            uBit.display.printChar('Y');
            serial::receive();
        } else {
            uBit.display.printChar('N');
        }
        uBit.sleep(1000);

        uBit.display.scroll(message);

        //  Go back to the menu.
        show();
    }
    //  If receiving a Tweet via TwitterReceiver
    else {
        //  TwitterReceiver displays the tweet, so all we need to do is go back
        //  to menu.
        show();
    }
}
