/* is only used for initializing the application.
 * It parses the data that describe the game environment from the medial/world.cfg file,
 * constructs the objects using tt gg::MObjectCreator, and returns the set of constructed objects.
 */

#ifndef LOADER_H
#define LOADER_H

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

namespace gg
{

    class MLoader
    {
    public:
        MLoader(irr::IrrlichtDevice *);

        std::vector<std::unique_ptr<gg::MObject>> load(std::string);

    private:
        const std::string m_media = "media/";

        bool loadSkybox(std::vector<std::string> &&);

        std::vector<std::string> split(std::stringstream &&);

        irr::IrrlichtDevice *m_irrDevice;
        std::vector<std::unique_ptr<gg::MObject>> m_objects;
        std::unique_ptr<MObjectCreator> m_objectCreator;
    };

}

#endif // LOADER_H
