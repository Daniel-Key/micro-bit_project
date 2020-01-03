#ifndef INTERACTIVE_COMPONENT_H_
#define INTERACTIVE_COMPONENT_H_

#include <stdlib.h>

#include "MicroBit.h"

// A component of the UI
class InteractiveComponent {
  private:
    InteractiveComponent* child = NULL;

  public:
    InteractiveComponent* parent;
    // Contructor: parent is the component to return to when the user tilts back
    InteractiveComponent(InteractiveComponent* parent);
    virtual ~InteractiveComponent(){};

    // on tilt forward
    virtual void accept() {
    }

    // on tilt back
    virtual void back();

    // display this component
    virtual void show() {
    }

    // when button a (left button) is pressed
    virtual void on_button_a() {
    }
    // when button b (right button) is pressed
    virtual void on_button_b() {
    }

    // when switching to this component from a parent
    virtual void controlGiven();

    // when switching to this component from a child
    virtual void controlReturned();

    // gives control to another component
    void switchToComponent(InteractiveComponent* child);

    // goes back to the component that launched this component
    void toParent();

    // flashes the symbol from `show` on and off
    void flash();
};

#endif /* end of include guard: INTERACTIVE_COMPONENT */
