#ifndef OBJECT_H
#define OBJECT_H
#include "Edem.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <vector>

class Object
{
    btRigidBody* rigidBody;
    std::vector<EDEM> edems;
};

#endif
