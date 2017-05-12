//creating object from the level files and models
//create dirt, buildings and other models, invisible skyboxes, ship, fired shots....

#ifndef OBJECTCREATOR_H
#define OBJECTCREATOR_H

#include "Object.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btConvexHullComputer.h>
#include <LinearMath/btQuaternion.h>

#include <string>
#include <vector>

namespace gg {

class MObjectCreator
{
public:
    MObjectCreator(irr::IrrlichtDevice*);
    MObject* createMeshRigidBody(std::vector<std::string>&&);
    MObject* createBoxedRigidBody(std::vector<std::string>&&);
    MObject* createSolidGround(std::vector<std::string>&&);
    MObject* shoot(btVector3 position, btVector3 impulse);

private:
    irr::IrrlichtDevice* m_irrDevice;
    const std::string m_media = "media/";
};

}
#endif // OBJECTCREATOR_H
