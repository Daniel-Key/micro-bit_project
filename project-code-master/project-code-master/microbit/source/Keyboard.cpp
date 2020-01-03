#include "Keyboard.h"

extern MicroBit uBit;

Keyboard keyboard(NULL);

// Add a letter to the internal array
void push_back(char*& message, size_t& len, char chr) {
    // new array 1 longer than the old to consever memory
    char* new_message = new char[len + 1];
    // copy old into new
    memcpy(new_message, message, len);
    // add new letter
    new_message[len] = chr;
    len++;
    // delete old array
    if (message != NULL) {
        delete[] message;
    }
    // assign old to new
    message = new_message;
}

Keyboard::Keyboard(InteractiveComponent* parent)
    : InteractiveComponent(parent) {
}

void Keyboard::back() {
    push_back(message, msg_len, '\0');

    appNum = 0;
    toParent();
}

void Keyboard::accept() {
    // when the microbit is tilted away from the user, type a letter
    switch ('A' + appNum) {
    case 'Z' + 1:
        // if at the end of the keyboard, type a space
        push_back(message, msg_len, ' ');
        break;
    default:
        // if not at the end of the keyboard, type the currently displayed
        // letter
        push_back(message, msg_len, 'A' + appNum);
    }
    flash();
}

ManagedString Keyboard::getMessage() {
    // wrap the current keyboard text as a ManagedString
    return ManagedString(message);
}

void Keyboard::on_button_a() {
    if (uBit.accelerometer.getX() < -300) {
        appNum += length - 5;
    } else {
        appNum += length - 1;
    }
    appNum %= length;
    show();
}
void Keyboard::on_button_b() {
    if (uBit.accelerometer.getX() > 300) {
        appNum += 5;
    } else {
        appNum++;
    }
    appNum %= length;
    show();
}
void Keyboard::show() {
    switch ('A' + appNum) {
    case 'Z' + 1:
        uBit.display.printChar('_');
        break;
    default:
        uBit.display.printChar('A' + appNum);
    }
}

void Keyboard::freeBuffer() {
    if (message != NULL) {
        delete[] message;
        message = NULL;
    }

    msg_len = 0;
}

size_t Keyboard::msgLength() {
    return msg_len;
}
