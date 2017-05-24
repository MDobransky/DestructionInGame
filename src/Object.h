//creates objects either by loading them or by gluing together edem elements (but without new joints)

#ifndef OBJECT_H
#define OBJECT_H

#include "MeshManipulators.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

#include <vector>
#include <memory>
#include <iostream>
#include <mutex>
#include <atomic>

namespace gg
{

    class MObject
    {
        typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
        typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
        typedef Polyhedron::HalfedgeDS HalfedgeDS;
        typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron;

    public:
        inline btRigidBody *getRigid()
        { return m_rigidBody.get(); }

        inline irr::scene::ISceneNode *getNode()
        { return m_irrSceneNode; }

        enum class Material
        {
            BUILDING, DEBREE, SHIP, SHOT, GROUND, DUST
        };
        std::mutex m_mutex;
        std::atomic_int version;

        inline Material getMaterial()
        { return m_material; }

        inline bool isEmpty()
        { return m_empty; }

        inline bool isDeleted()
        { return m_deleted; }

        inline void setDeleted()
        { m_deleted = true; }

        inline bool isMesh()
        { return m_isMesh; }

        inline Nef_polyhedron &getPolyhedron()
        { return m_polyhedron; }

        inline void setPolyhedron(Nef_polyhedron nef)
        {
            m_polyhedron = std::move(nef);
            m_isMesh = true;
        }

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
                {
                    delete m_rigidBody->getMotionState();
                }
                if(m_rigidBody->getCollisionShape() != nullptr)
                {
                    delete m_rigidBody->getCollisionShape();
                }
            }
        }

        MObject() : m_empty(true), m_deleted(false)
        {}

        MObject(btRigidBody *rb, irr::scene::ISceneNode *sn, bool mesh = false) : m_rigidBody(
                std::unique_ptr<btRigidBody>(rb)), m_irrSceneNode(sn), m_isMesh(mesh)
        {
            m_empty = m_rigidBody == nullptr;
            m_deleted = false;
            if(m_isMesh)
            {
                m_polyhedron = std::move(
                        MeshManipulators::makeNefPolyhedron(static_cast<irr::scene::IMeshSceneNode *>(sn)->getMesh()));
            }
            version.store(0);
        }

        MObject(btRigidBody *rb, irr::scene::ISceneNode *sn, Material mat, bool mesh = true) : m_rigidBody(
                std::unique_ptr<btRigidBody>(rb)), m_irrSceneNode(sn), m_material(mat), m_isMesh(mesh)
        {
            m_empty = m_rigidBody == nullptr;
            m_deleted = false;
            if(m_isMesh)
            {
                m_polyhedron = std::move(
                        MeshManipulators::makeNefPolyhedron(static_cast<irr::scene::IMeshSceneNode *>(sn)->getMesh()));
            }
            version.store(0);
        }

        MObject(btRigidBody *rb, irr::scene::ISceneNode *sn, Material mat, Nef_polyhedron &&nef) : m_rigidBody(
                std::unique_ptr<btRigidBody>(rb)), m_irrSceneNode(sn), m_material(mat), m_polyhedron(nef)
        {
            m_empty = m_rigidBody == nullptr;
            m_deleted = false;
            m_isMesh = true;
            version.store(0);
        }

        MObject(MObject &&other)
        {
            m_rigidBody = std::move(other.m_rigidBody);
            m_irrSceneNode = other.m_irrSceneNode;
            m_material = other.m_material;
            m_deleted = other.m_deleted;
            m_polyhedron = std::move(other.m_polyhedron);
            version.store(0);
        }

        MObject(MObject &) = delete;

        MObject &operator=(MObject &&) = delete;

        MObject &operator=(const MObject &) = delete;

    private:
        std::unique_ptr<btRigidBody> m_rigidBody;
        irr::scene::ISceneNode *m_irrSceneNode;
        bool m_empty, m_deleted = false;
        Material m_material;
        bool m_isMesh = false;
        Nef_polyhedron m_polyhedron;
    };

}
#endif
