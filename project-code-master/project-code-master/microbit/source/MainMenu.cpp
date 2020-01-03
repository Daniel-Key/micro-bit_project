#include "MainMenu.h"

extern int piano;
extern PatternLock* pattern_lock;

MainMenu::MainMenu()
    : Selector(NULL), messenger(this), displayID(this), weather(this),
      twitter(this), player(this) {
    // register menu items
    registerApp('M', &messenger);
    registerApp('I', &displayID);
    registerApp('W', &weather);
    registerApp('T', &twitter);
}

void MainMenu::controlReturned() {
    // free up memory used in pattern lock
    if (pattern_lock != NULL) {
        delete pattern_lock;
        pattern_lock = NULL;
    }
    show();
}
