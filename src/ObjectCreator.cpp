#include "ObjectCreator.h"

#include <iostream>
#include <cmath>


gg::MObjectCreator::MObjectCreator(IrrlichtDevice* irr) : m_irrDevice(irr)
{
}

gg::MObject* gg::MObjectCreator::createRigidBody(std::vector<std::string>&& items, ISceneNode *parent)
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

    // Create the shape
    btCollisionShape *Shape = MObjectCreator::convertMesh(Node);
    Shape->setMargin( 0.05f );

    // Add mass
    btVector3 localInertia;
    Shape->calculateLocalInertia(Mass, localInertia);

    // Create the rigid body object
    btRigidBody *rigidBody = new btRigidBody(Mass, motionState, Shape, localInertia);

    MObject* obj = new MObject(rigidBody, Node);
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
    Node->setMaterialTexture(0, m_irrDevice->getVideoDriver()->getTexture("media/rust0.jpg"));

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

    MObject * obj = new MObject(RigidBody, Node);

    // Store a pointer to the irrlicht node so we can update it later
    RigidBody->setUserPointer((void *)(obj));

    return obj;
}

btBvhTriangleMeshShape* gg::MObjectCreator::convertMesh(IMeshSceneNode * node)
{
    btTriangleMesh* btMesh = new btTriangleMesh();

    for (irr::u32 i = 0; i < node->getMesh()->getMeshBufferCount(); i++)
    {
        IMeshBuffer* meshBuffer = node->getMesh()->getMeshBuffer(i);
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












