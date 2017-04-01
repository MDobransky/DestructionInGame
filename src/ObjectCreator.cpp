#include "ObjectCreator.h"
#include "Material.h"

#include <iostream>
#include <cmath>
#include <map>

#include "tetgen/tetgen.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

gg::MObjectCreator::MObjectCreator(IrrlichtDevice* irr) : m_irrDevice(irr)
{
}

std::tuple<std::vector<gg::MObject*>,std::vector<btFixedConstraint*>>  gg::MObjectCreator::createDestructibleBody(std::vector<std::string>&& items, ISceneNode *parent)
{
    std::vector<MObject*> objects;
    std::vector<btFixedConstraint*> constraints;
    if(items.size() != 13)
    {
        std::cerr << "Object failed to load: wrong number of parameters\n";
        return std::make_tuple(objects,constraints);
    }

    float numbers[10];
    for(int i = 0; i < 10; i++)
    {
        numbers[i] = std::stof(items[i+3]);
    }

    std::string input(m_media+items[0]);
    //std::string texture(items[1]);
    btVector3 position(numbers[0],numbers[1],numbers[2]);
    core::vector3df rotation(numbers[3],numbers[4],numbers[5]);
    core::vector3df scale(numbers[6],numbers[7],numbers[8]);
    btScalar Mass = numbers[9];
//GENERATE TETRAHEDRONS
    tetgenio in,out;
    in.firstnumber = 0;
    in.load_ply(const_cast<char*>(input.c_str()));
    char* opt = const_cast<char*>(std::string("pnCa0.1").c_str());
    tetrahedralize(opt, &in, &out);

//make tetrahedron structures
    std::vector<tetrahedron> shards;

    for(int i = out.firstnumber; i < out.numberofcorners*out.numberoftetrahedra; i += out.numberofcorners)
    {
        btVector3 center(0,0,0);
        shards.push_back(tetrahedron());
        for(int j = 0; j < 4; j++)
        {
            btVector3 point(out.pointlist[out.tetrahedronlist[i+j]*3],
                                   out.pointlist[out.tetrahedronlist[i+j]*3+1],
                                   out.pointlist[out.tetrahedronlist[i+j]*3+2]);
            center += point;
            shards.back().points[j] = point;
            shards.back().neighbours[j] = out.neighborlist[i+j];
        }
        shards.back().center = btVector3(center/4);
    }
//make objects
    for(int i = 0; i < out.numberoftetrahedra; i++)
    {
    //generate irrlicht mesh
        SMesh* mesh = new SMesh();
        SMeshBuffer *buf = 0;
        buf = new SMeshBuffer();
        mesh->addMeshBuffer(buf);
        buf->drop();

        buf->Vertices.reallocate(4);
        buf->Vertices.set_used(4);
        buf->Indices.reallocate(12);
        buf->Indices.set_used(12);

        for(int j = 0; j < 4; j++)
        {
            buf->Vertices[j] = S3DVertex(float(shards[i].points[j].getX()),
                                         float(shards[i].points[j].getY()),
                                         float(shards[i].points[j].getZ()),
                                         shards[i].center.getX(),
                                         shards[i].center.getY(),
                                         shards[i].center.getZ(),
                                         video::SColor(100,100,100,100), 0, 1);
        }
        int indices[] = {1,0,2,1,3,0,1,2,3,0,3,2};
        for(int j = 0; j < 12; j++)
        {
            buf->Indices[j] = indices[j];
        }
        buf->recalculateBoundingBox();

        m_irrDevice->getSceneManager()->getMeshManipulator()->scale(mesh,scale);
        IMeshSceneNode* Node = m_irrDevice->getSceneManager()->addMeshSceneNode(mesh);

        Node->setMaterialType(EMT_SOLID);
        Node->setMaterialFlag(EMF_LIGHTING, 0);
        Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
        m_irrDevice->getSceneManager()->getMeshManipulator()->setVertexColors(Node->getMesh(), SColor(0,rand()%256,rand()%256,rand()%256));

    //generate bullet body
        // Set the initial position of the object
        btTransform Transform;
        Transform.setIdentity();
        Transform.setOrigin(position);
        btDefaultMotionState *motionState = new btDefaultMotionState(Transform);

        // Create the shape
        btCollisionShape *Shape = MObjectCreator::convertMeshToHull(Node);
        Shape->setMargin( 0.05f );

        // Add mass
        btVector3 localInertia;
        Shape->calculateLocalInertia(Mass/shards.size(), localInertia);

        // Create the rigid body object
        btRigidBody *rigidBody = new btRigidBody(Mass/shards.size(), motionState, Shape, localInertia);

        MMaterial* material = Material::getMaterial(items[2][0]);

        MObject* obj = new MObject(rigidBody, Node, material);

        rigidBody->setUserPointer((void *)(obj));
        objects.push_back(obj);
    }

//create constraints

    for(int a = 0; a < shards.size(); a++)
    {
        for(int j = 0; j < 4; j++)
        {
            int b = shards[a].neighbours[j];
            if(b != -1)
            {
                btRigidBody* bodyA = objects[a]->getRigid();
                btRigidBody* bodyB = objects[b]->getRigid();
                btTransform tr;
                tr.setIdentity();
                //btVector3 contactPosWorld = manifold->getContactPoint(minIndex).m_positionWorldOnA;
                //tr.setOrigin(contactPosWorld);
                //tr = bodyA->getWorldTransform().inverse()*tr;
                btFixedConstraint* fixed = new btFixedConstraint(*bodyA,*bodyB,tr,tr);
                fixed->setBreakingImpulseThreshold(20000);
                for(int i = 0; i < 4; i++)
                {
                    if(shards[b].neighbours[i] == a) {
                        shards[b].neighbours[i] = -1;
                    }
                }
                constraints.push_back(fixed);
            }
        }
    }

    return std::move(std::make_tuple(std::move(objects),constraints));
}
gg::MObject* gg::MObjectCreator::createMeshRigidBody(std::vector<std::string>&& items, ISceneNode *parent)
{
    if(items.size() != 13)
    {
        std::cerr << "Object failed to load: wrong number of parameters\n";
        return new MObject();
    }

    float numbers[10];
    for(int i = 0; i < 10; i++)
    {
        numbers[i] = std::stof(items[i+3]);
    }

    std::string input(items[0]);
    std::string texture(items[1]);
    //char material(items[2][0]);
    btVector3 position(numbers[0],numbers[1],numbers[2]);
    core::vector3df rotation(numbers[3],numbers[4],numbers[5]);
    core::vector3df scale(numbers[6],numbers[7],numbers[8]);
    const btScalar Mass = numbers[9];


    IMesh* mesh = m_irrDevice->getSceneManager()->getMesh((m_media + items[0]).c_str());
    m_irrDevice->getSceneManager()->getMeshManipulator()->scale(mesh,scale);
    IMeshSceneNode* Node = m_irrDevice->getSceneManager()->addMeshSceneNode( mesh );

    if(parent != NULL)
    {
        Node->setParent(parent);
        Node->setRotation(rotation);
    }
    Node->setMaterialType(EMT_SOLID);
    Node->setMaterialFlag(EMF_LIGHTING, 1);
    Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
    if(items[1] != "")
        Node->setMaterialTexture(0, m_irrDevice->getVideoDriver()->getTexture((m_media + items[1]).c_str()));

    // Set the initial position of the object
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(position);

    btDefaultMotionState *motionState = new btDefaultMotionState(Transform);

    // Create the shape
    btCollisionShape *Shape = MObjectCreator::convertMesh(Node);
    Shape->setMargin( 0.05f );

    // Add mass
    btVector3 localInertia;
    Shape->calculateLocalInertia(Mass, localInertia);

    // Create the rigid body object
    btRigidBody *rigidBody = new btRigidBody(Mass, motionState, Shape, localInertia);

    MMaterial* material = Material::getMaterial(items[2][0]);

    MObject* obj = new MObject(rigidBody, Node, material);
    // Store a pointer to the irrlicht node so we can update it later
    rigidBody->setUserPointer((void *)(obj));

    return obj;
}

gg::MObject* gg::MObjectCreator::createBoxedRigidBody(std::vector<std::string>&& items, ISceneNode *parent)
{
    if(items.size() != 13)
    {
        std::cerr << "Object failed to load: wrong number of parameters\n";
        return new MObject();
    }

    float numbers[10];
    for(int i = 0; i < 10; i++)
    {
        numbers[i] = std::stof(items[i+3]);
    }

    btVector3 position(numbers[0],numbers[1],numbers[2]);
    core::vector3df rotation(numbers[3],numbers[4],numbers[5]);
    core::vector3df scale(numbers[6],numbers[7],numbers[8]);
    const btScalar Mass = numbers[9];

    IMesh* mesh = m_irrDevice->getSceneManager()->getMesh((m_media + items[0]).c_str());
    m_irrDevice->getSceneManager()->getMeshManipulator()->scale(mesh,scale);
    IMeshSceneNode* Node = m_irrDevice->getSceneManager()->addMeshSceneNode( mesh );

    if(parent != NULL)
    {
        Node->setParent(parent);
        Node->setRotation(rotation);
    }
    Node->setMaterialType(EMT_SOLID);
    Node->setMaterialFlag(EMF_LIGHTING, 1);
    Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
    if(items[1] != "")
        Node->setMaterialTexture(0, m_irrDevice->getVideoDriver()->getTexture((m_media + items[1]).c_str()));

    // Set the initial position of the object
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(position);

    btDefaultMotionState *motionState = new btDefaultMotionState(Transform);

    //btConvexHullShape* hull = new btConvexHullShape();
    aabbox3df box = Node->getTransformedBoundingBox();
    float width =abs(box.MaxEdge.X-box.MinEdge.X);
    float height=abs(box.MaxEdge.Y-box.MinEdge.Y);
    float length=abs(box.MaxEdge.Z-box.MinEdge.Z);
    btBoxShape*hull = new btBoxShape(btVector3(0.1f, 0.1f, 0.1f));

    /*std::vector<btVector3> vertices(getVertices(Node));
    for (auto&& v : vertices)
    {
        hull->addPoint(v);
    }
*/
    // Create the shape
    btCollisionShape* Shape = hull;
    //Shape->setMargin( 0.05f );

    // Add mass
    btVector3 localInertia;
    Shape->calculateLocalInertia(Mass, localInertia);

    // Create the rigid body object
    btRigidBody *rigidBody = new btRigidBody(Mass, motionState, Shape, localInertia);

    MMaterial* material = Material::getMaterial(items[2][0]);

    MObject* obj = new MObject(rigidBody, Node, material);
    // Store a pointer to the irrlicht node so we can update it later
    rigidBody->setUserPointer((void *)(obj));

    return obj;
}

gg::MObject* gg::MObjectCreator::createSolidGround(btRigidBody * terrain)
{
    //Get position and size from terrain
    btTransform t;
    t.setIdentity();
    btVector3 min,max,pos;
    vector3df size;
    terrain->getCollisionShape()->getAabb(t,max,min);

    size = vector3df(abs(min.getX())+abs(max.getX()), 50, abs(min.getZ())+abs(max.getZ()));
    pos = btVector3(0,-1026+max.getY(),0);

    // Create an Irrlicht cube
    scene::ISceneNode *Node = m_irrDevice->getSceneManager()->addCubeSceneNode(1.0f);
    Node->setScale(size);
    Node->setMaterialFlag(video::EMF_LIGHTING, 1);
    Node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);

    // Set the initial position of the object
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(pos);

    // Give it a default MotionState
    btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

    // Create the shape
    btVector3 HalfExtents(size.X*0.5, size.Y*0.5 , size.Z*0.5);
    btCollisionShape *Shape = new btBoxShape(HalfExtents);

    // Add mass
    btVector3 LocalInertia;
    Shape->calculateLocalInertia(0, LocalInertia);

    // Create the rigid body object
    btRigidBody *RigidBody = new btRigidBody(0, MotionState, Shape, LocalInertia);
    //RigidBody->setGravity(btVector3(0,0,0));

    MObject * obj = new MObject(RigidBody, Node, &Material::Magic);

    // Store a pointer to the irrlicht node so we can update it later
    RigidBody->setUserPointer((void *)(obj));

    return obj;
}

btBvhTriangleMeshShape* gg::MObjectCreator::convertMesh(IMeshSceneNode * node)
{
    btTriangleMesh* btMesh = new btTriangleMesh();

    for (irr::u32 j = 0; j < node->getMesh()->getMeshBufferCount(); j++)
    {
        IMeshBuffer* meshBuffer = node->getMesh()->getMeshBuffer(j);
        S3DVertex* vertices = (S3DVertex*)meshBuffer->getVertices();
        u16* indices = meshBuffer->getIndices();

        for (u32 i = 0; i < meshBuffer->getIndexCount(); i += 3)
        {
            vector3df triangle1 = vertices[indices[i]].Pos;
            vector3df triangle2 = vertices[indices[i + 1]].Pos;
            vector3df triangle3 = vertices[indices[i + 2]].Pos;
            btMesh->addTriangle(btVector3(triangle1.X, triangle1.Y, triangle1.Z),
                                btVector3(triangle2.X, triangle2.Y, triangle2.Z),
                                btVector3(triangle3.X, triangle3.Y, triangle3.Z));
        }
    }
    return new btBvhTriangleMeshShape( btMesh, true );
}

btConvexHullShape* gg::MObjectCreator::convertMeshToHull(IMeshSceneNode * node)
{
    btConvexHullShape* hull= new btConvexHullShape();
    for (irr::u32 j = 0; j < node->getMesh()->getMeshBufferCount(); j++)
    {
        IMeshBuffer* meshBuffer = node->getMesh()->getMeshBuffer(j);
        S3DVertex* vertices = (S3DVertex*)meshBuffer->getVertices();
        for(irr::u32 i = 0; i < meshBuffer->getVertexCount(); i++)
        {
            vector3df vertex = vertices[i].Pos;
            hull->addPoint(btVector3(vertex.X, vertex.Y, vertex.Z));
        }
    }
    return hull;
}

std::vector<btVector3> gg::MObjectCreator::getVertices(IMeshSceneNode* Node)
{
    std::vector<btVector3> btvertices;
    for (irr::u32 i = 0; i < Node->getMesh()->getMeshBufferCount(); i++)
    {
        IMeshBuffer* meshBuffer = Node->getMesh()->getMeshBuffer(i);
        S3DVertex* vertices = (S3DVertex*)meshBuffer->getVertices();
        u16* indices = meshBuffer->getIndices();

        for (u32 i = 0; i < meshBuffer->getIndexCount(); i += 3)
        {
            vector3df triangle1 = vertices[indices[i]].Pos;
            vector3df triangle2 = vertices[indices[i + 1]].Pos;
            vector3df triangle3 = vertices[indices[i + 2]].Pos;
            btvertices.emplace_back(triangle1.X, triangle1.Y, triangle1.Z);
            btvertices.emplace_back(triangle2.X, triangle2.Y, triangle2.Z);
            btvertices.emplace_back(triangle3.X, triangle3.Y, triangle3.Z);
        }
    }
    return std::move(btvertices);
}

std::vector<btVector3> gg::MObjectCreator::getVertices(btConvexHullShape* hull)
{
    btVector3* points = hull->getUnscaledPoints();
    size_t n = hull->getNumPoints();
    std::vector<btVector3> vertices;

    for(size_t i = 0; i < n; i++)
    {
        vertices.push_back(*points);
        points++;
    }
    return std::move(vertices);
}











