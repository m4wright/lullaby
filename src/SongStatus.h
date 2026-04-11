#pragma once

#include <string>

struct SongStatus {
	std::string name;
	std::string artist;
	int volume;
	bool playing;

	SongStatus(std::string name, std::string artist, bool playing, int volume)
		: name(std::move(name)), artist(std::move(artist)), playing(playing), volume(volume) {
	}

	SongStatus(int volume) : playing(false), volume(volume) {}

	SongStatus() : playing(false), volume(100) {}

	auto operator<=>(const SongStatus&) const = default;
};