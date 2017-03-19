#include "Loader.h"
#include <fstream>
#include <sstream>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

gg::MLoader::MLoader(int w, int h , bool full) : m_width(w), m_heigth(h), m_fullscreen(full)
{

}

std::tuple<std::unique_ptr<irr::IrrlichtDevice>, std::vector<std::unique_ptr<gg::MObject>>,std::vector<std::unique_ptr<btFixedConstraint>>> gg::MLoader::load(std::string level)
{
    //initialize needed variables
    m_irrDevice.reset(createDevice(video::EDT_OPENGL, dimension2d<u32>(m_width,m_heigth), 32, m_fullscreen, false, false, 0)); //setEventReceiver(e) in game.cpp
    m_objects = std::vector<std::unique_ptr<gg::MObject>>();
    m_objectCreator.reset(new MObjectCreator(m_irrDevice.get()));

    std::string current_line;
    //open level file
    std::fstream fin;
    fin.open (level, std::fstream::in);

    //read first line and setup skybox
    std::getline(fin, current_line);
    if(!loadSkybox(split(std::stringstream(current_line))))
        std::cerr << "Loading skybox failed!\n";

    int i = 0;
    //read otherlines --terrain and models
    while(i < 2 && std::getline(fin, current_line))
    {
        if(current_line != "")
        {
            i++;
            MObject* obj = m_objectCreator->createMeshRigidBody(split(std::stringstream(current_line)));
            if(obj)
            {
                m_objects.push_back(std::unique_ptr<gg::MObject>(obj));
            }
        }
    }

    while(std::getline(fin, current_line))
    {
        if(current_line != "")
        {
            std::vector<MObject*> objs;
            std::vector<btFixedConstraint*> constraints;
            std::tie(objs,constraints) = m_objectCreator->createDestructibleBody(split(std::stringstream(current_line)));
            for(auto&& obj : objs)
            {
                if(obj)
                {
                    m_objects.push_back(std::unique_ptr<gg::MObject>(obj));
                }
            }
            for(auto&& cons : constraints)
            {
                if(cons)
                {
                    m_constraints.push_back(std::unique_ptr<btFixedConstraint>(cons));
                }
            }
        }
    }

    if(!m_objects.empty())
        m_objects.push_back(std::unique_ptr<gg::MObject>(m_objectCreator->createSolidGround(m_objects[0]->getRigid())));

    fin.close();
    return std::move(std::make_tuple(std::move(m_irrDevice), std::move(m_objects), std::move(m_constraints)));
}

bool gg::MLoader::loadSkybox(std::vector<std::string>&& files)
{
    if(files.size() < 6)
        return false;

    IVideoDriver *irrDriver = m_irrDevice->getVideoDriver();

    irrDriver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
    m_irrDevice->getSceneManager()->addSkyBoxSceneNode(
        irrDriver->getTexture((m_media + files[0]).c_str()),
        irrDriver->getTexture((m_media + files[1]).c_str()),
        irrDriver->getTexture((m_media + files[2]).c_str()),
        irrDriver->getTexture((m_media + files[3]).c_str()),
        irrDriver->getTexture((m_media + files[4]).c_str()),
        irrDriver->getTexture((m_media + files[5]).c_str())
        );
    irrDriver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);

    return true;
}

std::vector<std::string> gg::MLoader::split(std::stringstream&& split)
{
    std::vector<std::string> parts;
    for(std::string item; std::getline(split, item, ';');parts.push_back(item));
    return std::move(parts);
}
