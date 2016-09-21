#ifndef GAME_H
#define GAME_H

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <cstdlib>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class game
{
    friend class EventReceiverClass;
public:
    game();
    void run();
private:
    void CreateStartScene();
    void CreateShip(const btVector3 &TPosition);
    void CreateBox(const btVector3 &TPosition, const core::vector3df &TScale, btScalar TMass);
    void UpdatePhysics(u32 TDeltaTime);
    void UpdateRender(btRigidBody *TObject);
    void ClearObjects();
    void shoot();


    bool Done;
    btDiscreteDynamicsWorld *World;
    IrrlichtDevice *irrDevice;
    IVideoDriver *irrDriver;
    ISceneManager *irrScene;
    IGUIEnvironment *irrGUI;
    ITimer *irrTimer;
    list<btRigidBody *> Objects;
    IMeshSceneNode* IShip;
    ICameraSceneNode* Camera;
    btRigidBody* btShip;
    float velocity;

    class EventReceiverClass : public IEventReceiver
    {

    public:
        game * g;
        virtual bool OnEvent(const SEvent &TEvent) {

            if(TEvent.EventType == EET_KEY_INPUT_EVENT && !TEvent.KeyInput.PressedDown) {
                int torque = 500;
                switch(TEvent.KeyInput.Key) {
                    case KEY_ESCAPE:
                        g->Done = true;
                    break;
                    case KEY_KEY_C:
                        g->CreateStartScene();
                    break;
                    case KEY_KEY_W:
                    {
                        btVector3 worldTorque = g->btShip->getWorldTransform().getBasis() * btVector3(-torque,0,0);
                        g->btShip->activate();
                        g->btShip->applyTorque(worldTorque);
                    }
                    break;
                    case KEY_KEY_S:
                    {
                        btVector3 worldTorque = g->btShip->getWorldTransform().getBasis() * btVector3(torque,0,0);
                        g->btShip->activate();
                        g->btShip->applyTorque(worldTorque);
                    }
                    break;
                    case KEY_KEY_A:
                    {
                        btVector3 worldTorque = g->btShip->getWorldTransform().getBasis() * btVector3(0,-torque/2,0);
                        g->btShip->activate();
                        g->btShip->applyTorque(worldTorque);
                    }
                    break;
                    case KEY_KEY_D:
                    {
                        btVector3 worldTorque = g->btShip->getWorldTransform().getBasis() * btVector3(0,torque/2,0);
                        g->btShip->activate();
                        g->btShip->applyTorque(worldTorque);
                    }
                    break;
                    case KEY_KEY_Q:
                    {
                        btVector3 worldTorque = g->btShip->getWorldTransform().getBasis() * btVector3(0,0,-torque);
                        g->btShip->activate();
                        g->btShip->applyTorque(worldTorque);
                    }
                    break;
                    case KEY_KEY_E:
                    {
                        btVector3 worldTorque = g->btShip->getWorldTransform().getBasis() * btVector3(0,0,torque);
                        g->btShip->activate();
                        g->btShip->applyTorque(worldTorque);
                    }
                    break;
                    case KEY_LSHIFT:
                    {
                        g->velocity -= 10;
                        if(g->velocity < -100) g->velocity = -100;
                    }
                    break;
                    case KEY_LCONTROL:
                    {
                        g->velocity += 10;
                        if(g->velocity > 0) g->velocity = 0;
                    }
                    break;
                    case KEY_SPACE:
                    {
                        g->shoot();
                    }
                    break;
                    default:
                        return false;
                    break;
                }

                return true;
            }
            return false;
        }
    };

};



#endif // GAME_H
