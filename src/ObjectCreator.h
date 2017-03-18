//creating object from the level files and models
//create dirt, buildings and other models, invisible skyboxes, ship, fired shots....

#ifndef OBJECTCREATOR_H
#define OBJECTCREATOR_H

#include "Edem.h"
#include "Object.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btConvexHullComputer.h>
#include <LinearMath/btQuaternion.h>

#include <string>
#include <set>

namespace gg {

class MObjectCreator
{

public:
    MObjectCreator(irr::IrrlichtDevice*);
    std::vector<MObject*> createDestructibleBody(std::vector<std::string>&&, irr::scene::ISceneNode* parrent = NULL);
    MObject* createMeshRigidBody(std::vector<std::string>&&, irr::scene::ISceneNode* parrent = NULL);
    MObject* createConvexRigidBody(std::vector<std::string>&&, irr::scene::ISceneNode* parrent = NULL);
    MObject* createSolidGround(btRigidBody*); //size
    static btBvhTriangleMeshShape* convertMesh(irr::scene::IMeshSceneNode*);
    static btConvexHullShape* convertMeshToHull(irr::scene::IMeshSceneNode *);
    std::vector<std::unique_ptr<MObject>> decompose(MObject*);
private:
    class tetrahedron
    {
    public:
        btVector3 points[4];
        int neighbours[4];
        btVector3 center;
    };

    irr::IrrlichtDevice* m_irrDevice;
    const std::string m_media = "media/";
    std::vector<std::unique_ptr<MObject>> voronoiBBShatter(const btAlignedObjectArray<btVector3>&, const btVector3&, const btVector3&, const btQuaternion&, btScalar);
    std::vector<btVector3> getVertices(irr::scene::IMeshSceneNode*);
    std::vector<btVector3> getVertices(btConvexHullShape*);
};

}
#endif // OBJECTCREATOR_H
