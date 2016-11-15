//takes a file describing level, loads everything and returns irrDevice, Objects[] with stored edem elements

#ifndef LOADER_H
#define LOADER_H

#include <Edem.h>
#include <Object.h>
#include <EventReceiver.h>
#include <Material.h>

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <vector>
#include <iostream>
#include <string>
#include <tuple>


namespace gg {

}
class Loader
{
public:
    Loader();
    //takes the file describing what to load and returns irrlicht device and rigidbody objects
    std::tuple<std::unique_ptr<IrrlichtDevice>,std::unique_ptr<std::vector<gg::Object>> Load(std::string);
private:
    std::unique_ptr<IrrlichtDevice> irrDev;
    //objects[0] is always terrain
    //objects[1] is user controlled vehicle
    std::unique_ptr<std::vector<gg::Object>> objects;
    bool initialize_irrlicht();
    bool load_terrain(std::string, Material);
    bool load_vehicle(std::string); //always steel


};

}

#endif // LOADER_H
