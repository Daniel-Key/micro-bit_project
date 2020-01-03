#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "MicroBit.h"

#include "Selector.h"

// A keyboard component which can be used to enter text
class Keyboard : public InteractiveComponent {
  private:
    // the currently selected letter index
    int appNum = 0;
    // the number of lettrs available (A to Z + space)
    const size_t length = 27;
    // the text as a char array
    char* message = NULL;
    // the current length of the text
    size_t msg_len = 0;

  public:
    Keyboard(InteractiveComponent* parent);

    // see InteractiveComponent.h, which these methods override from
    void back() override;
    void accept() override;

    void on_button_a() override;
    void on_button_b() override;

    void show() override;

    // returns the message that the user typed
    ManagedString getMessage();

    // free the data in the buffer
    void freeBuffer();
    // get the current text length
    size_t msgLength();
};

#endif /* end of include guard: KEYBOARD_H_ */
