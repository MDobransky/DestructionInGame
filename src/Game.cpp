#include "Game.h"
#include "Material.h"
#include <iostream>
#include <string>
#include <algorithm>


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
double rnd() {return double(rand())/RAND_MAX;}
void gg::MGame::Run(bool debug, bool gravity)
{
    m_events = new MEventReceiver();
    m_done = false;
    m_irrDevice.reset(createDevice(video::EDT_OPENGL, dimension2d<u32>(1920,1080), 32, false, false, false, m_events));
    m_loader = std::make_unique<MLoader>(m_irrDevice.get());
    std::tie(m_objects, m_constraints) = m_loader->load("media/levels/1");

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

    m_resolver = std::make_unique<MCollisionResolver>(m_irrDevice.get(), m_btWorld);
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
//apply parameters
    const bool debug_draw_bullet = debug;
    if(!gravity)
    {
        m_btWorld->setGravity(btVector3(0,0,0));
    }
    else
    {
        m_btWorld->setGravity(btVector3(0,-256,0));
    }
    m_velocity = -10;
    CreateStartScene();

    IGUIStaticText* fpsTextElement = m_irrScene->getGUIEnvironment()->addStaticText(L"", rect<s32>(35, 35, 140, 50), false, false, 0);
///////////////////////////////////////////
    // Main loop
    u32 TimeStamp = m_irrTimer->getTime(), DeltaTime = 0;
    //u32 realStamp = m_irrTimer->getRealTime();
    stringw str = L"FPS: ";
    stringw fps;
    while(!m_done) {
        fps = str;
        fps += m_irrDriver->getFPS();
        fpsTextElement->setText(fps.c_str());
        m_irrDevice->setWindowCaption(fps.c_str());

        DeltaTime = m_irrTimer->getTime() - TimeStamp;
        TimeStamp = m_irrTimer->getTime();
        ApplySettings();
        if(!m_paused)
        {
            ApplyEvents();
            m_btShip->setLinearVelocity(m_btShip->getWorldTransform().getBasis() * (btVector3( 0.f, 0.0f, m_velocity )));
            UpdatePhysics(DeltaTime);
        }
        //camera
        btVector3 newup = quatRotate(m_btShip->getOrientation(),btVector3(0,1,0));
        btVector3 newtarg = m_btShip->getCenterOfMassPosition() +  m_btShip->getLinearVelocity();
        m_Camera->setUpVector(vector3df(newup.getX(),newup.getY(),newup.getZ()));
        m_Camera->setTarget(vector3df(newtarg.getX(),newtarg.getY(),newtarg.getZ()));
        //draw
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
/////////////////////////////////////////////////////////////
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

    //put objects created by loader into bullet world
    for(size_t i = 0; i < m_objects.size(); i++)
    {
        //ship
        if(i == 1)
        {
            m_btShip = m_objects[i]->getRigid();
            m_btShip->setActivationState(DISABLE_DEACTIVATION);
            m_btShip->setLinearVelocity(m_btShip->getWorldTransform().getBasis() * (btVector3( 0.f, 0.0f, m_velocity )));
            m_IShip = m_objects[i]->getNode();
        }
        m_btWorld->addRigidBody(m_objects[i]->getRigid());
        m_objects[i]->getRigid()->setDamping(0.9f,0.9f);
        m_objects[i]->getRigid()->activate();
    }
    for(size_t i = 0; i < m_constraints.size(); i++)
    {
        m_btWorld->addConstraint(m_constraints[i].get());
    }

    m_btShip->setGravity(btVector3(0,0,0));
    m_terrainTransform.setIdentity();
    m_objects[0]->getRigid()->getCollisionShape()->getAabb(m_terrainTransform,m_maxBoud,m_minBound);
    m_minBound.setY(m_minBound.getY()+51);

    //camera
    m_Camera = m_irrScene->addCameraSceneNode();
    btVector3 trans = m_btShip->getWorldTransform().getBasis() * btVector3(0,0.3,1);
    m_Camera->setPosition(vector3df(trans.getX(),trans.getY(),trans.getZ()));
    m_Camera->setParent(m_IShip);
    btVector3 newtarg = m_btShip->getCenterOfMassPosition() +  m_btShip->getLinearVelocity();
    m_Camera->setTarget(vector3df(newtarg.getX(),newtarg.getY(),newtarg.getZ()));

    //rendered distance
    m_Camera->setFarValue(1000);


    scene::IParticleSystemSceneNode* ps =
    m_irrScene->addParticleSystemSceneNode(false);
    ps->setParent(m_IShip);

    scene::IParticleEmitter* em = ps->createSphereEmitter(
        vector3df(0,0,0),
        0.01,
        core::vector3df(0.0f,0.0f,0.0f),   // initial direction
        300,500,                             // emit rate
        video::SColor(0,0,0,0),       // darkest color
        video::SColor(0,250,250,250),       // brightest color
        300,500,0,                         // min and max age, angle
        core::dimension2df(0.1,0.1),         // min size
        core::dimension2df(0.15f,0.15f));        // max size

    ps->setEmitter(em); // this grabs the emitter
    em->drop(); // so we can drop it here without deleting it

    scene::IParticleAffector* paf = ps->createFadeOutParticleAffector();

    ps->addAffector(paf); // same goes for the affector
    paf->drop();

    ps->setPosition(core::vector3df(0,0,0.35));
    ps->setScale(core::vector3df(1,1,1));
    ps->setMaterialFlag(video::EMF_LIGHTING, false);
    ps->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
    ps->setMaterialTexture(0, m_irrDriver->getTexture("media/glow.jpg"));
    ps->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    //ps->doParticleSystem(1000);

    for(auto&& object : m_objects)
    {
        UpdateRender(object->getRigid());
    }
}

void  gg::MGame::ApplyEvents()
{
    const float torque = std::sqrt(m_btShip->getLinearVelocity().length())/100 + 0.01;

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

    if(m_events->keyDown('Q'))
    {
        m_btShip->setDamping(0,0.9);
        m_btShip->setAngularVelocity(m_btShip->getWorldTransform().getBasis() * btVector3(0,-torque,0) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('E'))
    {
        m_btShip->setDamping(0,0.9);
        m_btShip->setAngularVelocity(m_btShip->getWorldTransform().getBasis() * btVector3(0,torque,0) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('A'))
    {
        m_btShip->setDamping(0,0.9);
        m_btShip->setAngularVelocity(m_btShip->getWorldTransform().getBasis() * btVector3(0,0,-torque) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('D'))
    {
        m_btShip->setDamping(0,0.9);
        m_btShip->setAngularVelocity(m_btShip->getWorldTransform().getBasis() * btVector3(0,0,torque) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('Z'))
    {
        m_velocity -= 1;
        if(m_velocity < -40) m_velocity = -40;
    }

    if(m_events->keyDown('X'))
    {
        m_velocity += 1;
        if(m_velocity >= 0) m_velocity = 0;
    }

    if(m_events->keyDown(irr::KEY_SPACE))
    {
        if(m_irrTimer->getTime() - m_shot_time > 300)
        {
            m_shot_time = m_irrTimer->getTime();
            Shoot();
        }
    }

}

void gg::MGame::ApplySettings()
{
    if(m_paused && m_events->keyPressed(irr::KEY_SPACE))
    {
        m_paused = false;
    }

    if(m_events->keyPressed('P'))
    {
        m_paused = true;
    }

    if(m_events->keyDown(irr::KEY_ESCAPE))
    {
        m_done = true;
    }
}

void  gg::MGame::Shoot()
{
    ///bullet size
    ISceneNode* Node = m_irrScene->addLightSceneNode();
    Node->setMaterialType(EMT_SOLID);
    Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);

    // Set the initial position
    btTransform Transform;
    Transform.setIdentity();

    btVector3 start(0,-0.1,-0.5);
    Transform.setOrigin(m_btShip->getCenterOfMassPosition() + m_btShip->getWorldTransform().getBasis() * start);

    // Give it a default MotionState
    btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

    // Create the shape/hitbox
    btCollisionShape *Shape = new btSphereShape(0.1f);

    // Add mass
    btVector3 LocalInertia;
    btScalar Mass = 750.0f;
    Shape->calculateLocalInertia(Mass, LocalInertia);

    // Create the rigid body object
    btRigidBody *bullet = new btRigidBody(Mass, MotionState, Shape, LocalInertia);

    //shoot it with speed
    bullet->applyImpulse(m_btShip->getWorldTransform().getBasis() * btVector3(0,0,-75000), m_btShip->getCenterOfMassPosition());

    MObject* obj = new MObject(bullet, Node, &MMaterial::Shot);

    // Store a pointer to the irrlicht node so we can update it later
    bullet->setUserPointer((void *)(obj));

    // Add it to the world
    m_btWorld->addRigidBody(bullet);
    bullet->setGravity(btVector3(0,0,0));

    m_objects.push_back(std::unique_ptr<gg::MObject>(obj));


    // attach billboard to light

    ISceneNode* nodeB = m_irrScene->addBillboardSceneNode(Node, core::dimension2d<f32>(0.5, 0.5));
    nodeB->setMaterialFlag(video::EMF_LIGHTING, false);
    nodeB->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    nodeB->setMaterialTexture(0, m_irrDriver->getTexture("media/shot3.jpg"));

    //nodeB->remove();

}

//following functions are copied from http://www.irrlicht3d.org/wiki/index.php?n=Main.GettingStartedWithBullet

// Runs the physics simulation.
// - TDeltaTime tells the simulation how much time has passed since the last frame so the simulation can run independently of the frame rate.
void  gg::MGame::UpdatePhysics(u32 TDeltaTime)
{
    m_objects.erase(std::remove_if(m_objects.begin(),m_objects.end(),[](auto&& x){return x->isDeleted();}),m_objects.end());
    m_btWorld->stepSimulation(TDeltaTime * 0.001f, 1, 1./60.);
    for(auto&& object : m_objects)
    {
        if(!object->isDeleted())
        {
            UpdateRender(object->getRigid());
        }
    }
    m_resolver->resolveAll();
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


