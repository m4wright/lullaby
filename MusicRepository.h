#pragma once

#include "Song.h"

#include <vector>


class MusicRepository {
public:
	std::vector<Song> fetchAllSongs() const;
};
