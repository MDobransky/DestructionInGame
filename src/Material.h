#ifndef MATERIALS_H
#define MATERIALS_H

#include <cstdint>


class Material
{
private:
    uint_fast32_t strength;
    uint_fast32_t density;
public:
    virtual ~Material() = 0;
    Material(int s, int d) : strength(s), density(d) {}
    //specific virtual functions for generatiing particles
};

class Concrete : Material
{
public:
    Concrete() : Material(5,10) //some specific constants
    {

    }
    virtual ~Concrete() {}
};

//more materials

#endif
