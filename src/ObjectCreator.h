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
    Object createTerrain(std::string&);
    Object createRigidBody(std::string&);

};

}
#endif // OBJECTCREATOR_H
