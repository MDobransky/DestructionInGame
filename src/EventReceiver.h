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
};

}

#endif
