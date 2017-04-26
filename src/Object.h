//creates objects either by loading them or by gluing together edem elements (but without new joints)

#ifndef OBJECT_H
#define OBJECT_H
#include "Material.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <vector>
#include <memory>
#include <iostream>

namespace gg {

class MObject
{
private:
    std::unique_ptr<btRigidBody> m_rigidBody;
    irr::scene::ISceneNode* m_irrSceneNode;
    bool m_empty, m_deleted = false;
    const MMaterial* m_material;
public:
    btRigidBody* getRigid() { return m_rigidBody.get(); }
    irr::scene::ISceneNode* getNode() { return m_irrSceneNode; }
    const MMaterial* getMaterial() { return m_material; }
    bool isEmpty() { return m_empty; }
    bool isDeleted() { return m_deleted; }
    void setDeleted() { m_deleted = true; }

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
    }

    MObject (btRigidBody* rb, irr::scene::ISceneNode* sn, const MMaterial* mat) :
        m_rigidBody(std::unique_ptr<btRigidBody>(rb)),
        m_irrSceneNode(sn),
        m_material(mat)
    {
        m_empty = m_rigidBody == nullptr;
        m_deleted = false;
    }

    MObject (MObject&& newObj)
    {
        m_rigidBody = std::move(newObj.m_rigidBody);
        m_irrSceneNode = newObj.m_irrSceneNode;
        m_material = newObj.m_material;
        m_deleted = newObj.m_deleted;
    }

    MObject (MObject&) = delete;
    MObject& operator= (MObject&&) = delete;
    MObject&  operator= (const MObject&) = delete;
};

}
#endif
