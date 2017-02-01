#ifndef MATERIALS_H
#define MATERIALS_H

#include <cstdint>

namespace gg {


class MMaterial
{
private:
    //strength will represent breaking force of joint in edem simulation
    double strength; //ultimate tensile strength in MPa
    uint_fast32_t density; //kg/m^3

protected:
    MMaterial(double s,int d) : strength(s), density(d) {}
    virtual ~MMaterial() = 0;
    //std::function
public:
    inline double getStrength() { return strength; }
    inline int getDensity() { return density; }
    //call function
};

//all the values are real-like
/*
class Concrete : Material
{
public:
    Concrete() : Material(3.5,2700)
    {

    }
    virtual ~Concrete() {}
};

class Steel : Material
{
public:
    Steel() : Material(450,8050)
    {

    }
    virtual ~Steel() {}
};

class Wood : Material
{
public:
    Wood() : Material(40,750)
    {

    }
    virtual ~Wood() {}
};

class Dirt : Material //sand like density, strength is not good constant
{
public:
    Dirt() : Material(0.1,1500)
    {

    }
    virtual ~Dirt() {}
};

class Rock : Material //limestone
{
public:
    Rock() : Material(100,2600)
    {

    }
    virtual ~Rock() {}
};
*/
}
#endif
