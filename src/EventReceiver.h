#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include "Game.h"

namespace gg {

class Game;

class EventReceiver : public IEventReceiver
{
public:
    EventReceiver(Game* g) : game(g) {}
    ~EventReceiver() {}

    EventReceiver(const EventReceiver&) = delete;
    EventReceiver& operator=(const EventReceiver&) = delete;

    Game * game;

    virtual bool OnEvent(const SEvent &TEvent);
private:
    const btVector3 empty = btVector3(0,0,0);
    const float torque = 0.5f;
};

}

#endif
