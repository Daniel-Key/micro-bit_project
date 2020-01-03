#include "NumberKeyboard.h"

extern MicroBit uBit;

NumberKeyboard::NumberKeyboard(InteractiveComponent* parent)
    : InteractiveComponent(parent) {
}

void NumberKeyboard::accept() {
    // add a new digit
    num *= 10;
    num += appNum;
    valid = true;
    flash();
}

void NumberKeyboard::back() {
    if (valid) {
        toParent();
    }
}

int NumberKeyboard::getNum() {
    return num;
}

void NumberKeyboard::controlGiven() {
    // when entered, reset
    valid = false;
    num = 0;
    appNum = 0;
    show();
}

void NumberKeyboard::on_button_a() {
    // modulus (%) doesn't always work as intended for negative numbers, so add
    // length before subtracting 1
    appNum += length - 1;
    appNum %= length;
    show();
}

void NumberKeyboard::on_button_b() {
    appNum++;
    appNum %= length;
    show();
}

void NumberKeyboard::show() {
    uBit.display.printChar('0' + appNum);
}
