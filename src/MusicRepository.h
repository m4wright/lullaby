#pragma once

#include "Song.h"

#include <vector>
#include <memory>
#include <string>
#include <optional>


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
	std::optional<Song> fetchSong(const std::string & name, const std::string & artist);

	bool addSong(const std::string& name, const std::string& artist, const std::string& path);
	bool updateSongPath(const std::string& name, const std::string& artist, const std::string& newPath);
	bool deleteSong(const std::string& name, const std::string& artist);
};
