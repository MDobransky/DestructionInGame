//creates objects either by loading them or by gluing together edem elements (but without new joints)

#ifndef OBJECT_H
#define OBJECT_H
#include "Edem.h"
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
    ISceneNode* m_irrSceneNode;
    std::vector<MEdem> m_edems;
    bool m_empty;
    MMaterial* m_material;
public:
    btRigidBody* getRigid() { return m_rigidBody.get(); }
    ISceneNode* getNode() { return m_irrSceneNode; }
    bool isEmpty() { return m_empty; }

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
    MObject () : m_empty(true) {}

    MObject (btRigidBody* rb, ISceneNode* sn) :
        m_rigidBody(std::unique_ptr<btRigidBody>(rb)),
        m_irrSceneNode(sn)
    {
        m_empty = m_rigidBody == nullptr;
    }


    MObject (MObject&& newObj)
    {
        m_rigidBody = std::move(newObj.m_rigidBody);
        m_edems = std::move(newObj.m_edems);
        m_irrSceneNode = std::move(newObj.m_irrSceneNode);
        m_material = &Material::Steel;
    }

    MObject (MObject&) = delete;
    MObject& operator= (MObject&& newObj) = delete;
    MObject&  operator= (const MObject&) = delete;
};

}
#endif
