#include "Game.h"
#include "Material.h"
#include <iostream>
#include <string>


using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

gg::MGame::MGame()
{
}

gg::MGame::~MGame()
{
}

void gg::MGame::Run(bool debug)
{
    m_events = new MEventReceiver();
    m_Done = false;

    m_loader = std::make_unique<MLoader>(1600,900,false);

    //load game from file
    m_objects = std::vector<std::unique_ptr<gg::MObject>>();
    m_irrDevice.reset(); //reset AFTER objects are deleted!!!
    std::tie(m_irrDevice, m_objects, m_constraints) = m_loader->load("media/levels/1");
    m_irrDevice->setEventReceiver(m_events);


    // Initialize irrlicht
    m_irrGUI = m_irrDevice->getGUIEnvironment();
    m_irrTimer = m_irrDevice->getTimer();
    m_irrScene = m_irrDevice->getSceneManager();
    m_irrDriver = m_irrDevice->getVideoDriver();

    m_irrDevice->getCursorControl()->setVisible(0);

    // Initialize bullet
    btDefaultCollisionConfiguration *CollisionConfiguration = new btDefaultCollisionConfiguration();
    btBroadphaseInterface *BroadPhase = new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000));
    btCollisionDispatcher *Dispatcher = new btCollisionDispatcher(CollisionConfiguration);
    btSequentialImpulseConstraintSolver *Solver = new btSequentialImpulseConstraintSolver();
    m_btWorld = new btDiscreteDynamicsWorld(Dispatcher, BroadPhase, Solver, CollisionConfiguration);

    MDebugDraw debugDraw(m_irrDevice.get());
       debugDraw.setDebugMode(
             btIDebugDraw::DBG_DrawWireframe |
             btIDebugDraw::DBG_DrawAabb |
             btIDebugDraw::DBG_DrawContactPoints |
             //btIDebugDraw::DBG_DrawText |
             //btIDebugDraw::DBG_DrawConstraintLimits |
             btIDebugDraw::DBG_DrawConstraints //|
       );
       m_btWorld->setDebugDrawer(&debugDraw);

       irr::video::SMaterial debugMat;
       debugMat.Lighting = false;
       const bool debug_draw_bullet = debug;


    CreateStartScene();

    m_velocity = 0;
    IGUIStaticText* fpsTextElement = m_irrScene->getGUIEnvironment()->addStaticText(L"", rect<s32>(35, 35, 140, 50), false, false, 0);

    // Main loop
    u32 TimeStamp = m_irrTimer->getTime(), DeltaTime = 0;
    //u32 realStamp = m_irrTimer->getRealTime();
    while(!m_Done) {
        int fps = m_irrDriver->getFPS();
        stringw str = L"FPS: ";
        str += fps;
        fpsTextElement->setText(str.c_str());
        m_irrDevice->setWindowCaption(str.c_str());


        DeltaTime = m_irrTimer->getTime() - TimeStamp;
        TimeStamp = m_irrTimer->getTime();
        ApplyEvents();
        m_btShip->setLinearVelocity(m_btShip->getWorldTransform().getBasis() * (btVector3( 0.f, 0.0f, m_velocity ))); //GRAVITY HERE


        m_Camera->setTarget(m_IShip->getPosition());

        //u32 b =  m_irrTimer->getRealTime();
        UpdatePhysics(DeltaTime);
        //std::cout << "simulation time: " << m_irrTimer->getRealTime() - b << "\n";
        m_Camera->setTarget(m_IShip->getPosition());

        m_irrDriver->beginScene(true, true, SColor(255, 20, 0, 0));
        //b =  m_irrTimer->getRealTime();
        m_irrScene->drawAll();
       //std::cout << "render time: " << m_irrTimer->getRealTime() - b << "\n";
        if (debug_draw_bullet)
        {
            m_irrDriver->setMaterial(debugMat);
            m_irrDriver->setTransform(irr::video::ETS_WORLD, irr::core::IdentityMatrix);
            m_btWorld->debugDrawWorld();
        }
        m_irrGUI->drawAll();
        m_irrDriver->endScene();
        m_irrDevice->run();

        //std::cout << "frame time: " << m_irrTimer->getRealTime() - realStamp << "\n";
        //realStamp = m_irrTimer->getRealTime();
    }

    //ClearObjects();
    delete m_btWorld;
    delete Solver;
    delete Dispatcher;
    delete BroadPhase;
    delete CollisionConfiguration;
}

void  gg::MGame::CreateStartScene()
{
    // Create the initial scene
    m_irrScene->addLightSceneNode(0, core::vector3df(0, 7000, 0), SColorf(4, 4, 4, 1),10000);
    m_irrScene->setAmbientLight(video::SColorf(0.3,0.3,0.3,1));

    ClearAllObjects();

    //put objects created by loader into bullet world
    for(size_t i = 0; i < m_objects.size(); i++)
    {
        //ship
        if(i == 1)
        {
            m_btShip = m_objects[i]->getRigid();
            m_IShip = m_objects[i]->getNode();
        }
        m_btWorld->addRigidBody(m_objects[i]->getRigid());
        m_Objects.push_back(m_objects[i]->getRigid());
        m_objects[i]->getRigid()->activate();
    }
    for(size_t i = 0; i < m_constraints.size(); i++)
    {
        m_btWorld->addConstraint(m_constraints[i].get());
    }


    m_terrainTransform.setIdentity();
    m_objects[0]->getRigid()->getCollisionShape()->getAabb(m_terrainTransform,m_maxBoud,m_minBound);
    m_minBound.setY(m_minBound.getY()+51);

    //camera
    m_Camera = m_irrScene->addCameraSceneNode();
    btVector3 trans = m_btShip->getWorldTransform().getBasis() * btVector3(0,0.3,+0.6);
    m_Camera->setPosition(vector3df(trans.getX(),trans.getY(),trans.getZ()));
    m_Camera->setTarget(m_IShip->getPosition());
    m_Camera->setParent(m_IShip);
    m_Camera->bindTargetAndRotation(1);

    //rendered distance
    m_Camera->setFarValue(100000);
}

void  gg::MGame::ApplyEvents()
{
    const float torque = 0.2f;

    if(m_events->keyDown('W'))
    {
        m_btShip->setDamping(0,0.9);
        m_btShip->setAngularVelocity(m_btShip->getWorldTransform().getBasis() * btVector3(-torque,0,0) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('S'))
    {
        m_btShip->setDamping(0,0.9);
        m_btShip->setAngularVelocity(m_btShip->getWorldTransform().getBasis() * btVector3(torque,0,0) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('A'))
    {
        m_btShip->setDamping(0,0.9);
        m_btShip->setAngularVelocity(m_btShip->getWorldTransform().getBasis() * btVector3(0,-torque/2,0) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('D'))
    {
        m_btShip->setDamping(0,0.9);
        m_btShip->setAngularVelocity(m_btShip->getWorldTransform().getBasis() * btVector3(0,torque/2,0) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('Q'))
    {
        m_btShip->setDamping(0,0.9);
        m_btShip->setAngularVelocity(m_btShip->getWorldTransform().getBasis() * btVector3(0,0,-torque) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('E'))
    {
        m_btShip->setDamping(0,0.9);
        m_btShip->setAngularVelocity(m_btShip->getWorldTransform().getBasis() * btVector3(0,0,torque) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('Z'))
    {
        m_velocity -= 15;
        if(m_velocity < -200) m_velocity = -200;
    }

    if(m_events->keyDown('X'))
    {
        m_velocity += 15;
        if(m_velocity >= 0) m_velocity = -1;
    }

    if(m_events->keyDown(irr::KEY_SPACE))
    {
        if(m_irrTimer->getTime() - m_shot_time > 500)
        {
            m_shot_time = m_irrTimer->getTime();
            m_left = !m_left;
            Shoot(m_left);
        }
    }

    if(m_events->keyDown(irr::KEY_ESCAPE))
    {
        m_Done = true;
    }
}

void  gg::MGame::Shoot(bool left)
{

    ///bullet size
    scene::IMeshSceneNode *Node = m_irrScene->addSphereSceneNode(1.0f);
    Node->setMaterialFlag(video::EMF_LIGHTING, 1);
    Node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
    Node->setMaterialTexture(0, m_irrDriver->getTexture("media/ice0.jpg"));

    // Set the initial position
    btTransform Transform;
    Transform.setIdentity();

    btVector3 start(0,0,0);
    if(left)
            start = btVector3(3,-5,-5);
    else    start = btVector3(-3,-5,-5);
    Transform.setOrigin(m_btShip->getCenterOfMassPosition() + m_btShip->getWorldTransform().getBasis() * start);

    // Give it a default MotionState
    btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

    // Create the shape/hitbox
    btCollisionShape *Shape = new btSphereShape(1.0f);

    // Add mass
    btVector3 LocalInertia;
    Shape->calculateLocalInertia(1.0f, LocalInertia);

    // Create the rigid body object
    btRigidBody *bullet = new btRigidBody(1.0f, MotionState, Shape, LocalInertia);

    //shoot it with speed
    bullet->applyImpulse(m_btShip->getWorldTransform().getBasis() * btVector3(0,0,-1000), m_btShip->getCenterOfMassPosition());

    MObject* obj = new MObject(bullet, Node, &Material::Shot);

    // Store a pointer to the irrlicht node so we can update it later
    bullet->setUserPointer((void *)(obj));

    // Add it to the world
    m_btWorld->addRigidBody(bullet);
    m_Objects.push_back(bullet);
    m_objects.push_back(std::unique_ptr<gg::MObject>(obj));
}

//following functions are copied from http://www.irrlicht3d.org/wiki/index.php?n=Main.GettingStartedWithBullet

// Runs the physics simulation.
// - TDeltaTime tells the simulation how much time has passed since the last frame so the simulation can run independently of the frame rate.
void  gg::MGame::UpdatePhysics(u32 TDeltaTime)
{
    //u32 b = m_irrTimer->getRealTime();
    m_btWorld->stepSimulation(TDeltaTime * 0.001f, 1);
    //std::cout << "bullet step time: " << m_irrTimer->getRealTime() - b << "\n";

    MCollisionResolver CollisionResolver(m_irrDevice.get(), m_btWorld);

    std::vector<MObject*> toDelete(CollisionResolver.getDeleted());

    for(list<btRigidBody *>::Iterator Iterator = m_Objects.begin(); Iterator != m_Objects.end(); ++Iterator)
    {
        UpdateRender(*Iterator);
    }

    if(!toDelete.empty())
    {
        for(MObject* obj : toDelete)
        {
            ClearObject(obj->getRigid());
        }
    }
}

// Passes bullet's orientation to irrlicht
void  gg::MGame::UpdateRender(btRigidBody *TObject)
{
    ISceneNode *Node = (static_cast<MObject *>(TObject->getUserPointer()))->getNode();
    if(Node)
    {
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
}

// Removes all objects from the world
void  gg::MGame::ClearAllObjects()
{
    for(list<btRigidBody *>::Iterator Iterator = m_Objects.begin(); Iterator != m_Objects.end(); ++Iterator)
    {
        btRigidBody *Object = *Iterator;

        // Delete irrlicht node
        ISceneNode *Node = (static_cast<MObject *>(Object->getUserPointer()))->getNode();
        Node->remove();
        Node = nullptr;

        // Remove the object from the world
        m_btWorld->removeRigidBody(Object);

        // Free memory
        delete Object->getMotionState();
        delete Object->getCollisionShape();
        delete Object;
        Object = nullptr;
    }
    m_Objects.clear();
}

void gg::MGame::ClearObject(btRigidBody * Object)
{
    // Delete irrlicht node
    ISceneNode *Node = (static_cast<MObject *>(Object->getUserPointer()))->getNode();
    Node->remove();

    // Remove the object from the world
    m_btWorld->removeRigidBody(Object);

    // Free memory
    delete Object->getMotionState();
    delete Object->getCollisionShape();
    delete Object;
}


