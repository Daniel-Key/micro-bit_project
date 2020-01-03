#ifndef SELECTOR_H_
#define SELECTOR_H_

#include "InteractiveComponent.h"

#include "MicroBit.h"

using namespace std;

// something that can be displayed: a char or image
union char_image_union {
    char c;
    MicroBitImage* image;
};
enum DisplayableType { DisplayableChar, DisplayableImage };
typedef struct {
    DisplayableType type;
    union char_image_union content;
} Displayable;

// a component that works like a menu, allowing the user to select a UI
// component to navigate to
class Selector : public InteractiveComponent {
  protected:
    int appNum = 0;
    size_t length = 0;
    InteractiveComponent** apps;
    Displayable* icons;

  public:
    Selector(InteractiveComponent* parent);
    virtual ~Selector();

    // see InteractiveComponent.h for overriden method docs

    void accept() override;

    void on_button_a() override;
    void on_button_b() override;
    void show() override;

    // register a component in the menu
    void registerApp(Displayable icon, InteractiveComponent* app);
    void registerApp(char icon, InteractiveComponent* app);
    void registerApp(MicroBitImage* icon, InteractiveComponent* app);
};

#endif /* end of include guard: MAIN_MENU_H_ */
