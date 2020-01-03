#include "Selector.h"

extern MicroBit uBit;

Selector::Selector(InteractiveComponent* parent)
    : InteractiveComponent(parent) {
    appNum = 0;
    length = 0;

    apps = NULL;
    icons = NULL;
}

Selector::~Selector() {
    delete[] apps;
    delete[] icons;
}

void Selector::registerApp(Displayable icon, InteractiveComponent* app) {
    // increase array size and copy data
    InteractiveComponent** apps_new = new InteractiveComponent*[length + 1];
    Displayable* icons_new = new Displayable[length + 1];

    for (size_t i = 0; i < length; i++) {
        apps_new[i] = apps[i];
        icons_new[i] = icons[i];
    }
    apps_new[length] = app;
    icons_new[length] = icon;

    if (apps != NULL) {
        delete apps;
        delete icons;
    }

    apps = apps_new;
    icons = icons_new;

    length++;
}

void Selector::registerApp(char icon, InteractiveComponent* app) {
    Displayable d;
    d.type = DisplayableChar;
    d.content.c = icon;

    registerApp(d, app);
}

void Selector::registerApp(MicroBitImage* icon, InteractiveComponent* app) {
    Displayable d;
    d.type = DisplayableImage;
    d.content.image = icon;

    registerApp(d, app);
}

void Selector::accept() {
    // when tilting away, switch to the currently selected component
    switchToComponent(apps[appNum]);
}

void Selector::show() {
    // display the currently selected image or char
    Displayable d = icons[appNum];
    switch (d.type) {
    case DisplayableChar:
        uBit.display.printChar(d.content.c);
        break;
    case DisplayableImage:
        uBit.display.print(*d.content.image);
        break;
    }
}

void Selector::on_button_a() {
    // move left by subtracting 1 from appNum and modulus to wrap around

    // modulus (%) doesn't always work as intended for negative numbers, so make
    // sure they're positive by adding length
    appNum += length - 1;
    appNum %= length;
    show();
}
void Selector::on_button_b() {
    // move right by adding 1 to appNum and modulus to wrap around
    appNum++;
    appNum %= length;
    show();
}
