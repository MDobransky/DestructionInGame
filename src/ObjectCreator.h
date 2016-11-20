//creating object from the level files and models
//create dirt, buildings and other models, invisible skyboxes, ship, fired shots....

#ifndef OBJECTCREATOR_H
#define OBJECTCREATOR_H

#include "Edem.h"
#include "Object.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <string>

namespace gg {


class ObjectCreator
{
public:
    ObjectCreator(IrrlichtDevice*);
    Object createTerrain(std::vector<std::string>&&);
    Object createRigidBody(std::vector<std::string>&&);
    Object createSolidGround(float, float); //size
    static btBvhTriangleMeshShape* convertMesh(IMeshSceneNode*);
private:
    IrrlichtDevice* irrDevice;
    const std::string media = "media/";
};

}
#endif // OBJECTCREATOR_H
