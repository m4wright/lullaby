#include "MusicController.h"
#include "MusicService.h"


#include <string>
#include <cstdlib>


int determine_port(int argc, char** argv) {
    if (argc > 1) {
        return atoi(argv[1]);
    }
    else {
        return 9096;
    }
}

std::string determine_base_path(int argc, char** argv) {
    if (argc > 2) {
        return argv[2];
    }
    else {
#ifdef _WIN32
        return "C:\\Users\\m4_wr\\source\\repos\\Lullabies\\Lullabies";
#endif

#ifdef linux
        return "/home/mathew/Documents/lullaby";
#endif
    }
}


std::string determine_mount_point(int argc, char** argv) {
    return determine_base_path(argc, argv) + "/static";
}

std::string determine_db_path(int argc, char** argv) {
    return determine_base_path(argc, argv) + "/db/music.db";
}

int main(int argc, char** argv)
{
    MusicService musicPlayer{ MusicRepository{determine_db_path(argc, argv)} };

	int port = determine_port(argc, argv);
    std::string mount_point = determine_mount_point(argc, argv);

    startServer(musicPlayer, port, mount_point);
}
