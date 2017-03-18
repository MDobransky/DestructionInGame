#include "EventReceiver.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

bool gg::MEventReceiver::OnEvent(const SEvent &event)
{
    if (event.EventType == EET_KEY_INPUT_EVENT)
    {
        keyState[event.KeyInput.Key] = event.KeyInput.PressedDown;
    }
    return false;
}

bool gg::MEventReceiver::keyDown(char keycode)
{
    return keyState[int(keycode)];
}
