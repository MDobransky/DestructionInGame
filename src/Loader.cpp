#include "Loader.h"
#include <fstream>
#include <sstream>

gg::MLoader::MLoader(int w, int h , bool full) : m_width(w), m_heigth(h), m_fullscreen(full)
{

}

std::tuple<std::unique_ptr<IrrlichtDevice>, std::vector<std::unique_ptr<gg::MObject>>> gg::MLoader::load(std::string level)
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

    //read otherlines --terrain and models
    while(std::getline(fin, current_line))
    {
        if(current_line != "")
        {
            if(m_objects.size() > 1) //parrent is terrain but not to itself or vehicle
            {
                ISceneNode* parent = (static_cast<MObject *>(m_objects[0]->getRigid()->getUserPointer()))->getNode();
                m_objects.push_back(std::unique_ptr<gg::MObject>(m_objectCreator->createRigidBody(split(std::stringstream(current_line)),parent)));
            }
            else
            {
                m_objects.push_back(std::unique_ptr<gg::MObject>(m_objectCreator->createRigidBody(split(std::stringstream(current_line)))));
            }
        }
    }

    if(!m_objects.empty())
        m_objects.push_back(std::unique_ptr<gg::MObject>(m_objectCreator->createSolidGround(m_objects[0]->getRigid())));

    fin.close();
    return std::move(std::make_tuple(std::move(m_irrDevice), std::move(m_objects)));
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
