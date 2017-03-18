#include <string>

#include "Game.h"

int main(int argc, char** argv)
{
    bool debug;
    for (int i = 0; i < argc; ++i)
    {
            debug = std::string(argv[i]) == "-d";
    }
        gg::MGame g;
        g.Run(debug);


return 0;
}

/*
 * 1. Load all the in-game rigid body object as instances of Object class
 * 2. Split every object into EDEM elements and store them
 * 3. Get textures for edems
 * 4. Give Objects to Bullet and start the simulation
 * 5. Detect collision
 *      6. Replace Object with his edem elements in Bullet
 *      7. Perform Edem simulation, debre generation and dust simulation
 *      8. After the edems are no longer in motion, give them to other thread to put them back into Objects
 */
