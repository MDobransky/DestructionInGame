#ifndef GAME_H
#define GAME_H

#include "Object.h"
#include "EventReceiver.h"


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

namespace gg {

class Game
{
    friend class EventReceiver;

private:
    void CreateStartScene();
    void CreateShip(const btVector3 &TPosition);
    void CreateBox(const btVector3 &TPosition, const core::vector3df &TScale, btScalar TMass);
    void UpdatePhysics(u32 TDeltaTime);
    void UpdateRender(btRigidBody *TObject);
    void ClearObjects();
    void Shoot();

    IrrlichtDevice *irrDevice;
    btDiscreteDynamicsWorld *World;

    bool Done;
    IVideoDriver *irrDriver;
    ISceneManager *irrScene;
    IGUIEnvironment *irrGUI;
    ITimer *irrTimer;
    list<btRigidBody *> Objects;
    IMeshSceneNode* IShip;
    ICameraSceneNode* Camera;
    btRigidBody* btShip;
    float velocity;



public:

    void Run();
    ~Game();
    Game();
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

};

}

#endif // GAME_H
