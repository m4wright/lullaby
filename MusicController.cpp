#include "MusicController.h"
#include "MusicSerializer.h"

#include "third_party/httplib.h"

#include <string>
#include <print>

SongStatus determineStatus(const MusicService& musicService) {
	const std::optional<Song>& currentSongOpt = musicService.getCurrentSong();
	return currentSongOpt.has_value()
		? SongStatus(currentSongOpt->name, currentSongOpt->artist, musicService.isPlaying())
		: SongStatus();
}

void startServer(MusicService& musicService, int port, const std::string& mount_point) {
	httplib::Server server;

	server.set_mount_point("/", mount_point);

	server.Get("/music", [&musicService](const httplib::Request&, httplib::Response& response) {
		std::vector<Song> songs = musicService.getAllSongs();
		SongStatus status = determineStatus(musicService);

		response.set_content(to_string(songs, status), "application/json");
	});

	server.Get("/music/now-playing", [&musicService](const httplib::Request&, httplib::Response& response) {
		SongStatus status = determineStatus(musicService);
		response.set_content(to_string(status), "application/json");
	});

	server.Get("/music/media/play", [&musicService](const httplib::Request& request, httplib::Response& response) {
		if (!(request.has_param("name") && request.has_param("artist"))) {
			response.set_content("Missing parameters", "text/plain");
			response.status = 400;
			return;
		}

		std::string name = request.get_param_value("name");
		std::string artist = request.get_param_value("artist");

		bool playing = musicService.play(name, artist);

		if (playing) {
			response.set_content("Playing " + name + " by " + artist, "text/plain");
		}
		else {
			response.set_content("Could not find " + name + " by " + artist, "text/plain");
			response.status = 400;
		}
	});

	server.Get("/music/media/toggle_pause", [&musicService](const httplib::Request& request, httplib::Response& response) {
		response.set_content(std::string(musicService.toggle()), "text/plain");
	});

	server.Get("/music/media/play_next", [&musicService](const httplib::Request& request, httplib::Response& response) {
		Song song = musicService.playNextSong();
		response.set_content("Playing " + song.name + " by " + song.artist, "text/plain");
	});

	server.Get("/music/media/play_previous", [&musicService](const httplib::Request& request, httplib::Response& response) {
		Song song = musicService.playPreviousSong();
		response.set_content("Playing " + song.name + " by " + song.artist, "text/plain");
	});


	std::println("Starting to listen on port {}", port);
	server.listen("0.0.0.0", port);
}