#ifndef COLLISIONRESOLVER_H
#define COLLISIONRESOLVER_H

#include "Object.h"
#include "ObjectCreator.h"
#include "MeshManipulators.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <voro++/voro++.hh>

#include <cstdlib>
#include <utility>
#include <vector>
#include <thread>
#include <future>
#include <mutex>
#include <deque>
#include <atomic>

namespace gg
{

    class MCollisionResolver
    {

    public:
        MCollisionResolver(irr::IrrlichtDevice *, btDiscreteDynamicsWorld *, MObjectCreator *,
                           std::vector<std::unique_ptr<MObject>> *);

        ~MCollisionResolver();

        bool isInside(btRigidBody *body, btVector3 point, btVector3 from);

        void resolveAll();

    private:
        void resolveCollision(MObject *object, btVector3 point, btVector3 from, btScalar impulse,
                              MObject *other_object);

        void generateDebree(irr::scene::IMesh *, btVector3 point, btVector3 impulse, MObject::Material);

        void meshSubtractor();

        void subtractionApplier();

        irr::IrrlichtDevice *m_irrDevice;
        btDiscreteDynamicsWorld *m_btWorld;
        MObjectCreator *m_objectCreator;
        std::vector<std::unique_ptr<MObject>> *m_objects;
        std::vector<MObject *> m_toDelete;
        std::deque<std::tuple<MObject *, irr::scene::IMesh *, irr::core::vector3df>> m_subtractionTasks;
        std::deque<std::tuple<MObject *, btVector3, btQuaternion, MeshManipulators::Nef_polyhedron,
                                            irr::scene::IMesh *, int>> m_subtractionResults;
        std::mutex m_taskQueueMutex;
        std::mutex m_resultQueueMutex;
        std::thread m_subtractor1;
        std::thread m_subtractor2;
        std::atomic<bool> m_done;
    };

}

#endif // COLLISIONRESOLVER_H
