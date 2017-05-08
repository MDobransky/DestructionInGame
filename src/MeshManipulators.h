#ifndef MESHMANIPULATORS_H
#define MESHMANIPULATORS_H

#include <vector>

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btConvexHullComputer.h>
#include <LinearMath/btQuaternion.h>
#include "../cork/include/cork.h"

namespace gg
{
    namespace MeshManipulators
    {
        btBvhTriangleMeshShape* convertMesh(irr::scene::IMeshSceneNode*);
        btConvexHullShape* convertMeshToHull(irr::scene::IMeshSceneNode *);
    }

}


#endif
