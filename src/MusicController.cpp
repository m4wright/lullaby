#include "MusicController.h"
#include "MusicSerializer.h"
#include "MusicStatusUpdater.h"

#include "third_party/httplib.h"

#include <string>
#include <print>


static_assert(WritableSink<httplib::DataSink>, "httplib::DataSink matches the WritableSink concept");


static MusicStatusUpdater statusUpdater{};


void startServer(MusicService& musicService, int port, const std::string& mount_point) {
	musicService.setOnSongStatusChange([](const SongStatus& status) {
		statusUpdater.updateStatus(status);
	});

	httplib::Server server;

	server.set_mount_point("/", mount_point);

	server.Get("/music", [&musicService](const httplib::Request&, httplib::Response& response) {
		std::vector<Song> songs = musicService.getAllSongs();
		SongStatus status = musicService.getCurrentStatus();

		response.set_content(to_string(songs, status), "application/json");
	});

	server.Get("/music/now-playing", [&musicService](const httplib::Request&, httplib::Response& response) {
		SongStatus status = musicService.getCurrentStatus();
		response.set_content(to_string(status), "application/json");
	});

	server.Get("/music/subscribe/now-playing", [&musicService](const httplib::Request&, httplib::Response& response) {
		response.set_header("Content-Type", "text/event-stream");
		response.set_header("Cache-Control", "no-cache");
		response.set_header("Connection", "keep-alive");

		response.set_chunked_content_provider("text/event-stream", [&musicService](size_t offset, httplib::DataSink& sink) {
			return statusUpdater.waitForUpdate(sink);
		});
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
		bool isPlaying = musicService.toggle();
		response.set_content(isPlaying ? "Song resumed" : "Song paused", "text/plain");
	});

	server.Get("/music/media/play_next", [&musicService](const httplib::Request& request, httplib::Response& response) {
		Song song = musicService.playNextSong();
		response.set_content("Playing " + song.name + " by " + song.artist, "text/plain");
	});

	server.Get("/music/media/play_previous", [&musicService](const httplib::Request& request, httplib::Response& response) {
		Song song = musicService.playPreviousSong();
		response.set_content("Playing " + song.name + " by " + song.artist, "text/plain");
	});

	server.Get("/admin/exit", [&server](const httplib::Request&, httplib::Response& response) {
		response.set_content("Shutting down server", "text/plain");
		std::thread([&server]{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::abort();
		}).detach();
	});

	// Admin API - Song Management
	server.Get("/admin/songs", [&musicService](const httplib::Request&, httplib::Response& response) {
		std::vector<Song> songs = musicService.getAllSongs();
		response.set_content(to_string_with_path(songs), "application/json");
	});

	server.Post("/admin/songs", [&musicService](const httplib::Request& request, httplib::Response& response) {
		if (!(request.has_param("name") && request.has_param("artist") && request.has_param("path"))) {
			response.set_content("Missing required fields: name, artist, path", "text/plain");
			response.status = 400;
			return;
		}

		std::string name = request.get_param_value("name");
		std::string artist = request.get_param_value("artist");
		std::string path = request.get_param_value("path");

		bool added = musicService.addSong(name, artist, path);
		if (added) {
			response.set_content("Song added successfully", "text/plain");
		} else {
			response.set_content("Failed to add song (may already exist)", "text/plain");
			response.status = 409;
		}
	});

	server.Put("/admin/songs", [&musicService](const httplib::Request& request, httplib::Response& response) {
		if (!(request.has_param("name") && request.has_param("artist") && request.has_param("path"))) {
			response.set_content("Missing required fields: name, artist, path", "text/plain");
			response.status = 400;
			return;
		}

		std::string name = request.get_param_value("name");
		std::string artist = request.get_param_value("artist");
		std::string path = request.get_param_value("path");

		bool updated = musicService.updateSongPath(name, artist, path);
		if (updated) {
			response.set_content("Song updated successfully", "text/plain");
		} else {
			response.set_content("Song not found", "text/plain");
			response.status = 404;
		}
	});

	server.Delete("/admin/songs", [&musicService](const httplib::Request& request, httplib::Response& response) {
		if (!(request.has_param("name") && request.has_param("artist"))) {
			response.set_content("Missing required fields: name, artist", "text/plain");
			response.status = 400;
			return;
		}

		std::string name = request.get_param_value("name");
		std::string artist = request.get_param_value("artist");

		bool deleted = musicService.deleteSong(name, artist);
		if (deleted) {
			response.set_content("Song deleted successfully", "text/plain");
		} else {
			response.set_content("Song not found", "text/plain");
			response.status = 404;
		}
	});


	std::println("Starting to listen on port {}", port);
	server.listen("0.0.0.0", port);
}