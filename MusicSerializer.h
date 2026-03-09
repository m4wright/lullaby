#pragma once

#include "Song.h"

#include <string>
#include <vector>

struct SongStatus {
	std::string name;
	std::string artist;
	bool playing;

	SongStatus(std::string name, std::string artist, bool playing)
		: name(std::move(name)), artist(std::move(artist)), playing(playing) {
	}

	SongStatus(): name(""), artist(""), playing(false) {}
};

std::string to_string(const std::vector<Song>& songs);
std::string to_string(const SongStatus& songStatus);
std::string to_string(const std::vector<Song>& songs, const SongStatus& songStatus);
