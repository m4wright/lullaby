#include "MusicController.h"
#include "MusicService.h"


#include <string>
#include <cstdlib>


int determine_port(int argc, char** argv) {
    if (argc > 1) {
        return atoi(argv[1]);
    }
    else {
        return 9095;
    }
}


std::string determine_mount_point(int argc, char** argv) {
    if (argc > 2) {
        return argv[2];
    }
    else {

#ifdef _WIN32
        return "C:\\Users\\m4_wr\\source\\repos\\Lullabies\\Lullabies\\static";
#endif

#ifdef linux
		return "/home/mathew/Documents/lullaby/static";
#endif
    }
}

int main(int argc, char** argv)
{
    
    MusicService musicPlayer{};

	int port = determine_port(argc, argv);
    std::string mount_point = determine_mount_point(argc, argv);

    startServer(musicPlayer, port, mount_point);
}
