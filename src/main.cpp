#include <string>

#include "Game.h"

int main(int argc, char **argv)
{
    bool debug = 0, gravity = 1;
    for(int i = 0; i < argc; ++i)
    {
        debug = std::string(argv[i]) == "-d";
        gravity = !(std::string(argv[i]) == "-g");
    }
    gg::MGame g;
    g.run(debug, gravity);


    return 0;
}
