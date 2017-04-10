#include "Material.h"

namespace gg {
//strength, density, fragmentation
const MMaterial MMaterial::Concrete = MMaterial(600,27,0.05);
const MMaterial MMaterial::Shot = MMaterial(0,-1,0); //not breakable
const MMaterial MMaterial::Steel = MMaterial(100000,80,0.7);
const MMaterial MMaterial::Wood = MMaterial(200,75,0.7);
const MMaterial MMaterial::Dirt = MMaterial(30,15,0.05);
const MMaterial MMaterial::Rock = MMaterial(600,27,0.05);
const MMaterial MMaterial::Magic = MMaterial(0,0,0); //indestructible
}
