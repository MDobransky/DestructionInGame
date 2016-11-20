//creates objects either by loading them or by gluing together edem elements (but without new joints)

#ifndef OBJECT_H
#define OBJECT_H
#include "Edem.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <vector>
#include <memory>
#include <iostream>

namespace gg {

class Object
{
private:
    std::unique_ptr<btRigidBody> rigidBody;
    std::vector<EDEM> edems;
    bool empty;
public:
    btRigidBody* getRigid() { return rigidBody.get(); }
    bool isEmpty() { return empty; }

    ~Object()
    {
        if(rigidBody.get() != nullptr)
        {
            if(rigidBody->getMotionState() != nullptr)
                delete rigidBody->getMotionState();
            if(rigidBody->getCollisionShape() != nullptr)
                delete rigidBody->getCollisionShape();
            ISceneNode *node = static_cast<ISceneNode *>(rigidBody->getUserPointer());
            if(node) node->remove();
        }
    }
    Object () : empty(true) {}
    Object (btRigidBody* rb) :rigidBody(std::unique_ptr<btRigidBody>(rb))
    {
        empty = rigidBody == nullptr;
    }
    Object (Object&& newObj)
    {
        rigidBody = std::move(newObj.rigidBody);
        edems = std::move(newObj.edems);
    }

    Object (Object&) = delete;
    Object& operator= (Object&& newObj) = delete;
    Object&  operator= (const Object&) = delete;
};

}
#endif
