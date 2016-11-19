#include "ObjectCreator.h"


gg::Object gg::ObjectCreator::createTerrain(std::string &)
{
Object a;
return std::move(a);
}

gg::Object gg::ObjectCreator::createRigidBody(std::string &)
{
    Object a;
    return std::move(a);
}
