#ifndef PATTERN_LOCK_H
#define PATTERN_LOCK_H

#include "InteractiveComponent.h"
#include "Keyboard.h"
#include "MicroBit.h"
#include "Serial.h"

#include <string.h>

class PatternLock : public InteractiveComponent {
  private:
    MicroBitImage im; // the image to display on the screen
    ManagedString username;

    int x = 0;       // the x position of the cursor
    int y = 0;       // the y position of the cursor
    int prev = 0;    // the previous node visited in the grid
    int visited = 0; // a bitset of all nodes visited

    // the current combination that has been entered thus far
    int curr_combination = 0;
    bool compare_to_local = false;

  public:
    // constructor: takes the next component to go to after unlocking as an
    // argument
    PatternLock(InteractiveComponent* next, bool force_relog);
    ~PatternLock(){};

    // for overriden method docs, see InteractiveComponent.h

    void controlGiven() override;
    void controlReturned() override;
    void show() override;

    // run the pattern unlock code
    void pattern();
    // pattern update and login logic
    inline bool update_pattern();
    // if A or B button pressed, call this
    inline void button_pressed();
    // draws a line between 2 grid nodes
    inline void draw_line(int from, int to);
    // draws a grid node
    void draw_point(int cx, int cy, int brightness);
};

#endif /* end of include guard: PATTERN_LOCK_H */
