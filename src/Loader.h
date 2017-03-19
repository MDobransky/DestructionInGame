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



namespace gg {

class MLoader
{


public:
    MLoader(int, int, bool); //width, heigth, fullscreen

    //takes the file describing what to load and returns irrlicht device and rigidbody objects
    std::tuple<std::unique_ptr<irr::IrrlichtDevice>, std::vector<std::unique_ptr<gg::MObject>>,std::vector<std::unique_ptr<btFixedConstraint>>> load(std::string);

private:
    const std::string m_media = "media/";
    bool loadSkybox(std::vector<std::string>&&);
    std::vector<std::string> split(std::stringstream&&);

    std::unique_ptr<irr::IrrlichtDevice> m_irrDevice;
    std::vector<std::unique_ptr<gg::MObject>> m_objects;
    std::vector<std::unique_ptr<btFixedConstraint>> m_constraints;
    std::unique_ptr<MObjectCreator> m_objectCreator;
    int m_width, m_heigth;
    bool m_fullscreen;
};

}

#endif // LOADER_H
