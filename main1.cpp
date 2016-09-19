
#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <cstdlib>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

// Functions
static void CreateStartScene();
static IAnimatedMeshSceneNode*  CreateShip(const btVector3 &TPosition, const core::vector3df &TScale, btScalar TMass);
void CreateBox(const btVector3 &TPosition, const core::vector3df &TScale, btScalar TMass);
static void UpdatePhysics(u32 TDeltaTime);
static void UpdateRender(btRigidBody *TObject);
static void ClearObjects();

// Globals
static bool Done = false;
static btDiscreteDynamicsWorld *World;
static IrrlichtDevice *irrDevice;
static IVideoDriver *irrDriver;
static ISceneManager *irrScene;
static IGUIEnvironment *irrGUI;
//static IFileSystem *irrFile;
static ITimer *irrTimer;
//static ILogger *irrLog;
static list<btRigidBody *> Objects;
static IAnimatedMeshSceneNode* Ship;
static ICameraSceneNode* Camera;
static btRigidBody* btShip;

// Event receiver
class EventReceiverClass : public IEventReceiver  {

public:

	virtual bool OnEvent(const SEvent &TEvent) {

		if(TEvent.EventType == EET_KEY_INPUT_EVENT && !TEvent.KeyInput.PressedDown) {
			int torque = 1000;
			switch(TEvent.KeyInput.Key) {
				case KEY_ESCAPE:
					Done = true;
				break;
				case KEY_KEY_X:
					CreateStartScene();
				break;
				case KEY_KEY_W:
				{
					btVector3 worldTorque = btShip->getWorldTransform().getBasis() * btVector3(-torque,0,0);
					btShip->activate();
					btShip->applyTorque(worldTorque);
				}
				break;
				case KEY_KEY_S:
				{
					btVector3 worldTorque = btShip->getWorldTransform().getBasis() * btVector3(torque,0,0);
					btShip->activate();
					btShip->applyTorque(worldTorque);
				}
				break;
				case KEY_KEY_A:
				{
					btVector3 worldTorque = btShip->getWorldTransform().getBasis() * btVector3(0,-torque,0);
					btShip->activate();
					btShip->applyTorque(worldTorque);
				}
				break;
				case KEY_KEY_D:
				{
					btVector3 worldTorque = btShip->getWorldTransform().getBasis() * btVector3(0,torque,0);
					btShip->activate();
					btShip->applyTorque(worldTorque);
				}
				break;
				case KEY_KEY_Q:
				{
					btVector3 worldTorque = btShip->getWorldTransform().getBasis() * btVector3(0,0,-torque);
					btShip->activate();
					btShip->applyTorque(worldTorque);
				}
				break;
				case KEY_KEY_E:
				{
					btVector3 worldTorque = btShip->getWorldTransform().getBasis() * btVector3(0,0,torque);
					btShip->activate();
					btShip->applyTorque(worldTorque);
				}
				break;
				case KEY_CONTROL:
					;
				break;
				case KEY_SHIFT:
					;
				break;
				default:
					return false;
				break;
			}

			return true;
		}

		return false;
	}
};

int main() {

	// Initialize irrlicht
	EventReceiverClass Receiver;
	irrDevice = createDevice(video::EDT_OPENGL, dimension2d<u32>(1600,900), 32, false, false, false, &Receiver);
	irrGUI = irrDevice->getGUIEnvironment();
	irrTimer = irrDevice->getTimer();
	irrScene = irrDevice->getSceneManager();
	irrDriver = irrDevice->getVideoDriver();

	irrDevice->getCursorControl()->setVisible(0);

	// Initialize bullet
	btDefaultCollisionConfiguration *CollisionConfiguration = new btDefaultCollisionConfiguration();
	btBroadphaseInterface *BroadPhase = new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000));
	btCollisionDispatcher *Dispatcher = new btCollisionDispatcher(CollisionConfiguration);
	btSequentialImpulseConstraintSolver *Solver = new btSequentialImpulseConstraintSolver();
	World = new btDiscreteDynamicsWorld(Dispatcher, BroadPhase, Solver, CollisionConfiguration);


	CreateStartScene();


	// Main loop
	u32 TimeStamp = irrTimer->getTime(), DeltaTime = 0;
	while(!Done) {

		DeltaTime = irrTimer->getTime() - TimeStamp;
		TimeStamp = irrTimer->getTime();

		UpdatePhysics(DeltaTime);

		irrDriver->beginScene(true, true, SColor(255, 20, 0, 0));
		btVector3 trans = btShip->getWorldTransform().getBasis() * btVector3(0,0,+10);
		Camera->setPosition(vector3df(trans.getX(),trans.getY(),trans.getZ())); 
		Camera->setTarget(Ship->getPosition());
		
		irrScene->drawAll();
		irrGUI->drawAll();
		irrDriver->endScene();
		irrDevice->run();
	}

	ClearObjects();
	delete World;
	delete Solver;
	delete Dispatcher;
	delete BroadPhase;
	delete CollisionConfiguration;

	irrDevice->drop();

	return 0;
}

// Runs the physics simulation.
// - TDeltaTime tells the simulation how much time has passed since the last frame so the simulation can run independently of the frame rate.
void UpdatePhysics(u32 TDeltaTime) {

	World->stepSimulation(TDeltaTime * 0.001f, 60);

	// Relay the object's orientation to irrlicht
	for(list<btRigidBody *>::Iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {

		UpdateRender(*Iterator);
	}	
}

// Creates a base box
void CreateStartScene() 
{
		// Create the initial scene
	irrScene->addLightSceneNode(0, core::vector3df(0, 300, 0), SColorf(4, 4, 4, 1));
	irrScene->setAmbientLight(video::SColorf(0.3,0.3,0.3,1));
	
	ClearObjects();

	//ground
	CreateBox(btVector3(0.0f, -0.1f, -0.5f), vector3df(9.0f, 0.2f, 11.0f), 0.0f);

	Ship = CreateShip(btVector3(0.0f, 2.0f, 0.0f), core::vector3df(9.0f, 3.0f, 11.0f), 5.5f);

	
	Camera= irrScene->addCameraSceneNode();
	btVector3 trans = btShip->getWorldTransform().getBasis() * btVector3(0,0,+10);
	Camera->setPosition(vector3df(trans.getX(),trans.getY(),trans.getZ())); 
   	Camera->setTarget(Ship->getPosition());
	Camera->setParent(Ship);
	
	btShip->setGravity(btVector3(0,0,0));

}



IAnimatedMeshSceneNode*  CreateShip(const btVector3 &TPosition, const core::vector3df &TScale, btScalar TMass) 
{

	IAnimatedMesh* mesh = irrScene->getMesh("xwing.3ds");
	irrScene->getMeshManipulator()->scale(mesh,core::vector3df(2.6,2.6,2.6));
	IAnimatedMeshSceneNode* Node = irrScene->addAnimatedMeshSceneNode( mesh );
	Node->setMaterialType(EMT_SOLID);
	Node->setMaterialFlag(EMF_LIGHTING, 1);
	Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);

	// Set the initial position of the object
	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(TPosition);


	btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

	// Create the shape
	btVector3 HalfExtents(TScale.X*0.5, TScale.Y*0.5 , TScale.Z*0.5);
	btCollisionShape *Shape = new btBoxShape(HalfExtents);

	// Add mass
	btVector3 LocalInertia;
	Shape->calculateLocalInertia(TMass, LocalInertia);

	// Create the rigid body object
	btRigidBody *RigidBody = new btRigidBody(TMass, MotionState, Shape, LocalInertia);

	// Store a pointer to the irrlicht node so we can update it later
	RigidBody->setUserPointer((void *)(Node));

	// Add it to the world
	World->addRigidBody(RigidBody);
	Objects.push_back(RigidBody);
	
btShip = RigidBody;
return Node;
}

// Create a box rigid body
void CreateBox(const btVector3 &TPosition, const core::vector3df &TScale, btScalar TMass) {

	// Create an Irrlicht cube
	scene::ISceneNode *Node = irrScene->addCubeSceneNode(1.0f);
	Node->setScale(TScale);
	Node->setMaterialFlag(video::EMF_LIGHTING, 1);
	Node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
	Node->setMaterialTexture(0, irrDriver->getTexture("rust0.jpg"));

	// Set the initial position of the object
	btTransform Transform;
	Transform.setIdentity();
	Transform.setOrigin(TPosition);

	// Give it a default MotionState
	btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

	// Create the shape
	btVector3 HalfExtents(TScale.X*0.5, TScale.Y*0.5 , TScale.Z*0.5);
	btCollisionShape *Shape = new btBoxShape(HalfExtents);
	
	// Add mass
	btVector3 LocalInertia;
	Shape->calculateLocalInertia(TMass, LocalInertia);

	// Create the rigid body object
	btRigidBody *RigidBody = new btRigidBody(TMass, MotionState, Shape, LocalInertia);

	// Store a pointer to the irrlicht node so we can update it later
	RigidBody->setUserPointer((void *)(Node));

	// Add it to the world
	World->addRigidBody(RigidBody);
	Objects.push_back(RigidBody);
}


// Passes bullet's orientation to irrlicht
void UpdateRender(btRigidBody *TObject) {
	ISceneNode *Node = static_cast<ISceneNode *>(TObject->getUserPointer());

	// Set position
	btVector3 Point = TObject->getCenterOfMassPosition();
	Node->setPosition(vector3df((f32)Point[0], (f32)Point[1], (f32)Point[2]));

	// Set rotation
	vector3df Euler;
	const btQuaternion& TQuat = TObject->getOrientation();
	quaternion q(TQuat.getX(), TQuat.getY(), TQuat.getZ(), TQuat.getW());
	q.toEuler(Euler);
	Euler *= RADTODEG;
	Node->setRotation(Euler);
}

// Removes all objects from the world
void ClearObjects() {

	for(list<btRigidBody *>::Iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		btRigidBody *Object = *Iterator;

		// Delete irrlicht node
		ISceneNode *Node = static_cast<ISceneNode *>(Object->getUserPointer());
		Node->remove();

		// Remove the object from the world
		World->removeRigidBody(Object);

		// Free memory
		delete Object->getMotionState();
		delete Object->getCollisionShape();
		delete Object;
	}

	Objects.clear();
}
