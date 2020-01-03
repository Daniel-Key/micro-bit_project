#ifndef NUMBER_ENTRY_H
#define NUMBER_ENTRY_H

#include "MicroBit.h"
#include "Selector.h"

#include <string.h>

// the UI component that allows the user to enter a number
class NumberKeyboard : public InteractiveComponent {
  private:
    // the number being entered
    int num;
    // determines if a number has been entered yet
    bool valid;

    // allowing selection
    int appNum = 0;
    const size_t length = 10;

  public:
    NumberKeyboard(InteractiveComponent* parent);
    // returns the number that the user has entered
    int getNum();

    // see InteractiveComponent.h for overriden method docs
    void accept() override;
    void back() override;

    void on_button_a() override;
    void on_button_b() override;

    void controlGiven() override;

    void show() override;
};

#endif /* end of include guard: NUMBER_ENTRY_H */
