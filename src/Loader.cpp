#include "Loader.h"
#include <fstream>
#include <sstream>

gg::Loader::Loader(int w, int h , bool full) : width(w), heigth(h), fullscreen(full)
{
    objectCreator = std::make_unique<ObjectCreator>();
}

std::tuple<std::unique_ptr<IrrlichtDevice>, std::vector<gg::Object>> gg::Loader::load(std::string level)
{
    //initialize needed variables
    irrDevice = std::unique_ptr<irr::IrrlichtDevice>(createDevice(video::EDT_OPENGL, dimension2d<u32>(width,heigth), 32, fullscreen, false, false, 0)); //setEventReceiver(e) in game.cpp
    objects = std::vector<gg::Object>();
    std::string current_line;
    //open level file
    std::fstream fin;
    fin.open (level, std::fstream::in);

    //read first line --terrain
    std::getline(fin, current_line);
    objects.push_back(objectCreator->createTerrain(current_line));

    //read second line and setup skybox
    std::getline(fin, current_line);
    if(!loadSkybox(current_line))
        std::cerr << "Loading skybox failed!\n";

    //read vehicle and all other models
    while(std::getline(fin, current_line))
        objects.push_back(objectCreator->createRigidBody(current_line));

    fin.close();
    return std::make_tuple(std::move(irrDevice), std::move(objects));
}

bool gg::Loader::loadSkybox(std::string& line)
{
    std::istringstream split(line);
    std::vector<std::string> files;
    for(std::string file; std::getline(split, file, ';');files.push_back(media + file));
    if(files.size() < 6)
        return false;

    IVideoDriver *irrDriver = irrDevice->getVideoDriver();

    irrDriver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    irrDevice->getSceneManager()->addSkyBoxSceneNode(
        irrDriver->getTexture(files[0].c_str()),
        irrDriver->getTexture(files[1].c_str()),
        irrDriver->getTexture(files[2].c_str()),
        irrDriver->getTexture(files[3].c_str()),
        irrDriver->getTexture(files[4].c_str()),
        irrDriver->getTexture(files[5].c_str())
        );
    irrDriver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

    return true;
}
