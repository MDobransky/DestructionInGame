#ifndef GAME_H
#define GAME_H

#include "Object.h"
#include "EventReceiver.h"
#include "Loader.h"
#include "CollisionResolver.h"
#include "ObjectCreator.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <cstdlib>
#include <utility>
#include <iostream>

namespace gg {

class MGame
{
public:
    void run(bool debug, bool gravity);
    ~MGame();
    MGame();
    MGame(const MGame&) = delete;
    MGame& operator=(const MGame&) = delete;

private:
    void createStartScene();
    void createEngineGlow(irr::scene::ISceneNode* parent);
    void updatePhysics(irr::u32 TDeltaTime);
    void updateRender(btRigidBody *TObject);
    void shoot();
    void applyEvents();
    void applySettings();

    btDiscreteDynamicsWorld *m_btWorld;
    std::unique_ptr<irr::IrrlichtDevice> m_irrDevice;
    irr::video::IVideoDriver *m_irrDriver;
    irr::scene::ISceneManager *m_irrScene;
    irr::gui::IGUIEnvironment *m_irrGUI;
    irr::ITimer *m_irrTimer;
    irr::scene::ICameraSceneNode* m_Camera;
    irr::scene::ISceneNode* m_IShip;
    btRigidBody* m_btShip = 0;
    btDefaultCollisionConfiguration* m_collisionConfiguration;
    btBroadphaseInterface* m_broadPhase;
    btCollisionDispatcher* m_dispatcher;
    btSequentialImpulseConstraintSolver* m_solver;

    std::unique_ptr<MObjectCreator> m_objectCreator;
    std::vector<std::unique_ptr<MObject>> m_objects;
    std::unique_ptr<MCollisionResolver> m_resolver;
    MEventReceiver* m_events;

    float m_velocity;
    irr::u32 m_shot_time = 0;
    bool m_done, m_paused = true;
};

class MDebugDraw : public btIDebugDraw
{

public:

   MDebugDraw(irr::IrrlichtDevice* const device) :
      mode(DBG_NoDebug), driver(device->getVideoDriver()), logger(device->getLogger())
   {

   }

   void drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
   {
      //workaround to bullet's inconsistent debug colors which are either from 0.0 - 1.0 or from 0.0 - 255.0
      irr::video::SColor newColor(255, (irr::u32)color[0], (irr::u32)color[1], (irr::u32)color[2]);
      if (color[0] <= 1.0 && color[0] > 0.0)
         newColor.setRed((irr::u32)(color[0]*255.0));
      if (color[1] <= 1.0 && color[1] > 0.0)
         newColor.setGreen((irr::u32)(color[1]*255.0));
      if (color[2] <= 1.0 && color[2] > 0.0)
         newColor.setBlue((irr::u32)(color[2]*255.0));

      this->driver->draw3DLine(
         irr::core::vector3df(from[0], from[1], from[2]),
         irr::core::vector3df(to[0], to[1], to[2]),
         newColor);
   }

   void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int, const btVector3&)
   {
      static const irr::video::SColor CONTACTPOINT_COLOR(255, 255, 255, 0); //bullet's are black :(

   //   this->drawLine(PointOnB, PointOnB + normalOnB*distance, CONTACTPOINT_COLOR);

      const btVector3 to(PointOnB + normalOnB*distance);

      this->driver->draw3DLine(
         irr::core::vector3df(PointOnB[0], PointOnB[1], PointOnB[2]),
         irr::core::vector3df(to[0], to[1], to[2]),
         CONTACTPOINT_COLOR);
   }

   void reportErrorWarning(const char* text)
   {
      this->logger->log(text, irr::ELL_ERROR);
   }

   void draw3dText(const btVector3&, const char*) { }

   void setDebugMode(int mode) { this->mode = mode; }

   int getDebugMode() const { return this->mode; }

private:

   int mode;

   irr::video::IVideoDriver* const driver;

   irr::ILogger* logger;
};

}

#endif // GAME_H
