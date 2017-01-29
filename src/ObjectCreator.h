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

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

namespace gg {


class ObjectCreator
{
public:
    ObjectCreator(IrrlichtDevice*);
    Object createRigidBody(std::vector<std::string>&&, ISceneNode* parrent = NULL);
    Object createSolidGround(btRigidBody*); //size
    static btBvhTriangleMeshShape* convertMesh(IMeshSceneNode*);
private:
    IrrlichtDevice* irrDevice;
    const std::string media = "media/";
};

}
#endif // OBJECTCREATOR_H
