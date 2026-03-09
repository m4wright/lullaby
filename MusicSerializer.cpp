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