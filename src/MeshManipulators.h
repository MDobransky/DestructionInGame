#ifndef MESHMANIPULATORS_H
#define MESHMANIPULATORS_H

#include <vector>

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btConvexHullComputer.h>
#include <LinearMath/btQuaternion.h>

namespace gg
{
    namespace MeshManipulators
    {
        btBvhTriangleMeshShape* convertMesh(irr::scene::IMeshSceneNode*);
        btConvexHullShape* convertMeshToHull(irr::scene::IMeshSceneNode *);
        irr::scene::IMesh* subtractMesh(irr::scene::IMesh* from, irr::scene::IMesh* what, irr::core::vector3df position);
    }

}


#endif
