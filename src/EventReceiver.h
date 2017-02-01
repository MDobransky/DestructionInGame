#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include "Game.h"

namespace gg {

class MGame;

class MEventReceiver : public IEventReceiver
{
public:
    MEventReceiver(MGame* g) : m_game(g) {}
    ~MEventReceiver() {}

    MEventReceiver(const MEventReceiver&) = delete;
    MEventReceiver& operator=(const MEventReceiver&) = delete;

    virtual bool OnEvent(const SEvent &TEvent);
private:
    const btVector3 m_empty = btVector3(0,0,0);
    const float m_torque = 0.5f;
    bool m_left = true;
    MGame * m_game;
};

}

#endif
