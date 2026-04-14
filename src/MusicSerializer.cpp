#include "MusicSerializer.h"
#include "SongStatus.h"

#include "third_party/json.hpp"

void to_json(nlohmann::json& j, const Song& song) {
	j["name"] = song.name;
	j["artist"] = song.artist;
}

std::string to_string(const std::vector<Song>& songs) {
	nlohmann::json result = songs;
	return result.dump();
}

std::string to_string_with_path(const std::vector<Song>& songs) {
	nlohmann::json j = nlohmann::json::array();
	for (const auto& song : songs) {
		nlohmann::json songJson;
		songJson["name"] = song.name;
		songJson["artist"] = song.artist;
		songJson["path"] = song.path;
		j.push_back(songJson);
	}
	return j.dump();
}

void to_json(nlohmann::json& j, const SongStatus& songStatus) {
	if (!songStatus.name.empty() && !songStatus.artist.empty()) {
		j["name"] = songStatus.name;
		j["artist"] = songStatus.artist;
	}

	j["event"] = "message";
	j["playing"] = songStatus.playing;
	j["volume"] = songStatus.volume;
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

std::string to_string(const SongStatus& songStatus, const TimerState& timerState) {
	nlohmann::json result = songStatus;

	nlohmann::json timerJson;
	timerJson["enabled"] = timerState.enabled;
	timerJson["duration_minutes"] = timerState.duration_minutes;
	timerJson["remaining_seconds"] = timerState.remaining_seconds;
	result["timer"] = timerJson;

	return result.dump();
}

std::string to_string(const std::vector<Song>& songs, const SongStatus& songStatus, const TimerState& timerState) {
	nlohmann::json result;
	result["songs"] = songs;
	result["now_playing"] = nlohmann::json::parse(to_string(songStatus, timerState));

	return result.dump();
}

std::string to_volume_string(int volume) {
	nlohmann::json result;
	result["volume"] = volume;
	return result.dump();
}