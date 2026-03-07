#include "MusicController.h"
#include "MusicService.h"

#include <print>



int main(int argc, char** argv)
{
    
    MusicService musicPlayer{};

    std::thread serverThread(startServer, &musicPlayer);

    musicPlayer.autoPlay();

    for (;;) {
		char c = getchar();

        if (c == 'p') {
			musicPlayer.pause();
		}
        else if (c == 'r') {
            musicPlayer.resume();
        }
        else if (c == 'q')
        {
			break;
        }
    }
}
