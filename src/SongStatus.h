#pragma once

#include <string>

struct SongStatus {
	std::string name;
	std::string artist;
	bool playing;

	SongStatus(std::string name, std::string artist, bool playing)
		: name(std::move(name)), artist(std::move(artist)), playing(playing) {
	}

	SongStatus() : name(""), artist(""), playing(false) {}

	auto operator<=>(const SongStatus&) const = default;
};