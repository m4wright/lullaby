#include "MusicSerializer.h"

#include "third_party/json.hpp"

void to_json(nlohmann::json& j, const Song& song) {
	j["name"] = song.name;
	j["artist"] = song.artist;
}

std::string to_string(const std::vector<Song>& songs) {
	nlohmann::json result = songs;
	return result.dump();
}

void to_json(nlohmann::json& j, const SongStatus& songStatus) {
	if (!songStatus.name.empty() && !songStatus.artist.empty()) {
		j["name"] = songStatus.name;
		j["artist"] = songStatus.artist;
	}

	j["playing"] = songStatus.playing;
}

std::string to_string(const SongStatus& songStatus) {
	nlohmann::json result = songStatus;
	return result.dump();
}

std::string to_string(const std::vector<Song>& songs, const SongStatus& songStatus) {
	nlohmann::json result;
	result["songs"] = songs;
	result["now_playing"] = songStatus;

	return result.dump();
}