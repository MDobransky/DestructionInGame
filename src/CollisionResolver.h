#ifndef COLLISIONRESOLVER_H
#define COLLISIONRESOLVER_H

#include "Object.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <voro++/voro++.hh>

#include <cstdlib>
#include <utility>
#include <vector>

namespace gg {

class MCollisionResolver
{
public:
    MCollisionResolver(irr::IrrlichtDevice*, btDiscreteDynamicsWorld*, std::vector<std::unique_ptr<MObject>>*);
    ~MCollisionResolver();
    std::vector<MObject*> getDeleted();
    bool isInside(btRigidBody* body, btVector3 point, btVector3 from);
    void resolveAll();
private:
    void resolveCollision(MObject* object, btVector3 point, btVector3 from, btScalar impulse);
    void generateVoro(MObject* body, btVector3 point, btVector3 from, btScalar size);
    irr::IrrlichtDevice* m_irrDevice;
    btDiscreteDynamicsWorld* m_btWorld;
    std::vector<std::unique_ptr<MObject>>* m_objects;
    std::vector<MObject*> m_toDelete;

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
            //std::cout << "cut\n";
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
