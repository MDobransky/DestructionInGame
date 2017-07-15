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
#include <queue>
#include <condition_variable>
#include <fstream>
#include <iostream>

namespace gg
{

    class MCollisionResolver
    {

    public:
        MCollisionResolver(irr::IrrlichtDevice *, btDiscreteDynamicsWorld *, MObjectCreator *,
                           std::vector<std::unique_ptr<MObject>> *);

        ~MCollisionResolver();

        void resolveAll();

    private:
        void resolveCollision(MObject *object, btVector3 point, btScalar impulse,
                              MObject *other_object);

        void meshSubtractor(); //thread

        void meshDecomposer(); //thread

        void subtractionApplier(); //call every loop

        void decompositionApplier(); //call every loop

        irr::IrrlichtDevice *m_irrDevice;
        btDiscreteDynamicsWorld *m_btWorld;
        MObjectCreator *m_objectCreator;
        std::vector<std::unique_ptr<MObject>> *m_objects;
        std::deque<std::tuple<MObject *, irr::scene::IMesh *, irr::core::vector3df>> m_subtractionTasks;
        std::queue<std::tuple<MObject *, btVector3, btQuaternion, MeshManipulators::Nef_polyhedron,
                                            irr::scene::IMesh *, int>> m_subtractionResults;
        std::mutex m_subtractionTasksMutex;
        std::mutex m_subtractionResultsMutex;
        std::condition_variable m_subtractionCondVar;
        std::queue<std::tuple<MObject *, irr::scene::IMesh *>> m_decompositionTasks;
        std::queue<std::tuple<MObject *, btCollisionShape *>> m_decompositionResults;
        std::mutex m_decompositionTasksMutex;
        std::mutex m_decompositionResultsMutex;
        std::condition_variable m_decompositionCondVar;
        std::thread m_subtractor;
        std::thread m_decomposer;
        std::atomic<bool> m_done;

        std::ofstream m_file_subtraction;
        std::ofstream m_file_decomposition;
        std::ofstream m_file_total;
    };



}

#endif // COLLISIONRESOLVER_H
