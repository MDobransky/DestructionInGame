#include "CollisionResolver.h"
#include <cmath>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


#define debug
#ifdef debug
std::ostream& operator<<(std::ostream& os, const irr::core::vector3df& v)
{
    os << "(" << v.X << " " << v.Y << " " << v.Z << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const btVector3& v)
{
    os << "(" << v.getX() << " " << v.getY() << " " << v.getZ() << ")";
    return os;
}
#endif

gg::MCollisionResolver::MCollisionResolver(IrrlichtDevice* irrDev, btDiscreteDynamicsWorld* btDDW,
                                           MObjectCreator* creator, std::vector<std::unique_ptr<MObject>>* objs)
        : m_irrDevice(irrDev),
          m_btWorld(btDDW),
          m_objectCreator(creator),
          m_objects(objs)
{
    m_done.store(false);
    m_subtractor = std::move(std::thread([this] { meshSubtractor(); }));
    m_decomposer = std::move(std::thread([this] { meshDecomposer(); }));
}

gg::MCollisionResolver::~MCollisionResolver()
{
    m_done.store(true);
    m_subtractionCondVar.notify_one();
    m_decompositionCondVar.notify_one();
    m_subtractor.join();
    m_decomposer.join();
}

void gg::MCollisionResolver::resolveCollision(MObject* obj, btVector3 point,
                                              btScalar impulse, MObject* other)
{
    //std::lock_guard<std::mutex> objlock(obj->m_mutex);
    //std::lock_guard<std::mutex> otherlock(other->m_mutex);
    MObject::Material material = obj->getMaterial();
    if(other->deleted)
    {
        return;
    }
    else if(material == MObject::Material::GROUND)
    {
        return;
    }
    else if(material != MObject::Material::SHIP && impulse > 0 )
    {
        if(other->getMaterial() == MObject::Material::SHOT)
        {
            m_btWorld->removeCollisionObject(other->getRigid());
            other->deleted = true;
        }
        if(obj->isMesh() && ((other->getMaterial() != MObject::Material::GROUND && impulse > 100)|| impulse > 200 || other->getMaterial() == MObject::Material::SHOT))
        {
            using namespace voro;
            float cube_size = impulse/10.f;
            vector3df cube_min(-cube_size, -cube_size, -cube_size);
            vector3df cube_max(cube_size, cube_size, cube_size);

            container con(cube_min.X, cube_max.X,
                          cube_min.Y, cube_max.Y,
                          cube_min.Z, cube_max.Z,
                          8,8,8,
                          false,false,false,
                          8);

            con.put(0,0,0,0);
            for(auto&& i : {1,2})
            {
                con.put(i,std::fmod(rand(),cube_size*2) + cube_min.X, std::fmod(rand(),cube_size*2) + cube_min.Y, std::fmod(rand(),cube_size*2) + cube_min.Z);
            }

            c_loop_all loop(con);
            loop.start();
            voronoicell c;
            con.compute_cell(c,loop);
            IMesh* debree_mesh = gg::MeshManipulators::convertMesh(c);

            std::lock_guard<std::mutex> lock (m_subtractionTasksMutex);
            vector3df relative_position(vector3df(point.x(),point.y(),point.z()) - obj->getNode()->getPosition());
            obj->reference_count++;
            m_subtractionTasks.push_back(std::make_tuple(obj, debree_mesh, relative_position));
            m_subtractionCondVar.notify_one();
        }
    }


}

void gg::MCollisionResolver::meshSubtractor()
{
    MObject* obj;
    IMesh* mesh;
    vector3df position;
    while(!m_done)
    {
        std::unique_lock<std::mutex> taskLock(m_subtractionTasksMutex);
        m_subtractionCondVar.wait(taskLock, [this]() { return !m_subtractionTasks.empty() || m_done;});
        if(m_subtractionTasks.size() > 0)
        {
            std::tie(obj, mesh, position) = m_subtractionTasks.front();
            m_subtractionTasks.pop_front();
            taskLock.unlock();

            if(obj->deleted)
            {
                return;
            }
            int old_version = obj->version.load();
            try
            {
                quaternion quat(obj->getNode()->getRelativeTransformation());
                quat.makeInverse();
                position = quat * position;
                quat = obj->getPolyhedronTransform();
                position = quat.makeInverse() * position + obj->translation;

                MeshManipulators::Nef_polyhedron newPoly, debree;
                {
                    std::lock_guard<std::mutex> objlock(obj->m_mutex);
                    std::tie(newPoly, debree) = MeshManipulators::subtractMesh(obj->getPolyhedron(), mesh, position);
                }
                std::vector<MeshManipulators::Nef_polyhedron> newNefPolyhedrons(std::move(MeshManipulators::splitPolyhedron(std::move(newPoly))));
                std::vector<MeshManipulators::Nef_polyhedron> debreeVector(std::move(MeshManipulators::splitPolyhedron(std::move(debree))));
                newNefPolyhedrons.insert(newNefPolyhedrons.end(), debreeVector.begin(), debreeVector.end());
                for(size_t i = 0; i < newNefPolyhedrons.size(); i++)
                {
                    IMesh* new_mesh;
                    vector3df center;
                    std::tie(new_mesh, center) = MeshManipulators::convertPolyToMesh(newNefPolyhedrons[i]);
                    std::lock_guard<std::mutex> resLock(m_subtractionResultsMutex);
                    if(i == 0)
                    {
                        obj->reference_count++;
                        m_subtractionResults.push(std::make_tuple(obj,
                                                                  obj->getRigid()->getCenterOfMassPosition(),
                                                                  obj->getRigid()->getOrientation(),
                                                                  std::move(newNefPolyhedrons[0]),
                                                                  new_mesh,
                                                                  old_version));
                    }
                    else
                    {
                        vector3df newPosition;
                        newPosition = quaternion(obj->getNode()->getRelativeTransformation()) * center;
                        newPosition += obj->getNode()->getPosition();
                        MObject* newObj = new MObject(NULL, NULL, obj->getMaterial(), false);
                        newObj->translation = center;
                        newObj->reference_count++;
                        m_subtractionResults.push(
                                std::make_tuple(newObj,
                                                btVector3(newPosition.X, newPosition.Y, newPosition.Z),
                                                obj->getRigid()->getOrientation(),
                                                std::move(newNefPolyhedrons[i]),
                                                new_mesh,
                                                old_version));
                    }
                }
            }
            catch(...)
            {
                std::cout << "FAILED\n";
            }
            obj->reference_count--;
        }
    }
}

void gg::MCollisionResolver::meshDecomposer()
{
    MObject* obj;
    IMesh* mesh;

    while(!m_done)
    {
        std::unique_lock<std::mutex> taskLock(m_decompositionTasksMutex);
        m_decompositionCondVar.wait(taskLock, [this]() { return !m_decompositionTasks.empty() || m_done;});
        if(m_decompositionTasks.size() > 0)
        {
            std::tie(obj, mesh) = m_decompositionTasks.front();
            m_decompositionTasks.pop();
            taskLock.unlock();
            btCollisionShape *shape = new btHACDCompoundShape(MeshManipulators::convertMesh(mesh));
            shape->setMargin(0.01f);
            std::lock_guard<std::mutex> resLock(m_decompositionResultsMutex);
            m_decompositionResults.push(std::make_tuple(obj, shape));
        }
    }
}

void gg::MCollisionResolver::subtractionApplier()
{

    MObject* obj = NULL;
    IMesh* new_mesh = NULL;
    btVector3 position;
    int old_version;
    btQuaternion rotation;
    MeshManipulators::Nef_polyhedron newPoly;
    {
        std::lock_guard<std::mutex> resLock(m_subtractionResultsMutex);
        if(m_subtractionResults.size() > 0)
        {
            std::tie(obj, position, rotation, newPoly, new_mesh, old_version) = m_subtractionResults.front();
            m_subtractionResults.pop();
        }
    }
    if(obj && new_mesh)
    {
        if(obj->version > old_version)
        {
            return;
        }
        {
            std::lock_guard<std::mutex> objLock(obj->m_mutex);
            obj->setPolyhedron(std::move(newPoly));
        }
        if(obj->getRigid())
        {
            IMeshSceneNode* Node = static_cast<IMeshSceneNode*>(obj->getNode());
            Node->setMesh(new_mesh);
            Node->setMaterialType(EMT_SOLID);
            Node->setMaterialFlag(EMF_LIGHTING, 0);
            Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
            obj->version++;
        }
        else
        {
            std::unique_ptr<MObject> object(m_objectCreator->createMeshRigidBodyWithTmpShape(new_mesh, position, 10, obj->getMaterial(), std::move(newPoly)));
            object->reference_count.store(obj->reference_count);
            object->translation = obj->translation;
            delete obj;
            obj = object.get();
            btTransform tr(rotation);
            tr.setOrigin(position);
            obj->getRigid()->setWorldTransform(tr);
            m_objects->push_back(std::move(object));
            m_btWorld->addRigidBody(obj->getRigid());
        }
        std::unique_lock<std::mutex> taskLock(m_decompositionTasksMutex);
        m_decompositionTasks.push(std::make_tuple(obj, new_mesh));
        m_decompositionCondVar.notify_one();
    }
    else if (obj)
    {
        obj->deleted = true;
    }
}

void gg::MCollisionResolver::decompositionApplier()
{
    std::lock_guard<std::mutex> resLock(m_decompositionResultsMutex);
    while(m_decompositionResults.size() > 0)
    {
        MObject* obj = NULL;
        btCollisionShape* new_shape = NULL;
        std::tie(obj, new_shape) = m_decompositionResults.front();
        m_decompositionResults.pop();
        if(obj && new_shape)
        {
            delete obj->getRigid()->getCollisionShape();
            obj->getRigid()->setCollisionShape(new_shape);
            obj->reference_count--;
        }
    }
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
            btScalar impulse = pt.getAppliedImpulse();

            MObject* objectA, *objectB;
            objectA = static_cast<MObject*>(obA->getUserPointer());
            objectB = static_cast<MObject*>(obB->getUserPointer());

            if(obA->getUserPointer())
            {
                objectA = static_cast<MObject*>(obA->getUserPointer());
            }
            if(obB->getUserPointer())
            {
                objectB = static_cast<MObject*>(obB->getUserPointer());
            }

            if(!objectA->isDeleted())
            {
                resolveCollision(objectA, ptA, impulse, objectB);
            }
            if(!objectB->isDeleted())
            {
                resolveCollision(objectB, ptB, impulse, objectA);
            }
        }
    }
    subtractionApplier();
    decompositionApplier();
}

//DUST GENERATOR
/*    scene::IParticleSystemSceneNode* ps =
    m_irrDevice->getSceneManager()->addParticleSystemSceneNode(false);

    scene::IParticleEmitter* em = ps->createSphereEmitter(
        vector3df(point.getX(),point.getY(),point.getZ()),
        2.f,
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
