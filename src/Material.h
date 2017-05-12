#ifndef MATERIALS_H
#define MATERIALS_H

#include <cstdint>

namespace gg {

class MMaterial
{
public:
    double getStrength() const { return strength; }
    int getDensity() const { return density; }
    float getFragmentation() const { return fragment_size; }
    MMaterial(double s,int d, float f) : strength(s), density(d), fragment_size(f) {}
    MMaterial() : strength(0), density(0), fragment_size(0) {}
    ~MMaterial() {}
    static const MMaterial Shot;
    static const MMaterial Concrete;
    static const MMaterial Steel;
    static const MMaterial Wood;
    static const MMaterial Dirt;
    static const MMaterial Rock;
    static const MMaterial Magic;
    static const MMaterial Ship;

    static const MMaterial* getMaterial(char m)
    {
        switch (m) {
        case 'C':
            return &Concrete;
        case 'S':
            return &Steel;
        case 'W':
            return &Wood;
        case 'D':
            return &Dirt;
        case 'R':
            return &Rock;
        case 'N':
            return &Ship;
        default : return &Magic;
        }
    }

private:
    double strength;
    uint_fast32_t density;
    float fragment_size;
};
}
#endif
