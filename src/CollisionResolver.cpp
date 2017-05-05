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

void gg::MCollisionResolver::resolveCollision(MObject* obj, btVector3 point, btVector3 from, btScalar impulse)
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
        if(impulse > 50) //get material property
        {
            generateVoro(obj,point,from,3);
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
            m_btWorld->removeCollisionObject(obj->getRigid());
            obj->removeNode();
            obj->setDeleted();
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
                  int(size),int(size),int(size),
                  false,false,false,
                  8);

    wall_cylinder cyl(point.getX(),point.getY(),point.getZ(),
                      from.getX(),from.getY(),from.getZ(),
                      size);
    con.add_wall(cyl);

    wall_custom body_wall(this,obj->getRigid(),from);
    con.add_wall(body_wall);

    for(int i = 0; i < 100; i++)
    {
        int x = rand()%int(2*cube_size)+point.getX()-cube_size;
        int y = rand()%int(2*cube_size)+point.getY()-cube_size;
        int z = rand()%int(2*cube_size)+point.getZ()-cube_size;
        if(con.point_inside(x,y,z))
        {
            con.put(i,x,y,z);
        }

    }
    c_loop_all loop(con);
    loop.start();
    voronoicell c;
    std::vector<double> vertices;
    do if(con.compute_cell(c,loop))
    {
        c.vertices(vertices);
        std::cout << vertices.size()/3 << "\n";
        std::cout << c.number_of_faces() << "\n";
        std::cout << c.number_of_edges() << "\n";
    } while (loop.inc());
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
   /* if(in%2)
    {
        ISceneNode* node = m_irrDevice->getSceneManager()->addSphereSceneNode();
        btVector3 spot = body->getWorldTransform() * point;
        node->setPosition(vector3df(spot.getX(),spot.getY(),spot.getZ()));
        std::cout << "yes\n";
    }*/
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
            MObject* objectA = static_cast<MObject*>(obA->getUserPointer());
            MObject* objectB = static_cast<MObject*>(obB->getUserPointer());
            //btVector3 from = pt.m_normalWorldOnB;
            if(!objectA->isDeleted())
            {
                resolveCollision(objectA, ptA, ptA-ptB, impulse);
            }
            if(!objectB->isDeleted())
            {
                resolveCollision(objectB, ptB, ptB-ptA, impulse);
            }
        }
    }
}

