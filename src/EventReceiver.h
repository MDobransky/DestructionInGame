#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include "Game.h"

namespace gg {

class MEventReceiver : public irr::IEventReceiver
{
public:
    MEventReceiver() {}
    ~MEventReceiver() {}

    MEventReceiver(const MEventReceiver&) = delete;
    MEventReceiver& operator=(const MEventReceiver&) = delete;

    virtual bool OnEvent(const irr::SEvent &TEvent);
    bool keyDown(char keycode);
private:
    bool keyState[irr::KEY_KEY_CODES_COUNT] = {0,irr::KEY_KEY_CODES_COUNT};
};

}

#endif
