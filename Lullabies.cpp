#include "MusicController.h"
#include "MusicService.h"


#include <cstdlib>

int main(int argc, char** argv)
{
    
    MusicService musicPlayer{};

    int port = 9095;

    if (argc > 1) {
	port = atoi(argv[1]);
    }

    startServer(&musicPlayer, port);
}
