#ifndef COLLISIONRESOLVER_H
#define COLLISIONRESOLVER_H

#include "Object.h"
#include "ObjectCreator.h"

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

namespace gg {

class MCollisionResolver
{
public:
    MCollisionResolver(irr::IrrlichtDevice*, btDiscreteDynamicsWorld*, MObjectCreator*, std::vector<std::unique_ptr<MObject>>*);
    ~MCollisionResolver();
    bool isInside(btRigidBody* body, btVector3 point, btVector3 from);
    void resolveAll();
private:
    void resolveCollision(MObject* object, btVector3 point, btVector3 from, btScalar impulse, MObject::Material other_object);
    void generateDebree(irr::scene::IMesh*, btVector3 point, btVector3 impulse, MObject::Material);
    void meshSubtractor();
    void subtractionApplier();
    irr::IrrlichtDevice* m_irrDevice;
    btDiscreteDynamicsWorld* m_btWorld;
    MObjectCreator* m_objectCreator;
    std::vector<std::unique_ptr<MObject>>* m_objects;
    std::vector<MObject*> m_toDelete;
    std::deque<std::tuple<MObject*, irr::scene::IMesh*, irr::core::vector3df>> m_subtractionTasks;
    std::deque<std::tuple<MObject*, irr::scene::IMesh*>> m_subtractionResults;
    std::mutex m_taskQueueMutex;
    std::mutex m_resultQueueMutex;
    std::thread m_subtractor1;
    std::thread m_subtractor2;
    std::atomic<bool> m_done;

    class wall_custom : public voro::wall
    {
    public:

        wall_custom(MCollisionResolver* resolv, btRigidBody* body, btVector3& dir, int id=-42)
            : m_resolver(resolv), m_rigidBody(body), m_direction(dir), m_id(id)
        {}

        bool point_inside(double x, double y, double z)
        {
            btVector3 point(x,y,z);
            return m_resolver->isInside(m_rigidBody, point, m_direction);
        }

        template<class TCell>
        bool cut_cell_base(TCell& c, double x, double y, double z)
        {
            return c.nplane(x,y,z,m_id);
            return true;
        }

        bool cut_cell(voro::voronoicell& c, double x, double y, double z)
        {
            return cut_cell_base(c,x,y,z);
        }
        bool cut_cell(voro::voronoicell_neighbor& c, double x, double y, double z)
        {
            return cut_cell_base(c,x,y,z);
        }
    private:
        MCollisionResolver* m_resolver;
        btRigidBody* m_rigidBody;
        btVector3 m_direction;
        int m_id;
 };
};

}

#endif // COLLISIONRESOLVER_H
