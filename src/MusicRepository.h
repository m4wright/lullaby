#pragma once

#include "Song.h"

#include <vector>
#include <memory>
#include <string>


class MusicRepository {
	struct Impl;
	std::unique_ptr<Impl> impl;

public:
 MusicRepository(const std::string& dbPath);
	~MusicRepository();

	MusicRepository(const MusicRepository&) = delete;
	MusicRepository& operator=(const MusicRepository&) = delete;

   MusicRepository(MusicRepository&&) noexcept;
	MusicRepository& operator=(MusicRepository&&) noexcept;

	std::vector<Song> fetchAllSongs();
};
