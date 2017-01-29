#include "Game.h"
#include <iostream>
#include <string>


using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


gg::Game::Game()
{
}

gg::Game::~Game()
{
    delete events;
}

void gg::Game::Run()
{
    events = new EventReceiver(this);
    Done = false;

    loader = std::make_unique<Loader>(1600,900,false);

    //load game from file
    objects = std::vector<gg::Object>();
    irrDevice.reset(); //reset AFTER objects are deleted!!!
    std::tie(irrDevice, objects) = loader->load("media/levels/1");
    irrDevice->setEventReceiver(events);


    // Initialize irrlicht
    irrGUI = irrDevice->getGUIEnvironment();
    irrTimer = irrDevice->getTimer();
    irrScene = irrDevice->getSceneManager();
    irrDriver = irrDevice->getVideoDriver();

    irrDevice->getCursorControl()->setVisible(0);

    // Initialize bullet
    btDefaultCollisionConfiguration *CollisionConfiguration = new btDefaultCollisionConfiguration();
    btBroadphaseInterface *BroadPhase = new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000));
    btCollisionDispatcher *Dispatcher = new btCollisionDispatcher(CollisionConfiguration);
    btSequentialImpulseConstraintSolver *Solver = new btSequentialImpulseConstraintSolver();
    World = new btDiscreteDynamicsWorld(Dispatcher, BroadPhase, Solver, CollisionConfiguration);

    DebugDraw debugDraw(irrDevice.get());
       debugDraw.setDebugMode(
             btIDebugDraw::DBG_DrawWireframe |
             btIDebugDraw::DBG_DrawAabb |
             btIDebugDraw::DBG_DrawContactPoints |
             //btIDebugDraw::DBG_DrawText |
             //btIDebugDraw::DBG_DrawConstraintLimits |
             btIDebugDraw::DBG_DrawConstraints //|
       );
       World->setDebugDrawer(&debugDraw);

       irr::video::SMaterial debugMat;
       debugMat.Lighting = false;
       const bool debug_draw_bullet = false;



    CreateStartScene();


    velocity = -50;

    // Main loop
    u32 TimeStamp = irrTimer->getTime(), DeltaTime = 0;
    while(!Done) {
        irrDevice->setWindowCaption(std::to_wstring(irrDriver->getFPS()).c_str());

        DeltaTime = irrTimer->getTime() - TimeStamp;
        TimeStamp = irrTimer->getTime();

        btShip->setLinearVelocity(btShip->getWorldTransform().getBasis() * (btVector3( 0.f, -9.8f, velocity )));

        Camera->setTarget(IShip->getPosition());

        UpdatePhysics(DeltaTime);

        irrDriver->beginScene(true, true, SColor(255, 20, 0, 0));

        irrScene->drawAll();

        if (debug_draw_bullet)
        {
            irrDriver->setMaterial(debugMat);
            irrDriver->setTransform(irr::video::ETS_WORLD, irr::core::IdentityMatrix);
            World->debugDrawWorld();
        }
        irrGUI->drawAll();
        irrDriver->endScene();
        irrDevice->run();
    }

    //ClearObjects();
    delete World;
    delete Solver;
    delete Dispatcher;
    delete BroadPhase;
    delete CollisionConfiguration;

    //irrDevice->drop();
}

void  gg::Game::CreateStartScene()
{
    // Create the initial scene
    irrScene->addLightSceneNode(0, core::vector3df(0, 7000, 0), SColorf(4, 4, 4, 1),10000);
    irrScene->setAmbientLight(video::SColorf(0.3,0.3,0.3,1));

    ClearObjects();

    //put objects created by loader into bullet world
    for(size_t i = 0; i < objects.size(); i++)
    {
        //ship
        if(i == 1)
        {
            btShip = objects[i].getRigid();
            IShip = static_cast<IMeshSceneNode *>(btShip->getUserPointer());
        }
        World->addRigidBody(objects[i].getRigid());
        Objects.push_back(objects[i].getRigid());
        objects[i].getRigid()->activate();
    }

    //camera
    Camera = irrScene->addCameraSceneNode();
    btVector3 trans = btShip->getWorldTransform().getBasis() * btVector3(0,0.3,+0.6);
    Camera->setPosition(vector3df(trans.getX(),trans.getY(),trans.getZ()));
    Camera->setTarget(IShip->getPosition());
    Camera->setParent(IShip);
    Camera->bindTargetAndRotation(1);

    //rendered distance
    Camera->setFarValue(100000);
}

void  gg::Game::Shoot()
{
    ///bullet size
    scene::IMeshSceneNode *Node = irrScene->addSphereSceneNode(10.0f);
    Node->setMaterialFlag(video::EMF_LIGHTING, 1);
    Node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
    Node->setMaterialTexture(0, irrDriver->getTexture("media/ice0.jpg"));

    // Set the initial position
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(btShip->getCenterOfMassPosition() + btShip->getWorldTransform().getBasis() * btVector3(0,-3,0));

    // Give it a default MotionState
    btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

    // Create the shape/hitbox
    btCollisionShape *Shape = new btSphereShape(10.0f);

    // Add mass
    btVector3 LocalInertia;
    Shape->calculateLocalInertia(1.0f, LocalInertia);

    // Create the rigid body object
    btRigidBody *bullet = new btRigidBody(1.0f, MotionState, Shape, LocalInertia);

    //shoot it with speed
    bullet->applyImpulse(btShip->getWorldTransform().getBasis() * btVector3(0,0,-1000), btShip->getCenterOfMassPosition());

    // Store a pointer to the irrlicht node so we can update it later
    bullet->setUserPointer((void *)(Node));

    // Add it to the world
    World->addRigidBody(bullet);
    Objects.push_back(bullet);

}

//following functions are copied from http://www.irrlicht3d.org/wiki/index.php?n=Main.GettingStartedWithBullet

// Runs the physics simulation.
// - TDeltaTime tells the simulation how much time has passed since the last frame so the simulation can run independently of the frame rate.
void  gg::Game::UpdatePhysics(u32 TDeltaTime)
{

    World->stepSimulation(TDeltaTime * 0.001f, 60);

    // Relay the object's orientation to irrlicht
    for(list<btRigidBody *>::Iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator)
    {
        UpdateRender(*Iterator);
    }
    Camera->setTarget(IShip->getPosition());
}

// Passes bullet's orientation to irrlicht
void  gg::Game::UpdateRender(btRigidBody *TObject)
{
    ISceneNode *Node = static_cast<ISceneNode *>(TObject->getUserPointer());

    // Set position
    btVector3 Point = TObject->getCenterOfMassPosition();
    Node->setPosition(vector3df((f32)Point[0], (f32)Point[1], (f32)Point[2]));

    // Set rotation
    vector3df Euler;
    const btQuaternion& TQuat = TObject->getOrientation();
    quaternion q(TQuat.getX(), TQuat.getY(), TQuat.getZ(), TQuat.getW());
    q.toEuler(Euler);
    Euler *= RADTODEG;
    Node->setRotation(Euler);
}

// Removes all objects from the world
void  gg::Game::ClearObjects()
{
    for(list<btRigidBody *>::Iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator)
    {
        btRigidBody *Object = *Iterator;

        // Delete irrlicht node
        ISceneNode *Node = static_cast<ISceneNode *>(Object->getUserPointer());
        Node->remove();

        // Remove the object from the world
        World->removeRigidBody(Object);

        // Free memory
        delete Object->getMotionState();
        delete Object->getCollisionShape();
        delete Object;
    }
    Objects.clear();
}
