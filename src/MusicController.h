#pragma once


#include "MusicService.h"
#include <string>



void startServer(MusicService& musicService, int port, const std::string& mount_point);
