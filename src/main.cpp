#include <string>

#include "Game.h"

int main(int argc, char **argv)
{
    bool debug = 0;
    if(argc > 1)
    {
        debug = std::string(argv[1]) == "-d";
    }
    gg::MGame g;
    g.run(debug);


    return 0;
}
