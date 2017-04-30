#ifndef COLLISIONRESOLVER_H
#define COLLISIONRESOLVER_H

#include "Object.h"
//#include "Material.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <cstdlib>
#include <utility>
#include <vector>

namespace gg {

class MCollisionResolver
{
public:
    MCollisionResolver(irr::IrrlichtDevice*, btDiscreteDynamicsWorld*);
    ~MCollisionResolver();
    std::vector<MObject*> getDeleted();
    bool isInside(btRigidBody*, btVector3&);
    void resolveAll();
private:
    void resolveCollision(MObject*, btVector3&, btScalar&);
    irr::IrrlichtDevice* m_irrDevice;
    btDiscreteDynamicsWorld * m_btWorld;
    std::vector<MObject*> m_toDelete;
};

}

#endif // COLLISIONRESOLVER_H
