#ifndef EDEM_H
#define EDEM_H
//#include "Material.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

namespace gg {

//Extended Distinct Element Method' Element
class MEdem
{
private:
    btRigidBody* rigidBody;
 //   MMaterial* material;
    ITexture * texture;
    //set of pointers to neighbouring elements

public:

};

}

#endif
