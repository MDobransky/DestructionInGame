#include "Game.h"
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
    m_events = new MEventReceiver();
    m_done = false;
    m_velocity = -10;

    m_irrDevice.reset(createDevice(video::EDT_OPENGL, dimension2d<u32>(1920, 1080), 32, false, false, false, m_events));
    m_irrGUI = m_irrDevice->getGUIEnvironment();
    m_irrTimer = m_irrDevice->getTimer();
    m_irrScene = m_irrDevice->getSceneManager();
    m_irrDriver = m_irrDevice->getVideoDriver();

    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_broadPhase = new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000));
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_solver = new btSequentialImpulseConstraintSolver();
    m_btWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadPhase, m_solver, m_collisionConfiguration);

    m_objectCreator.reset(new MObjectCreator(m_irrDevice.get()));
    m_resolver = std::make_unique<MCollisionResolver>(m_irrDevice.get(), m_btWorld, m_objectCreator.get(), &m_objects);
}

gg::MGame::~MGame()
{
    //delete m_dispatcher;
    //delete m_collisionConfiguration;
    //delete m_broadPhase;
    //delete m_solver;
    delete m_btWorld;

}

void gg::MGame::run(bool debug, bool gravity)
{

    ITexture *images = m_irrDriver->getTexture("media/loading.jpg");
    m_irrDevice->getCursorControl()->setVisible(0);
    m_irrDevice->setWindowCaption(L"Simulation of environment destruction");
    m_irrDriver->beginScene(true, true, SColor(255, 20, 0, 0));
    m_irrScene->drawAll();
    m_irrDriver->draw2DImage(images, core::position2d<s32>(0, 0), core::rect<s32>(0, 0, 1920, 1080), 0,
                             video::SColor(255, 255, 255, 255), false);
    m_irrDriver->endScene();

    MLoader loader(m_irrDevice.get());
    m_objects = loader.load("media/levels/3");
    for(size_t i = 0; i < m_objects.size(); i++)
    {
        //ship
        if(i == 1)
        {
            m_btShip = m_objects[i]->getRigid();
            m_btShip->setActivationState(DISABLE_DEACTIVATION);
            m_btShip->setLinearVelocity(m_btShip->getWorldTransform().getBasis() * (btVector3(0.f, 0.0f, m_velocity)));
            m_IShip = m_objects[i]->getNode();
        }
        m_btWorld->addRigidBody(m_objects[i]->getRigid());
        m_objects[i]->getRigid()->setDamping(0.9f, 0.9f);
        m_objects[i]->getRigid()->activate();
    }


    MDebugDraw debugDraw(m_irrDevice.get());
    debugDraw.setDebugMode(
            btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_DrawContactPoints |
                    btIDebugDraw::DBG_DrawConstraints //|
                          );
    m_btWorld->setDebugDrawer(&debugDraw);
    irr::video::SMaterial debugMat;
    debugMat.Lighting = false;
    const bool debug_draw_bullet = debug;

    m_btWorld->setGravity(btVector3(0, gravity ? -9 : 0, 0));
    m_btShip->setGravity(btVector3(0, 0, 0));
    createStartScene();

    m_irrScene->getGUIEnvironment()->addStaticText(L"FPS:", rect<s32>(35, 35, 50, 50), false, false, 0);
    IGUIStaticText *fpsTextElement = m_irrScene->getGUIEnvironment()->addStaticText(L"", rect<s32>(50, 35, 180, 50),
                                                                                    false, false, 0);

    u32 TimeStamp = m_irrTimer->getTime(), DeltaTime = 0;
///////////////////////////////////////////Main loop
    while(!m_done)
    {
        fpsTextElement->setText(std::to_wstring(m_irrDriver->getFPS()).c_str());

        DeltaTime = m_irrTimer->getTime() - TimeStamp;
        TimeStamp = m_irrTimer->getTime();
        applySettings();
        if(!m_paused)
        {
            applyEvents();
            m_btShip->setLinearVelocity(m_btShip->getWorldTransform().getBasis() * (btVector3(0.f, 0.0f, m_velocity)));
            updatePhysics(DeltaTime);
        }
        //camera
        btVector3 newup = quatRotate(m_btShip->getOrientation(), btVector3(0, 1, 0));
        btVector3 newtarg = m_btShip->getCenterOfMassPosition() + m_btShip->getLinearVelocity();
        m_Camera->setUpVector(vector3df(newup.getX(), newup.getY(), newup.getZ()));
        m_Camera->setTarget(vector3df(newtarg.getX(), newtarg.getY(), newtarg.getZ()));
        //draw
        m_irrDriver->beginScene(true, true, SColor(255, 20, 0, 0));
        m_irrScene->drawAll();

        if(debug_draw_bullet)
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

}

void gg::MGame::createStartScene()
{
    // Create the initial scene
    m_irrScene->addLightSceneNode(0, core::vector3df(0, 7000, 0), SColorf(4, 4, 4, 1), 10000);
    m_irrScene->setAmbientLight(video::SColorf(0.3, 0.3, 0.3, 1));

    //camera
    m_Camera = m_irrScene->addCameraSceneNode();
    btVector3 trans = m_btShip->getWorldTransform().getBasis() * btVector3(0, 0.3, 1);
    m_Camera->setPosition(vector3df(trans.getX(), trans.getY(), trans.getZ()));
    m_Camera->setParent(m_IShip);
    btVector3 newtarg = m_btShip->getCenterOfMassPosition() + m_btShip->getLinearVelocity();
    m_Camera->setTarget(vector3df(newtarg.getX(), newtarg.getY(), newtarg.getZ()));

    //rendered distance
    m_Camera->setFarValue(1000);

    createEngineGlow(m_IShip);

    for(auto &&object : m_objects)
    {
        updateRender(object->getRigid());
    }
}

void gg::MGame::createEngineGlow(ISceneNode *parent)
{
    scene::IParticleSystemSceneNode *ps = m_irrScene->addParticleSystemSceneNode(false);
    ps->setParent(parent);

    scene::IParticleEmitter *em = ps->createSphereEmitter(vector3df(0, 0, 0), 0.01,
                                                          core::vector3df(0.0f, 0.0f, 0.0f),   // initial direction
                                                          300, 500,                             // emit rate
                                                          video::SColor(0, 0, 0, 0),       // darkest color
                                                          video::SColor(0, 250, 250, 250),       // brightest color
                                                          300, 500, 0,                         // min and max age, angle
                                                          core::dimension2df(0.1, 0.1),         // min size
                                                          core::dimension2df(0.15f, 0.15f));        // max size

    ps->setEmitter(em); // this grabs the emitter
    em->drop(); // so we can drop it here without deleting it

    scene::IParticleAffector *paf = ps->createFadeOutParticleAffector();

    ps->addAffector(paf); // same goes for the affector
    paf->drop();

    ps->setPosition(core::vector3df(0, 0, 0.35));
    ps->setScale(core::vector3df(1, 1, 1));
    ps->setMaterialFlag(video::EMF_LIGHTING, false);
    ps->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
    ps->setMaterialTexture(0, m_irrDriver->getTexture("media/glow.jpg"));
    ps->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
}

void gg::MGame::applyEvents()
{
    const float torque = std::sqrt(m_btShip->getLinearVelocity().length()) / 100 + 0.01;

    if(m_events->keyDown('W'))
    {
        m_btShip->setDamping(0, 0.9);
        m_btShip->setAngularVelocity(
                m_btShip->getWorldTransform().getBasis() * btVector3(-torque, 0, 0) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('S'))
    {
        m_btShip->setDamping(0, 0.9);
        m_btShip->setAngularVelocity(
                m_btShip->getWorldTransform().getBasis() * btVector3(torque, 0, 0) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('Q'))
    {
        m_btShip->setDamping(0, 0.9);
        m_btShip->setAngularVelocity(
                m_btShip->getWorldTransform().getBasis() * btVector3(0, -torque, 0) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('E'))
    {
        m_btShip->setDamping(0, 0.9);
        m_btShip->setAngularVelocity(
                m_btShip->getWorldTransform().getBasis() * btVector3(0, torque, 0) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('A'))
    {
        m_btShip->setDamping(0, 0.9);
        m_btShip->setAngularVelocity(
                m_btShip->getWorldTransform().getBasis() * btVector3(0, 0, -torque) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('D'))
    {
        m_btShip->setDamping(0, 0.9);
        m_btShip->setAngularVelocity(
                m_btShip->getWorldTransform().getBasis() * btVector3(0, 0, torque) + m_btShip->getAngularVelocity());
    }

    if(m_events->keyDown('Z'))
    {
        m_velocity -= 5;
        if(m_velocity < -80)
        {
            m_velocity = -80;
        }
    }

    if(m_events->keyDown('X'))
    {
        m_velocity += 5;
        if(m_velocity >= -20)
        {
            m_velocity = -20;
        }
    }

    if(m_events->keyDown(irr::KEY_SPACE))
    {
        if(m_irrTimer->getTime() - m_shot_time > 800)
        {
            m_shot_time = m_irrTimer->getTime();
            shoot();
        }
    }

}

void gg::MGame::applySettings()
{
    if(m_paused && m_events->keyPressed(irr::KEY_SPACE))
    {
        m_paused = false;
    }

    if(!m_paused && m_events->keyPressed('P'))
    {
        m_paused = true;
    }

    if(m_events->keyDown(irr::KEY_ESCAPE))
    {
        m_done = true;
    }
}

void gg::MGame::shoot()
{
    btVector3 position =
            m_btShip->getCenterOfMassPosition() + m_btShip->getWorldTransform().getBasis() * btVector3(0, -0.1, -0.5);
    btVector3 impulse = m_btShip->getWorldTransform().getBasis() * btVector3(0, 0, -200);
    std::unique_ptr<MObject> shot(m_objectCreator->shoot(position, impulse));
    m_btWorld->addRigidBody(shot->getRigid());
    shot->getRigid()->setGravity(btVector3(0, 0, 0));
    m_objects.push_back(std::move(shot));

}

void gg::MGame::updatePhysics(u32 TDeltaTime)
{
    m_objects.erase(std::remove_if(m_objects.begin(), m_objects.end(),
                                   [](auto &&x) { return x->deleted.load() && x->reference_count == 0; }),
                    m_objects.end());
    m_btWorld->stepSimulation(TDeltaTime * 0.001f, 1, 1. / 60.);
    for(auto &&object : m_objects)
    {
        if(!object->isDeleted())
        {
            updateRender(object->getRigid());
        }
    }
    m_resolver->resolveAll();
}

void gg::MGame::updateRender(btRigidBody *TObject)
{
    ISceneNode *Node = (static_cast<MObject *>(TObject->getUserPointer()))->getNode();
    if(Node)
    {
        // Set position
        btVector3 Point = TObject->getCenterOfMassPosition();
        Node->setPosition(vector3df((f32) Point.getX(), (f32) Point.getY(), (f32) Point.getZ()));
        // Set rotation
        vector3df Euler;
        const btQuaternion &TQuat = TObject->getOrientation();
        quaternion q(TQuat.getX(), TQuat.getY(), TQuat.getZ(), TQuat.getW());
        q.toEuler(Euler);
        Euler *= RADTODEG;
        Node->setRotation(Euler);
    }
}


