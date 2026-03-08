#include "MusicController.h"

#include "json.hpp"
#include "httplib.h"

#include <string>


void to_json(nlohmann::json& j, const Song& song) {
	j["name"] = song.name;
	j["artist"] = song.artist;
}


void startServer(MusicService* musicService, int port) {
	httplib::Server server;

	//server.set_mount_point("/", "/home/mathew/Documents/lullaby/static");
	server.set_mount_point("/", "C:\\Users\\m4_wr\\source\\repos\\Lullabies\\Lullabies\\static");

	using json = nlohmann::json;

	server.Get("/music", [&musicService](const httplib::Request&, httplib::Response& response) {
		auto songs = musicService->getAllSongs();

		json result = songs;

		response.set_content(result.dump(), "application/json");
		});

	server.Get("/music/media/play", [&musicService](const httplib::Request& request, httplib::Response& response) {
		if (!(request.has_param("name") && request.has_param("artist"))) {
			response.set_content("Missing parameters", "text/plain");
			response.status = 400;
			return;
		}

		std::string name = request.get_param_value("name");
		std::string artist = request.get_param_value("artist");

		bool playing = musicService->play(name, artist);

		if (playing) {
			response.set_content("Playing " + name + " by " + artist, "text/plain");
		}
		else {
			response.set_content("Could not find " + name + " by " + artist, "text/plain");
			response.status = 400;
		}
		});

	server.Get("/music/media/toggle_pause", [&musicService](const httplib::Request& request, httplib::Response& response) {
		response.set_content(std::string(musicService->toggle()), "text/plain");
		});

	server.Get("/music/media/play_next", [&musicService](const httplib::Request& request, httplib::Response& response) {
		Song song = musicService->playNextSong();
		response.set_content("Playing " + song.name + " by " + song.artist, "text/plain");
		});

	server.Get("/music/media/play_previous", [&musicService](const httplib::Request& request, httplib::Response& response) {
		Song song = musicService->playPreviousSong();
		response.set_content("Playing " + song.name + " by " + song.artist, "text/plain");
		});

	std::println("Starting to listen on port {}", port);
	server.listen("0.0.0.0", port);
}