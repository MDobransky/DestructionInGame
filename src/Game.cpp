#include "Game.h"
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
    delete m_events;
}

void gg::MGame::Run()
{
    m_events = new MEventReceiver(this);
    m_Done = false;

    m_loader = std::make_unique<MLoader>(1600,900,false);

    //load game from file
    m_objects = std::vector<std::unique_ptr<gg::MObject>>();
    m_irrDevice.reset(); //reset AFTER objects are deleted!!!
    std::tie(m_irrDevice, m_objects) = m_loader->load("media/levels/1");
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

    DebugDraw debugDraw(m_irrDevice.get());
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
       const bool debug_draw_bullet = false;



    CreateStartScene();


    m_velocity = -50;

    // Main loop
    u32 TimeStamp = m_irrTimer->getTime(), DeltaTime = 0;
    while(!m_Done) {
        m_irrDevice->setWindowCaption(std::to_wstring(m_irrDriver->getFPS()).c_str());

        DeltaTime = m_irrTimer->getTime() - TimeStamp;
        TimeStamp = m_irrTimer->getTime();

        m_btShip->setLinearVelocity(m_btShip->getWorldTransform().getBasis() * (btVector3( 0.f, -9.8f, m_velocity )));

        m_Camera->setTarget(m_IShip->getPosition());

        UpdatePhysics(DeltaTime);

        m_irrDriver->beginScene(true, true, SColor(255, 20, 0, 0));

        m_irrScene->drawAll();

        if (debug_draw_bullet)
        {
            m_irrDriver->setMaterial(debugMat);
            m_irrDriver->setTransform(irr::video::ETS_WORLD, irr::core::IdentityMatrix);
            m_btWorld->debugDrawWorld();
        }
        m_irrGUI->drawAll();
        m_irrDriver->endScene();
        m_irrDevice->run();
    }

    //ClearObjects();
    delete m_btWorld;
    delete Solver;
    delete Dispatcher;
    delete BroadPhase;
    delete CollisionConfiguration;

    //irrDevice->drop();
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

    MObject* obj = new MObject(bullet, Node);

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

    m_btWorld->stepSimulation(TDeltaTime * 0.001f, 60);


    int numManifolds = m_btWorld->getDispatcher()->getNumManifolds();
    //For each contact manifold
    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold* contactManifold = m_btWorld->getDispatcher()->getManifoldByIndexInternal(i);
        const btRigidBody* obA = static_cast<const btRigidBody*>(contactManifold->getBody0());
        const btRigidBody* obB = static_cast<const btRigidBody*>(contactManifold->getBody1());
        contactManifold->refreshContactPoints(obA->getWorldTransform(), obB->getWorldTransform());

       // btManifoldPoint& pt = contactManifold->getContactPoint(0);
        //btVector3 ptA = pt.getPositionWorldOnA();
        //btVector3 ptB = pt.getPositionWorldOnB();

        if(obA == m_btShip || obB == m_btShip)
            std::cout << "Game over\n";
        
        MObject* objectA = static_cast<MObject*>(obA->getUserPointer());
        MObject* objectB = static_cast<MObject*>(obB->getUserPointer());

        //World->removeCollisionObject(const_cast<btCollisionObject*>(obA));
    }

    std::vector<btRigidBody*> toDelete;
    // destroy objects out of bounds and relay the object's orientation to irrlicht
    int i = 0;
    for(list<btRigidBody *>::Iterator Iterator = m_Objects.begin(); Iterator != m_Objects.end(); ++Iterator)
    {
        UpdateRender(*Iterator);
       /* btVector3 pos = (*Iterator)->getCenterOfMassPosition();
        pos = (*Iterator)->getWorldTransform().getBasis() * m_terrainTransform.getBasis() * pos;

        if(pos.getX() < m_minBound.getX() || pos.getY() < m_minBound.getY() || pos.getZ() < m_minBound.getZ() ||
                pos.getX() > m_maxBoud.getX() || pos.getY() > m_maxBoud.getY() || pos.getZ() > m_maxBoud.getZ())
        {
         //   toDelete.push_back(*Iterator);
        }*/
    }

    if(!toDelete.empty())
    {
        for(btRigidBody* obj : toDelete)
        {
            ClearObject(obj);
        }
    }

    m_Camera->setTarget(m_IShip->getPosition());
}

// Passes bullet's orientation to irrlicht
void  gg::MGame::UpdateRender(btRigidBody *TObject)
{
    ISceneNode *Node = (static_cast<MObject *>(TObject->getUserPointer()))->getNode();

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


