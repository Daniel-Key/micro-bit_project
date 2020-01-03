#ifndef MAIN_MENU_H_
#define MAIN_MENU_H_

#include "DisplayID.h"
#include "Messenger.h"
#include "PatternLock.h"
#include "Piano.h"
#include "PianoPlayer.h"
#include "Selector.h"
#include "Twitter.h"
#include "Weather.h"

#include "MicroBit.h"

// the main menu of the microbit UI
class MainMenu : public Selector {
  private:
    // the child apps
    Messenger messenger;
    DisplayID displayID;
    Weather weather;
    Twitter twitter;

  public:
    PianoPlayer player;
    // has no parent
    MainMenu();

    // see InteractiveComponent.h for descriptions of overriden methods
    void controlReturned() override;
};

#endif /* end of include guard: MAIN_MENU_H_ */
