#include "InteractiveComponent.h"

extern MicroBit uBit;
extern InteractiveComponent* curr_component;

// See the header file for a description of what each method is for.

InteractiveComponent::InteractiveComponent(InteractiveComponent* parent) {
    // keep a reference to the parent in the app
    this->parent = parent;
}

void InteractiveComponent::back() {
    // by default, tilting the microbit back will go back to the parent
    toParent();
}

void InteractiveComponent::switchToComponent(InteractiveComponent* child) {
    // in order to switch to a component:

    // set it as a child
    this->child = child;
    // set the current component, allowing events to call the methods of the new
    // component
    curr_component = child;
    // call the entry method - this will call the `show` method by default
    child->controlGiven();
}

void InteractiveComponent::toParent() {
    if (parent != NULL) {
        parent->child = NULL;
        // reset the current component so events call the correct app's methods
        curr_component = parent;
        // call the entry method - this will call the `show` method by default
        parent->controlReturned();
    }
}

void InteractiveComponent::controlGiven() {
    show();
}

void InteractiveComponent::controlReturned() {
    show();
}

void InteractiveComponent::flash() {
    uBit.display.clear();
    uBit.sleep(100);

    show();
    uBit.sleep(100);

    uBit.display.clear();
    uBit.sleep(100);

    show();
}
