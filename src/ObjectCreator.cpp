#include "ObjectCreator.h"
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

gg::MObjectCreator::MObjectCreator(IrrlichtDevice *irr) : m_irrDevice(irr)
{
}

gg::MObject *gg::MObjectCreator::createMeshRigidBody(std::vector<std::string> &&items)
{
    if(items.size() != 13)
    {
        std::cerr << "Object failed to load: wrong number of parameters\n";
        return new MObject();
    }

    float numbers[10];
    for(int i = 0; i < 10; i++)
    {
        numbers[i] = std::stof(items[i + 3]);
    }

    std::string input(items[0]);
    btVector3 position(numbers[0], numbers[1], numbers[2]);
    core::vector3df rotation(numbers[3], numbers[4], numbers[5]);
    core::vector3df scale(numbers[6], numbers[7], numbers[8]);
    const btScalar Mass = numbers[9];

    IMesh *mesh_orig = m_irrDevice->getSceneManager()->getMesh((m_media + input).c_str());
    IMesh *mesh = m_irrDevice->getSceneManager()->getMeshManipulator()->createMeshUniquePrimitives(mesh_orig);

    IMeshSceneNode *Node = m_irrDevice->getSceneManager()->addMeshSceneNode(mesh);
    m_irrDevice->getSceneManager()->getMeshManipulator()->scale(mesh, scale);
    Node->setRotation(rotation);

    MeshManipulators::Nef_polyhedron polyhedron = std::move(MeshManipulators::makeNefPolyhedron(mesh));

    vector3df center;
    //make colorfull mesh
    std::tie(mesh, center) = MeshManipulators::convertPolyToMesh(polyhedron);
    Node->setMesh(mesh);
    Node->setMaterialType(EMT_SOLID);
    Node->setMaterialFlag(EMF_LIGHTING, 0);
    Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);

    // Set the initial position of the object
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(position + btVector3(center.X, center.Y, center.Z));

    btDefaultMotionState *motionState = new btDefaultMotionState(Transform);

    // Create the shape
    btCollisionShape *Shape = new btHACDCompoundShape(MeshManipulators::convertMesh(Node));
    Shape->setMargin(0.01f);

    // Add mass
    btVector3 localInertia;
    Shape->calculateLocalInertia(Mass, localInertia);

    // Create the rigid body object
    btRigidBody *rigidBody = new btRigidBody(Mass, motionState, Shape, localInertia);

    MObject::Material material = MObject::Material::BUILDING;

    MObject *obj = new MObject(rigidBody, Node, material, std::move(polyhedron));
    // Store a pointer to the irrlicht node so we can update it later
    rigidBody->setUserPointer((void *) (obj));


    return obj;
}

gg::MObject *gg::MObjectCreator::createBoxedRigidBody(std::vector<std::string> &&items)
{
    if(items.size() != 13)
    {
        std::cerr << "Object failed to load: wrong number of parameters\n";
        return new MObject();
    }

    float numbers[10];
    for(int i = 0; i < 10; i++)
    {
        numbers[i] = std::stof(items[i + 3]);
    }

    btVector3 position(numbers[0], numbers[1], numbers[2]);
    core::vector3df rotation(numbers[3], numbers[4], numbers[5]);
    core::vector3df scale(numbers[6], numbers[7], numbers[8]);
    const btScalar Mass = numbers[9];

    IMesh *mesh = m_irrDevice->getSceneManager()->getMesh((m_media + items[0]).c_str());
    m_irrDevice->getSceneManager()->getMeshManipulator()->scale(mesh, scale);
    IMeshSceneNode *Node = m_irrDevice->getSceneManager()->addMeshSceneNode(mesh);


    Node->setRotation(rotation);
    Node->setMaterialType(EMT_SOLID);
    Node->setMaterialFlag(EMF_LIGHTING, 1);
    Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
    if(items[1] != "")
    {
        Node->setMaterialTexture(0, m_irrDevice->getVideoDriver()->getTexture((m_media + items[1]).c_str()));
    }

    // Set the initial position of the object
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(position);

    btDefaultMotionState *motionState = new btDefaultMotionState(Transform);

    // Create the shape
    btCollisionShape *Shape = new btBoxShape(btVector3(0.1f, 0.1f, 0.1f));

    // Add mass
    btVector3 localInertia;
    Shape->calculateLocalInertia(Mass, localInertia);

    // Create the rigid body object
    btRigidBody *rigidBody = new btRigidBody(Mass, motionState, Shape, localInertia);

    MObject::Material material = MObject::Material::SHIP;

    MObject *obj = new MObject(rigidBody, Node, material, false);
    rigidBody->setUserPointer((void *) (obj));

    return obj;
}

gg::MObject *gg::MObjectCreator::createSolidGround(std::vector<std::string> &&items)
{
    if(items.size() != 13)
    {
        std::cerr << "Object failed to load: wrong number of parameters\n";
        return new MObject();
    }

    float numbers[10];
    for(int i = 0; i < 10; i++)
    {
        numbers[i] = std::stof(items[i + 3]);
    }

    std::string texture(items[1]);
    btVector3 position(numbers[0], numbers[1], numbers[2]);
    core::vector3df rotation(numbers[3], numbers[4], numbers[5]);
    core::vector3df scale(numbers[6], numbers[7], numbers[8]);
    const btScalar Mass = numbers[9];

    // Create an Irrlicht cube
    scene::ISceneNode *Node = m_irrDevice->getSceneManager()->addCubeSceneNode(1.0f);
    Node->setScale(scale*vector3df(10,1,10));
    Node->setRotation(rotation);
    Node->setMaterialFlag(video::EMF_LIGHTING, 1);
    Node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
    if(items[1] != "")
    {
        Node->setMaterialTexture(0, m_irrDevice->getVideoDriver()->getTexture((m_media + texture).c_str()));
    }


    // Set the initial position of the object
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(position);

    // Give it a default MotionState
    btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

    // Create the shape
    btVector3 HalfExtents(scale.X * 0.5, scale.Y * 0.5, scale.Z * 0.5);
    btCollisionShape *Shape = new btBoxShape(HalfExtents);

    // Add mass
    btVector3 LocalInertia;
    Shape->calculateLocalInertia(Mass, LocalInertia);

    // Create the rigid body object
    btRigidBody *RigidBody = new btRigidBody(Mass, MotionState, Shape, LocalInertia);
    RigidBody->setGravity(btVector3(0, 0, 0));

    MObject *obj = new MObject(RigidBody, Node, MObject::Material::GROUND, false);

    RigidBody->setUserPointer((void *) (obj));

    return obj;
}

gg::MObject *gg::MObjectCreator::shoot(btVector3 position, btVector3 impulse)
{
    ///bullet size
    ISceneNode *Node = m_irrDevice->getSceneManager()->addLightSceneNode();
    Node->setMaterialType(EMT_SOLID);
    Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);

    // Set the initial position
    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(position);

    btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

    btCollisionShape *Shape = new btSphereShape(0.2f);
    Shape->setMargin(0.1);

    btVector3 LocalInertia;
    btScalar Mass = 1.0f;
    Shape->calculateLocalInertia(Mass, LocalInertia);

    btRigidBody *bullet = new btRigidBody(Mass, MotionState, Shape, LocalInertia);
    bullet->applyImpulse(impulse, position);

    MObject *obj = new MObject(bullet, Node, MObject::Material::SHOT, false);

    bullet->setUserPointer((void *) (obj));

    ISceneNode *nodeB = m_irrDevice->getSceneManager()->addBillboardSceneNode(Node, core::dimension2d<f32>(0.5, 0.5));
    nodeB->setMaterialFlag(video::EMF_LIGHTING, false);
    nodeB->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
    nodeB->setMaterialTexture(0, m_irrDevice->getVideoDriver()->getTexture("media/shot3.jpg"));

    return obj;
}

gg::MObject *gg::MObjectCreator::createMeshRigidBodyWithTmpShape(IMesh* mesh, btVector3 position, btScalar mass,
                                                     gg::MObject::Material material,
                                                     gg::MeshManipulators::Nef_polyhedron &&poly)
{
    IMeshSceneNode *Node = m_irrDevice->getSceneManager()->addMeshSceneNode(mesh);
    Node->setPosition(vector3df(position.getX(), position.getY(), position.getZ()));
    Node->setMaterialType(EMT_SOLID);
    Node->setMaterialFlag(EMF_LIGHTING, 0);
    Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);

    btTransform Transform;
    Transform.setIdentity();
    Transform.setOrigin(position);

    btDefaultMotionState *motionState = new btDefaultMotionState(Transform);

    btCollisionShape *Shape = new btSphereShape(mesh->getBoundingBox().getExtent().getLength());

    btVector3 localInertia;
    Shape->calculateLocalInertia(mass, localInertia);

    btRigidBody *rigidBody = new btRigidBody(mass, motionState, Shape, localInertia);

    MObject *fragment = new MObject(rigidBody, Node, material, std::move(poly));
    rigidBody->setUserPointer((void *) (fragment));

    return fragment;
}












