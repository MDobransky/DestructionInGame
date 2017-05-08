#include "ObjectCreator.h"
#include "Material.h"
#include "MeshManipulators.h"


#include <iostream>
#include <cmath>
#include <map>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

gg::MObjectCreator::MObjectCreator(IrrlichtDevice* irr) : m_irrDevice(irr)
{
}

gg::MObject* gg::MObjectCreator::createMeshRigidBody(std::vector<std::string>&& items)
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


    IMesh* mesh_orig = m_irrDevice->getSceneManager()->getMesh((m_media + items[0]).c_str());
    IMesh* mesh = m_irrDevice->getSceneManager()->getMeshManipulator()->createMeshUniquePrimitives(mesh_orig);

    IMeshSceneNode* Node = m_irrDevice->getSceneManager()->addMeshSceneNode(mesh);
    m_irrDevice->getSceneManager()->getMeshManipulator()->scale(mesh,scale);
    Node->setRotation(rotation);
    Node->setMaterialType(EMT_SOLID);
    Node->setMaterialFlag(EMF_LIGHTING, 0);
    Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
    if(items[1] != "")
        Node->setMaterialTexture(0, m_irrDevice->getVideoDriver()->getTexture((m_media + items[1]).c_str()));

    // Set the initial position of the object
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(position);

    btDefaultMotionState *motionState = new btDefaultMotionState(Transform);

    // Create the shape
    btCollisionShape *Shape = MeshManipulators::convertMesh(Node);
    Shape->setMargin( 0.05f );

    // Add mass
    btVector3 localInertia;
    Shape->calculateLocalInertia(Mass, localInertia);

    // Create the rigid body object
    btRigidBody *rigidBody = new btRigidBody(Mass, motionState, Shape, localInertia);

    const MMaterial* material = MMaterial::getMaterial(items[2][0]);

    MObject* obj = new MObject(rigidBody, Node, material);
    // Store a pointer to the irrlicht node so we can update it later
    rigidBody->setUserPointer((void *)(obj));

    return obj;
}

gg::MObject* gg::MObjectCreator::createBoxedRigidBody(std::vector<std::string>&& items)
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


    Node->setRotation(rotation);
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
    /*aabbox3df box = Node->getTransformedBoundingBox();
    float width =abs(box.MaxEdge.X-box.MinEdge.X);
    float height=abs(box.MaxEdge.Y-box.MinEdge.Y);
    float length=abs(box.MaxEdge.Z-box.MinEdge.Z);*/
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

   const MMaterial* material = MMaterial::getMaterial(items[2][0]);

    MObject* obj = new MObject(rigidBody, Node, material);
    // Store a pointer to the irrlicht node so we can update it later
    rigidBody->setUserPointer((void *)(obj));

    return obj;
}

gg::MObject* gg::MObjectCreator::createSolidGround(std::vector<std::string>&& items)
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

    //std::string input(items[0]);
    std::string texture(items[1]);
    //char material(items[2][0]);
    btVector3 position(numbers[0],numbers[1],numbers[2]);
    core::vector3df rotation(numbers[3],numbers[4],numbers[5]);
    core::vector3df scale(numbers[6],numbers[7],numbers[8]);
    const btScalar Mass = numbers[9];

    // Create an Irrlicht cube
    scene::ISceneNode *Node = m_irrDevice->getSceneManager()->addCubeSceneNode(1.0f);
    Node->setScale(scale);
    Node->setRotation(rotation);
    Node->setMaterialFlag(video::EMF_LIGHTING, 1);
    Node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
    if(items[1] != "")
        Node->setMaterialTexture(0, m_irrDevice->getVideoDriver()->getTexture((m_media + texture).c_str()));


    // Set the initial position of the object
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(position);

    // Give it a default MotionState
    btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

    // Create the shape
    btVector3 HalfExtents(scale.X*0.5, scale.Y*0.5 , scale.Z*0.5);
    btCollisionShape *Shape = new btBoxShape(HalfExtents);

    // Add mass
    btVector3 LocalInertia;
    Shape->calculateLocalInertia(Mass, LocalInertia);

    // Create the rigid body object
    btRigidBody *RigidBody = new btRigidBody(Mass, MotionState, Shape, LocalInertia);
    RigidBody->setGravity(btVector3(0,0,0));

    MObject * obj = new MObject(RigidBody, Node, &MMaterial::Magic);

    RigidBody->setUserPointer((void *)(obj));

    return obj;
}












