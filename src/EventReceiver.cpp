#include "EventReceiver.h"

bool gg::MEventReceiver::OnEvent(const SEvent &TEvent)
{

    if(TEvent.EventType == EET_KEY_INPUT_EVENT && TEvent.KeyInput.PressedDown)
    {
        btVector3 torqueVec(0,0,0);

        switch(TEvent.KeyInput.Key)
        {
            case KEY_ESCAPE:
                m_game->m_Done = true;
            break;
            case KEY_KEY_C:
                m_game->CreateStartScene();
            break;
            case KEY_KEY_W:
            {
                torqueVec = btVector3(-m_torque,0,0);
            }
            break;
            case KEY_KEY_S:
            {
                torqueVec = btVector3(m_torque,0,0);
            }
            break;
            case KEY_KEY_A:
            {
                torqueVec = btVector3(0,-m_torque/5,0);
            }
            break;
            case KEY_KEY_D:
            {
                torqueVec = btVector3(0,m_torque/5,0);
            }
            break;
            case KEY_KEY_Q:
            {
                torqueVec = btVector3(0,0,-m_torque);
            }
            break;
            case KEY_KEY_E:
            {
                torqueVec = btVector3(0,0,m_torque);
            }
            break;
            case KEY_LSHIFT:
            {
                m_game->m_velocity -= 20;
                if(m_game->m_velocity < -200) m_game->m_velocity = -200;
            }
            break;
            case KEY_LCONTROL:
            {
                m_game->m_velocity += 20;
                if(m_game->m_velocity > 0) m_game->m_velocity = 0;
            }
            break;
            case KEY_SPACE:
            {
                m_left = !m_left;
                m_game->Shoot(m_left);
            }
            break;
            default:
                return false;
            break;
        }
        //apply the effect of WASDQE
        if(torqueVec != m_empty)
        {
            m_game->m_btShip->setDamping(0,0.9);
            m_game->m_btShip->setAngularVelocity(m_game->m_btShip->getWorldTransform().getBasis() * torqueVec + m_game->m_btShip->getAngularVelocity());
        }
        return true;
    }
    return false;
}
