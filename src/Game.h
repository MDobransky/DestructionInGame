#ifndef GAME_H
#define GAME_H

#include "Object.h"
#include "EventReceiver.h"
#include "Loader.h"
#include "CollisionResolver.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <cstdlib>
#include <utility>
#include <iostream>



namespace gg {

class MEventReceiver;

class MGame
{

    friend class MEventReceiver;

private:
    void CreateStartScene();
    std::tuple<btRigidBody*, irr::scene::IMeshSceneNode*> CreateShip(const btVector3 &TPosition);
    void CreateBox(const btVector3 &TPosition, const irr::core::vector3df &TScale, btScalar TMass);
    void UpdatePhysics(irr::u32 TDeltaTime);
    void UpdateRender(btRigidBody *TObject);
    void Shoot();
    void ApplyEvents();
    void ApplySettings();

    //IrrlichtDevice *irrDevice;
    btDiscreteDynamicsWorld *m_btWorld;

    bool m_done, m_paused = true;
    bool m_left = false;
    irr::video::IVideoDriver *m_irrDriver;
    irr::scene::ISceneManager *m_irrScene;
    irr::gui::IGUIEnvironment *m_irrGUI;
    irr::ITimer *m_irrTimer;
    irr::scene::ISceneNode* m_IShip;
    irr::scene::ICameraSceneNode* m_Camera;
    btRigidBody* m_btShip = 0;
    float m_velocity;
    std::unique_ptr<MObjectCreator> m_objectCreator;
    irr::u32 m_shot_time = 0;

    std::unique_ptr<irr::IrrlichtDevice> m_irrDevice;
    std::unique_ptr<MLoader> m_loader;
    std::vector<std::unique_ptr<MObject>> m_objects;
    MEventReceiver* m_events;
    btTransform m_terrainTransform;
    btVector3 m_minBound, m_maxBoud;
    std::vector<std::unique_ptr<btFixedConstraint>> m_constraints;

public:
    void Run(bool debug, bool gravity);
    ~MGame();
    MGame();
    MGame(const MGame&) = delete;
    MGame& operator=(const MGame&) = delete;

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
