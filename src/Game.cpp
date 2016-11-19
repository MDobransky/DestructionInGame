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
        std::tie(irrDevice, objects) = loader->load("media/levels/1");
        irrDevice->setEventReceiver(events);



    // Initialize irrlicht
        irrGUI = irrDevice->getGUIEnvironment();
        irrTimer = irrDevice->getTimer();
        irrScene = irrDevice->getSceneManager();
        irrDriver = irrDevice->getVideoDriver();

        irrDevice->getCursorControl()->setVisible(0);



        IMesh* mesh = irrScene->getMesh("media/vue_ready_shasta.obj");

        irrScene->getMeshManipulator()->scale(mesh,core::vector3df(10,10,10));
        IMeshSceneNode* Node = irrScene->addMeshSceneNode( mesh );
        Node->setMaterialType(EMT_SOLID);
        Node->setMaterialFlag(EMF_LIGHTING, 1);
        Node->setMaterialTexture(0, irrDriver->getTexture("media/terrain-texture.jpg"));


        // Initialize bullet
        btDefaultCollisionConfiguration *CollisionConfiguration = new btDefaultCollisionConfiguration();
        btBroadphaseInterface *BroadPhase = new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000));
        btCollisionDispatcher *Dispatcher = new btCollisionDispatcher(CollisionConfiguration);
        btSequentialImpulseConstraintSolver *Solver = new btSequentialImpulseConstraintSolver();
        World = new btDiscreteDynamicsWorld(Dispatcher, BroadPhase, Solver, CollisionConfiguration);

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
            irrGUI->drawAll();
            irrDriver->endScene();
            irrDevice->run();
        }

        ClearObjects();
        delete World;
        delete Solver;
        delete Dispatcher;
        delete BroadPhase;
        delete CollisionConfiguration;

  //      irrDevice->drop();
}

void  gg::Game::CreateStartScene()
{
    // Create the initial scene
    irrScene->addLightSceneNode(0, core::vector3df(0, 7000, 0), SColorf(4, 4, 4, 1),10000);
    irrScene->setAmbientLight(video::SColorf(0.3,0.3,0.3,1));

    ClearObjects();
    //ground
    //CreateBox(btVector3(0.0f, -500.0f, -0.5f), vector3df(20000.0f, 0.2f, 20000.0f), 0.0f);
    //position, size, mass
    CreateShip(btVector3(0.0f, 2000.0f, 0.0f));

    //camera
    Camera = irrScene->addCameraSceneNode();
    btVector3 trans = btShip->getWorldTransform().getBasis() * btVector3(0,3,+10);
    Camera->setPosition(vector3df(trans.getX(),trans.getY(),trans.getZ()));
    Camera->setTarget(IShip->getPosition());
    Camera->setParent(IShip);
    Camera->bindTargetAndRotation(1);

    //rendered distance
    Camera->setFarValue(100000);
/*
    //terrain
    scene::ITerrainSceneNode* terrain = irrScene->addTerrainSceneNode(
          "./media/terrain-heightmap.bmp",0,-1,core::vector3df(0,-20,0));

    terrain->setScale(core::vector3df(80, 8.8f, 80));
    terrain->setMaterialFlag(video::EMF_LIGHTING, false);

    terrain->setMaterialTexture(0, irrDriver->getTexture("./media/terrain-texture.jpg"));
    terrain->setMaterialTexture(1, irrDriver->getTexture("./media/detailmap3.jpg"));

    terrain->setMaterialType(video::EMT_DETAIL_MAP);
    terrain->scaleTexture(1.0f, 20.0f);
*/
}



void  gg::Game::CreateShip(const btVector3 &TPosition)
{
    const core::vector3df Size = core::vector3df(9.0f, 3.0f, 11.0f);
    const btScalar Mass = 5.5f;

    IMesh* mesh = irrScene->getMesh("media/xwing.3ds");

    irrScene->getMeshManipulator()->scale(mesh,core::vector3df(2.6,2.6,2.6));
    IMeshSceneNode* Node = irrScene->addMeshSceneNode( mesh );
    Node->setMaterialType(EMT_SOLID);
    Node->setMaterialFlag(EMF_LIGHTING, 1);
    Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);

    // Set the initial position of the object
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(TPosition);

    btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

    // Create the shape
    //TODO \/ for better hitboxes
    //https://studiofreya.com/game-maker/bullet-physics/from-irrlicht-mesh-to-bullet-physics-mesh/
    btVector3 HalfExtents(Size.X*0.5, Size.Y*0.5 , Size.Z*0.5);
    btCollisionShape *Shape = new btBoxShape(HalfExtents);

    // Add mass
    btVector3 LocalInertia;
    Shape->calculateLocalInertia(Mass, LocalInertia);

    // Create the rigid body object
    btRigidBody *RigidBody = new btRigidBody(Mass, MotionState, Shape, LocalInertia);

    // Store a pointer to the irrlicht node so we can update it later
    RigidBody->setUserPointer((void *)(Node));

    // Add it to the world
    World->addRigidBody(RigidBody);
    Objects.push_back(RigidBody);

    btShip = RigidBody;
    IShip = Node;
}



// Create a box rigid body
void  gg::Game::CreateBox(const btVector3 &TPosition, const core::vector3df &TScale, btScalar TMass) {

    // Create an Irrlicht cube
    scene::ISceneNode *Node = irrScene->addCubeSceneNode(1.0f);
    Node->setScale(TScale);
    Node->setMaterialFlag(video::EMF_LIGHTING, 1);
    Node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
    Node->setMaterialTexture(0, irrDriver->getTexture("media/rust0.jpg"));

    // Set the initial position of the object
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(TPosition);

    // Give it a default MotionState
    btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

    // Create the shape
    btVector3 HalfExtents(TScale.X*0.5, TScale.Y*0.5 , TScale.Z*0.5);
    btCollisionShape *Shape = new btBoxShape(HalfExtents);

    // Add mass
    btVector3 LocalInertia;
    Shape->calculateLocalInertia(TMass, LocalInertia);

    // Create the rigid body object
    btRigidBody *RigidBody = new btRigidBody(TMass, MotionState, Shape, LocalInertia);

    // Store a pointer to the irrlicht node so we can update it later
    RigidBody->setUserPointer((void *)(Node));

    // Add it to the world
    World->addRigidBody(RigidBody);
    Objects.push_back(RigidBody);
}

void  gg::Game::Shoot()
{
    ///bullet size
    scene::IMeshSceneNode *Node = irrScene->addSphereSceneNode(0.5f,32);
    Node->setMaterialFlag(video::EMF_LIGHTING, 1);
    Node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
    Node->setMaterialTexture(0, irrDriver->getTexture("media/ice0.jpg"));

    // Set the initial position
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(btShip->getCenterOfMassPosition() + btShip->getWorldTransform().getBasis() * btVector3(0,-3,-6));

    // Give it a default MotionState
    btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

    // Create the shape/hitbox
    btCollisionShape *Shape = new btSphereShape(0.2f);

    // Add mass
    btVector3 LocalInertia;
    Shape->calculateLocalInertia(1.0f, LocalInertia);

    // Create the rigid body object
    btRigidBody *bullet = new btRigidBody(1.0f, MotionState, Shape, LocalInertia);

    //shoot it with speed
    bullet->applyImpulse(btShip->getWorldTransform().getBasis() * btVector3(0,0,-1000),btShip->getCenterOfMassPosition());

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
