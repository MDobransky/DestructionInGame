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


class MLoader
{
public:
    MLoader(int, int, bool); //width, heigth, fullscreen

    //takes the file describing what to load and returns irrlicht device and rigidbody objects
    std::tuple<std::unique_ptr<IrrlichtDevice>, std::vector<std::unique_ptr<gg::MObject>>> load(std::string);

private:
    const std::string m_media = "media/";
    bool loadSkybox(std::vector<std::string>&&);
    std::vector<std::string> split(std::stringstream&&);

    std::unique_ptr<IrrlichtDevice> m_irrDevice;
    std::vector<std::unique_ptr<gg::MObject>> m_objects;
    std::unique_ptr<MObjectCreator> m_objectCreator;
    int m_width, m_heigth;
    bool m_fullscreen;



};

}

#endif // LOADER_H
