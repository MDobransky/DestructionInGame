#include "CollisionResolver.h"

gg::MCollisionResolver::MCollisionResolver(IrrlichtDevice *irrDev, btDiscreteDynamicsWorld *btDDW) : m_irrDevice(irrDev), m_btWorld(btDDW)
{

}

gg::MCollisionResolver::~MCollisionResolver()
{

}

std::vector<gg::MObject *> gg::MCollisionResolver::getDeleted()
{
    return std::move(m_toDelete);
}

void gg::MCollisionResolver::resolveCollision(MObject *, btVector3 &, MObject *, btVector3 &)
{

}

void gg::MCollisionResolver::resolveAll()
{
    int numManifolds = m_btWorld->getDispatcher()->getNumManifolds();
    //For each contact manifold
    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold* contactManifold = m_btWorld->getDispatcher()->getManifoldByIndexInternal(i);
        const btRigidBody* obA = static_cast<const btRigidBody*>(contactManifold->getBody0());
        const btRigidBody* obB = static_cast<const btRigidBody*>(contactManifold->getBody1());
        contactManifold->refreshContactPoints(obA->getWorldTransform(), obB->getWorldTransform());

        btManifoldPoint& pt = contactManifold->getContactPoint(0);
        btVector3 ptA = pt.getPositionWorldOnA();
        btVector3 ptB = pt.getPositionWorldOnB();
        btVector3& normalOnB = pt.m_normalWorldOnB;

        MObject* objectA = static_cast<MObject*>(obA->getUserPointer());
        MObject* objectB = static_cast<MObject*>(obB->getUserPointer());

        resolveCollision(objectA, ptA, objectB, ptB);

    }
}

