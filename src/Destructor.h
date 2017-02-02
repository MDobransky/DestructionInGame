#ifndef DESTRUCTOR_H
#define DESTRUCTOR_H

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <cstdlib>
#include <utility>

namespace gg {

class MDestructor
{
public:
    MDestructor();
    ~MDestructor();
    static void resolveCollision(MObject*, btVector3&, MObject*, btVector3&);
    

};

}
#endif // DESTRUCTOR_H
