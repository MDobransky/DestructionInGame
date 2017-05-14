//creates objects either by loading them or by gluing together edem elements (but without new joints)

#ifndef OBJECT_H
#define OBJECT_H

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <vector>
#include <memory>
#include <iostream>
#include <atomic>
#include <mutex>

namespace gg {

class MObject
{
public:
    inline btRigidBody* getRigid() { return m_rigidBody.get(); }
    inline irr::scene::ISceneNode* getNode() { return m_irrSceneNode; }
    enum class Material {BUILDING, DEBREE, SHIP, SHOT, GROUND, DUST};
    std::atomic<int> m_version;
    std::mutex m_mutex;
    inline Material getMaterial() { return m_material; }
    inline bool isEmpty() { return m_empty; }
    inline bool isDeleted() { return m_deleted; }
    inline void setDeleted() { m_deleted = true; }
    inline void removeNode()
    {
        if(m_irrSceneNode)
        {
            m_irrSceneNode->remove();
            m_irrSceneNode = 0;
        }
    }

    ~MObject()
    {
        if(m_rigidBody.get() != nullptr)
        {
            if(m_rigidBody->getMotionState() != nullptr)
                delete m_rigidBody->getMotionState();
            if(m_rigidBody->getCollisionShape() != nullptr)
                delete m_rigidBody->getCollisionShape();
        }
    }

    MObject () : m_empty(true), m_deleted(false) {}

    MObject (btRigidBody* rb, irr::scene::ISceneNode* sn) :
        m_rigidBody(std::unique_ptr<btRigidBody>(rb)),
        m_irrSceneNode(sn)
    {
        m_empty = m_rigidBody == nullptr;
        m_deleted = false;
        m_version.store(0);
    }

    MObject (btRigidBody* rb, irr::scene::ISceneNode* sn, Material mat) :
        m_rigidBody(std::unique_ptr<btRigidBody>(rb)),
        m_irrSceneNode(sn),
        m_material(mat)
    {
        m_empty = m_rigidBody == nullptr;
        m_deleted = false;
        m_version.store(0);
    }

    MObject (MObject&& newObj)
    {
        m_rigidBody = std::move(newObj.m_rigidBody);
        m_irrSceneNode = newObj.m_irrSceneNode;
        m_material = newObj.m_material;
        m_deleted = newObj.m_deleted;
        m_version.store(0);
    }

    MObject (MObject&) = delete;
    MObject& operator= (MObject&&) = delete;
    MObject&  operator= (const MObject&) = delete;

private:
    std::unique_ptr<btRigidBody> m_rigidBody;
    irr::scene::ISceneNode* m_irrSceneNode;
    bool m_empty, m_deleted = false;
    Material m_material;
};

}
#endif
