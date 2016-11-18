#include "Loader.h"

gg::Loader::Loader(int w, int h , bool full)
{
    irrDevice = std::unique_ptr<irr::IrrlichtDevice>(createDevice(video::EDT_OPENGL, dimension2d<u32>(w,h), 32, full, false, false, 0)); //setEventReceiver(e) in game.cpp
    objects = std::vector<gg::Object>();
}


std::tuple<std::unique_ptr<IrrlichtDevice>, std::vector<gg::Object>> gg::Loader::Load(std::string)
{
//
return std::make_tuple(std::move(irrDevice), std::move(objects));
}
