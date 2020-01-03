#ifndef DISPLAY_ID_H
#define DISPLAY_ID_H

#include "InteractiveComponent.h"
#include "MicroBit.h"
#include "config/ProtocolConfig.h"

// displays the ID of the microbit
class DisplayID : public InteractiveComponent {
  public:
    DisplayID(InteractiveComponent* parent);

    void show() override;
};

#endif /* end of include guard: DISPLAY_ID_H */
