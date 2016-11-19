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
private:
    btRigidBody* rigidBody;
    std::vector<EDEM> edems;
public:
    ~Object() {}
    Object () {}
    Object (Object&&) = default;
    Object (Object&) = default;
    Object& operator= (Object&&) = default;
    Object&  operator= (const Object&) = default;

};

}
#endif
