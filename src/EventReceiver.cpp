#include "EventReceiver.h"

bool gg::EventReceiver::OnEvent(const SEvent &TEvent)
{

    if(TEvent.EventType == EET_KEY_INPUT_EVENT && !TEvent.KeyInput.PressedDown)
    {
        int torque = 500;
        btVector3 torqueVec(0,0,0);

        switch(TEvent.KeyInput.Key)
        {
            case KEY_ESCAPE:
                game->Done = true;
            break;
            case KEY_KEY_C:
                game->CreateStartScene();
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
                game->velocity -= 10;
                if(game->velocity < -100) game->velocity = -100;
            }
            break;
            case KEY_LCONTROL:
            {
                game->velocity += 10;
                if(game->velocity > 0) game->velocity = 0;
            }
            break;
            case KEY_SPACE:
            {
                game->Shoot();
            }
            break;
            default:
                return false;
            break;
        }
        //apply the effect of WASDQE
        game->btShip->applyTorque(game->btShip->getWorldTransform().getBasis() * torqueVec);

        return true;
    }
    return false;
}
