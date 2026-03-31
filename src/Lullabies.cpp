#include "MusicController.h"
#include "MusicService.h"


#include <string>
#include <cstdlib>

namespace {
    int determine_port(int argc, char** argv) {
        return atoi(argv[1]);
    }

    const std::string& determine_base_path(int argc, char** argv) {
        static std::string basePath{};

        if (basePath.empty()) {
            basePath = argv[2];
        }

        return basePath;
    }


    std::string determine_mount_point(int argc, char** argv) {
        return determine_base_path(argc, argv) + "/static";
    }

    std::string determine_db_path(int argc, char** argv) {
        return determine_base_path(argc, argv) + "/db/music.db";
    }
}

int main(int argc, char** argv)
{
    if (argc <= 2) {
        throw std::runtime_error("Required arguments: <port> <base path>");
    }
    MusicService musicPlayer{ MusicRepository{determine_db_path(argc, argv)} };

	int port = determine_port(argc, argv);
    std::string mount_point = determine_mount_point(argc, argv);

    startServer(musicPlayer, port, mount_point);
}
