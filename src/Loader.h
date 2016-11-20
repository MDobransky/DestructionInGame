//takes a file describing level, loads everything and returns irrDevice, Objects[] with stored edem elements

#ifndef LOADER_H
#define LOADER_H

#include "Edem.h"
#include "Object.h"
#include "ObjectCreator.h"

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <vector>
#include <iostream>
#include <string>
#include <tuple>
#include <memory>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

namespace gg {


class Loader
{
public:
    Loader(int, int, bool); //width, heigth, fullscreen

    //takes the file describing what to load and returns irrlicht device and rigidbody objects
    std::tuple<std::unique_ptr<IrrlichtDevice>, std::vector<Object>> load(std::string);

private:
    const std::string media = "media/";
    bool load_vehicle(std::string&);
    bool loadSkybox(std::vector<std::string>&&);
    std::unique_ptr<IrrlichtDevice> irrDevice;
    std::vector<Object> objects;
    std::unique_ptr<ObjectCreator> objectCreator;
    int width, heigth;
    bool fullscreen;
    std::vector<std::string> split(std::stringstream&&);


};

}

#endif // LOADER_H
