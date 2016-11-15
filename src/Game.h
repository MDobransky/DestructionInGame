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

    ~Game();
    Game();

public:

    void Run();

    static Game& Instance()
    {
        static Game instance;
        return instance;
    }

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

};


#endif // GAME_H
