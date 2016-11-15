#ifndef EVENTRECEIVER_H
#define EVENTRECEIVER_H

#include "Game.h"

//singleton
class EventReceiver : public IEventReceiver
{
public:
    EventReceiver() {};
    ~EventReceiver() {};

    EventReceiver(const EventReceiver&) = delete;
    EventReceiver& operator=(const EventReceiver&) = delete;

    virtual bool OnEvent(const SEvent &TEvent)
    {

        if(TEvent.EventType == EET_KEY_INPUT_EVENT && !TEvent.KeyInput.PressedDown)
        {
            int torque = 500;
            btVector3 torqueVec(0,0,0);

            switch(TEvent.KeyInput.Key)
            {
                case KEY_ESCAPE:
                    Game::Instance().Done = true;
                break;
                case KEY_KEY_C:
                    Game::Instance().CreateStartScene();
                break;
                case KEY_KEY_W:
                {
                    torqueVec = btVector3(-torque,0,0);
                }
                break;
                case KEY_KEY_S:
                {
                    torqueVec = btVector3(torque,0,0);
                }
                break;
                case KEY_KEY_A:
                {
                    torqueVec = btVector3(0,-torque/2,0);
                }
                break;
                case KEY_KEY_D:
                {
                    torqueVec = btVector3(0,torque/2,0);
                }
                break;
                case KEY_KEY_Q:
                {
                    torqueVec = btVector3(0,0,-torque);
                }
                break;
                case KEY_KEY_E:
                {
                    torqueVec = btVector3(0,0,torque);
                }
                break;
                case KEY_LSHIFT:
                {
                    Game::Instance().velocity -= 10;
                    if(Game::Instance().velocity < -100) Game::Instance().velocity = -100;
                }
                break;
                case KEY_LCONTROL:
                {
                    Game::Instance().velocity += 10;
                    if(Game::Instance().velocity > 0) Game::Instance().velocity = 0;
                }
                break;
                case KEY_SPACE:
                {
                    Game::Instance().Shoot();
                }
                break;
                default:
                    return false;
                break;
            }
            //apply the effect of WASDQE
            Game::Instance().btShip->applyTorque(Game::Instance().btShip->getWorldTransform().getBasis() * torqueVec);

            return true;
        }
        return false;
    }
};

#endif
