#ifndef MATERIALS_H
#define MATERIALS_H

#include <cstdint>

namespace gg {

class MMaterial
{
private:
    double strength; //ultimate tensile strength in MPa
    uint_fast32_t density; //kg/m^3
public:
    double getStrength() { return strength; }
    int getDensity() { return density; }
    MMaterial(double s,int d) : strength(s), density(d) {}
    ~MMaterial() {}
};

    namespace Material {

    static MMaterial Shot = MMaterial(0,-1);
    static MMaterial Concrete = MMaterial(3.5,2700);
    static MMaterial Steel = MMaterial(450,8050);
    static MMaterial Wood = MMaterial(40,750);
    static MMaterial Dirt = MMaterial(0.1,1500);
    static MMaterial Rock = MMaterial(100,2600);

    }
}
#endif
