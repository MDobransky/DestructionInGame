#include "Loader.h"
#include <fstream>
#include <sstream>

gg::Loader::Loader(int w, int h , bool full) : width(w), heigth(h), fullscreen(full)
{

}

std::tuple<std::unique_ptr<IrrlichtDevice>, std::vector<gg::Object>> gg::Loader::load(std::string level)
{
    //initialize needed variables
    irrDevice.reset(createDevice(video::EDT_OPENGL, dimension2d<u32>(width,heigth), 32, fullscreen, false, false, 0)); //setEventReceiver(e) in game.cpp
    objects = std::vector<gg::Object>();
    objectCreator.reset(new ObjectCreator(irrDevice.get()));

    std::string current_line;
    //open level file
    std::fstream fin;
    fin.open (level, std::fstream::in);

    //read first line --terrain
    std::getline(fin, current_line);
    objects.push_back(objectCreator->createTerrain(split(std::stringstream(current_line))));
    if(!objects.back().isEmpty())
        objects.push_back(objectCreator->createSolidGround(objects.back().getRigid()));

    //read second line and setup skybox
    std::getline(fin, current_line);
    if(!loadSkybox(split(std::stringstream(current_line))))
        std::cerr << "Loading skybox failed!\n";

    //read vehicle and all other models
    while(std::getline(fin, current_line))
        objects.push_back(objectCreator->createRigidBody(split(std::stringstream(current_line))));

    fin.close();
    return std::make_tuple(std::move(irrDevice), std::move(objects));
}

bool gg::Loader::loadSkybox(std::vector<std::string>&& files)
{
    if(files.size() < 6)
        return false;

    IVideoDriver *irrDriver = irrDevice->getVideoDriver();

    irrDriver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    irrDevice->getSceneManager()->addSkyBoxSceneNode(
        irrDriver->getTexture((media + files[0]).c_str()),
        irrDriver->getTexture((media + files[1]).c_str()),
        irrDriver->getTexture((media + files[2]).c_str()),
        irrDriver->getTexture((media + files[3]).c_str()),
        irrDriver->getTexture((media + files[4]).c_str()),
        irrDriver->getTexture((media + files[5]).c_str())
        );
    irrDriver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

    return true;
}

std::vector<std::string> gg::Loader::split(std::stringstream&& split)
{
    std::vector<std::string> parts;
    for(std::string item; std::getline(split, item, ';');parts.push_back(item));
    return std::move(parts);
}
