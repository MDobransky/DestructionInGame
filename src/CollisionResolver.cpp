#include "CollisionResolver.h"
#include "MeshManipulators.h"

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

gg::MCollisionResolver::MCollisionResolver(IrrlichtDevice *irrDev, btDiscreteDynamicsWorld *btDDW, std::vector<std::unique_ptr<MObject>>* objs) : m_irrDevice(irrDev), m_btWorld(btDDW), m_objects(objs)
{
}

gg::MCollisionResolver::~MCollisionResolver()
{

}

std::vector<gg::MObject *> gg::MCollisionResolver::getDeleted()
{
    return std::move(m_toDelete);
}

void gg::MCollisionResolver::resolveCollision(MObject* obj, btVector3 point, btVector3 from, btScalar impulse)
{
    const MMaterial* material = obj->getMaterial();
    if(material == &MMaterial::Magic)
    {
        //nothing
    }
    else if(material == &MMaterial::Shot && impulse > 20)
    {
        m_btWorld->removeCollisionObject(obj->getRigid());
        obj->removeNode();
        obj->setDeleted();
    }
    else if(material != &MMaterial::Ship)
    {
        if(impulse > 20) //get material property
        {
            //push event to stack, generate dust to cover
            //take from stack, use voro++ to create convex shape
            //subtract shape from mesh and replace it
            //cut convex shape to bits and put them to world
            generateVoro(obj,point,from,4);
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
            //m_btWorld->removeCollisionObject(obj->getRigid());
            //obj->removeNode();
            //obj->setDeleted();
        }
    }
}

void gg::MCollisionResolver::generateVoro(gg::MObject* obj, btVector3 point, btVector3 from, btScalar size)
{
using namespace voro;
    float cube_size = size + size*0.2f;
    container con(point.getX()-cube_size,point.getX()+cube_size,
                  point.getY()-cube_size,point.getY()+cube_size,
                  point.getZ()-cube_size,point.getZ()+cube_size,
                  8,8,8,
                  false,false,false,
                  8);
/*
    wall_cylinder cyl(point.getX(),point.getY(),point.getZ(),
                      from.getX(),from.getY(),from.getZ(),
                      size);
    con.add_wall(cyl);*/
/*
    wall_custom body_wall(this,obj->getRigid(),from);
    con.add_wall(body_wall);
*/
    /*for(int i = 1; i < 10; i++)
    {
        int x = rand()%int(2*cube_size)+point.getX()-cube_size;
        int y = rand()%int(2*cube_size)+point.getY()-cube_size;
        int z = rand()%int(2*cube_size)+point.getZ()-cube_size;
        if(con.point_inside(x,y,z))
        {
            con.put(i,x,y,z);
            break;
        }
    }*/
    con.put(0,point.getX(),point.getY(),point.getZ());
    c_loop_all loop(con);
    loop.start();
    voronoicell c;
    //do
    {
        //if(con.compute_cell(c,loop) && (btVector3(loop.x(),loop.y(),loop.z()) - point).length() > size/3)
        {
            voronoicell c;
            con.compute_cell(c,loop);
            //c.init_base(-2,2,-2,2,-2,2);
            IMesh* mesh = gg::MeshManipulators::createMesh(c);
           /* btTransform Transform;
            Transform.setIdentity();
            Transform.setOrigin(btVector3(loop.x(),loop.y(),loop.z()));
            btRigidBody *RigidBody = new btRigidBody(0, new btDefaultMotionState(Transform), new btBvhTriangleMeshShape(btMesh, false), btVector3());
            //m_btWorld->addRigidBody(RigidBody);
*/


            IMeshSceneNode* Node = m_irrDevice->getSceneManager()->addMeshSceneNode(mesh);
            Node->setPosition(vector3df(loop.x(),loop.y(),loop.z()));
           // MObject* fragment = new MObject(RigidBody, Node, &MMaterial::Magic);
    Node->setVisible(0);
            //RigidBody->setUserPointer((void *)(fragment));
            //m_objects->push_back(std::unique_ptr<gg::MObject>(fragment));

            try
            {
                IMesh* new_mesh = MeshManipulators::subtractMesh(static_cast<IMeshSceneNode*>(obj->getNode())->getMesh(), mesh, Node->getPosition() - obj->getNode()->getPosition());

                if(new_mesh)
                {
                    Node = static_cast<IMeshSceneNode*>(obj->getNode());
                    Node->setMesh(new_mesh);
                    Node->setMaterialType(EMT_SOLID);
                    Node->setMaterialFlag(EMF_LIGHTING, 0);
                    Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
                    btRigidBody* body = obj->getRigid();
                    delete body->getCollisionShape();
                    btCollisionShape *Shape = MeshManipulators::convertMesh(Node);
                    Shape->setMargin( 0.05f );
                    body->setCollisionShape(Shape);
                }
                else
                {
                    m_btWorld->removeCollisionObject(obj->getRigid());
                    obj->removeNode();
                    obj->setDeleted();
                }
            }
            catch(...)
            {
                std::cout << "FAILED\n";
            }
        }
    } //while (loop.inc());
}

bool gg::MCollisionResolver::isInside(btRigidBody* body, btVector3 point,btVector3 from)
{
    int in = 0;
    btVector3 last = from;

    btCollisionWorld::AllHitsRayResultCallback clbck(from, point); //entering body
    m_btWorld->rayTest(from, point, clbck);
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

            btScalar impulse = pt.getAppliedImpulse();
            if(obA->getUserPointer())
            {
                MObject* objectA = static_cast<MObject*>(obA->getUserPointer());
                if(!objectA->isDeleted())
                {
                    resolveCollision(objectA, ptA, ptA-ptB, impulse);
                }
            }
            if(obB->getUserPointer())
            {
                MObject* objectB = static_cast<MObject*>(obB->getUserPointer());
                if(!objectB->isDeleted())
                {
                    resolveCollision(objectB, ptB, ptB-ptA, impulse);
                }
            }
        }
    }
}

