#include "CollisionResolver.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

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

void gg::MCollisionResolver::resolveCollision(MObject * obj, btVector3 & pA, btScalar & impulse)
{
    const MMaterial* material = obj->getMaterial();
    if(material == &MMaterial::Magic)
    {
        //nothing
    }
    else if(material == &MMaterial::Shot && impulse > 100)
    {
        m_btWorld->removeCollisionObject(obj->getRigid());
        obj->removeNode();
        obj->setDeleted();
    }
    else if(material != &MMaterial::Ship)
    {
        if(impulse > 100) //get material property
        {
/*

            scene::IParticleSystemSceneNode* ps =
            m_irrDevice->getSceneManager()->addParticleSystemSceneNode(false);

            scene::IParticleEmitter* em = ps->createSphereEmitter(
                vector3df(0,0,0),
                5.f,
                core::vector3df(0.0f,0.0f,0.0f),   // initial direction
                1000,10000,                             // emit rate
                video::SColor(0,0,0,0),       // darkest color
                video::SColor(0,100,100,100),       // brightest color
                1000,2000,0,                         // min and max age, angle
                core::dimension2df(1,1),         // min size
                core::dimension2df(3.f,3.f));        // max size

            ps->setEmitter(em); // this grabs the emitter
            em->drop(); // so we can drop it here without deleting it

            scene::IParticleAffector* paf = ps->createFadeOutParticleAffector();

            ps->addAffector(paf); // same goes for the affector
            paf->drop();

            ps->setPosition(obj->getNode()->getAbsolutePosition());
            ps->setScale(core::vector3df(1,1,1));
            ps->setMaterialFlag(video::EMF_LIGHTING, false);
            ps->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
            ps->setMaterialTexture(0, m_irrDevice->getVideoDriver()->getTexture("media/dust2.png"));
            ps->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
            ps->doParticleSystem(1000);
*/
            m_btWorld->removeCollisionObject(obj->getRigid());
            obj->removeNode();
            obj->setDeleted();
        }
    }
}
#include <iomanip>
#include <limits>
bool gg::MCollisionResolver::isInside(btRigidBody* body, btVector3& point)
{
//TODO find better starting point, check only one object is crossed
    btVector3 heaven = body->getCenterOfMassPosition() + btVector3(0,1000,0); //must be outside body and (heaven, point) can't cross any other body
    int in = 0, out = 0;
    btVector3 last = heaven;

    btCollisionWorld::AllHitsRayResultCallback clbck(heaven, point); //entering body
    m_btWorld->rayTest(heaven, point, clbck);
    for(int i = 0; i < clbck.m_hitPointWorld.size(); i++)
    {
        if(int(last.getX()*100) != int(clbck.m_hitPointWorld[i].getX()*100) || //this can fail on extra thin walls
           int(last.getY()*100) != int(clbck.m_hitPointWorld[i].getY()*100) || //we have to filter almost equal positions because edges and vertices count multiple times
           int(last.getZ()*100) != int(clbck.m_hitPointWorld[i].getZ()*100))
        {
            last = clbck.m_hitPointWorld[i];
            in++;
        }
    }
    return in%2;
}

void gg::MCollisionResolver::resolveAll()
{

    int numManifolds = m_btWorld->getDispatcher()->getNumManifolds();
    //For each contact manifold
    for (int i = 0; i < numManifolds; i++)
    {
        btPersistentManifold* contactManifold = m_btWorld->getDispatcher()->getManifoldByIndexInternal(i);
        if(contactManifold->getBody0() && contactManifold->getBody1())
        {
            const btRigidBody* obA = static_cast<const btRigidBody*>(contactManifold->getBody0());
            const btRigidBody* obB = static_cast<const btRigidBody*>(contactManifold->getBody1());
            contactManifold->refreshContactPoints(obA->getWorldTransform(), obB->getWorldTransform());
            btManifoldPoint& pt = contactManifold->getContactPoint(0);
            btVector3 ptA = pt.getPositionWorldOnA();
            btVector3 ptB = pt.getPositionWorldOnB();
            //btVector3& normalOnB = pt.m_normalWorldOnB;
            btScalar impulse = pt.getAppliedImpulse();
            MObject* objectA = static_cast<MObject*>(obA->getUserPointer());
            MObject* objectB = static_cast<MObject*>(obB->getUserPointer());
            if(!objectA->isDeleted())
            {
                resolveCollision(objectA, ptA, impulse);
            }
            if(!objectB->isDeleted())
            {
                resolveCollision(objectB, ptB, impulse);
            }
        }
    }
}

