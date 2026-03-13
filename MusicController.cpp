#include "MusicController.h"
#include "MusicSerializer.h"

#include "third_party/httplib.h"

#include <string>
#include <print>
#include <shared_mutex>
#include <condition_variable>


class MusicStatusUpdater {
	std::shared_mutex mtx{};
	std::condition_variable_any cv{};

	std::string currentStatusStr = "data: " + to_string(SongStatus{}) + "\n\n";

public:
	bool waitForUpdate(httplib::DataSink& sink) {
		std::string status;

		{
			std::shared_lock lock(mtx);
			cv.wait(lock);
			status = currentStatusStr;
		}


		if (!sink.is_writable()) {
			return false;
		}
		if (!sink.write(status.data(), status.size())) {
			return false;
		}

		return true;
	}

	void updateStatus(const SongStatus& status) {
		std::unique_lock lock(mtx);
		currentStatusStr = "data: " + to_string(status) + "\n\n";
		cv.notify_all();
	}
};

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

	server.Get("/exit", [&server](const httplib::Request&, httplib::Response& response) {
		response.set_content("Shutting down server", "text/plain");
		std::thread([&server]{
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::abort();
		}).detach();
	});


	std::println("Starting to listen on port {}", port);
	server.listen("0.0.0.0", port);
}