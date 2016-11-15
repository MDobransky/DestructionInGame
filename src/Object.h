//creates objects either by loading them or by gluing together edem elements (but without new joints)

#ifndef OBJECT_H
#define OBJECT_H
#include "Edem.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <vector>

namespace gg {

class Object
{
    btRigidBody* rigidBody;
    std::vector<EDEM> edems;

};

}
#endif
