#include "EventReceiver.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

bool gg::MEventReceiver::OnEvent(const SEvent &event)
{
    if(event.EventType == EET_KEY_INPUT_EVENT)
    {
        for(int i = 0; i < irr::KEY_KEY_CODES_COUNT; i++)
        {
            oldState[i] = keyState[i];
        }
        keyState[event.KeyInput.Key] = event.KeyInput.PressedDown;
    }
    else if (event.GUIEvent.EventType == EGET_ELEMENT_CLOSED)
    {
        keyState[irr::KEY_ESCAPE] = event.KeyInput.PressedDown;
    }
    return false;
}

bool gg::MEventReceiver::keyDown(char keycode)
{
    return keyState[int(keycode)];
}

bool gg::MEventReceiver::keyPressed(char keycode)
{
    return oldState[int(keycode)] == 0 && keyState[int(keycode)];
}
