#include "MusicService.h"

#include <print>
#include <vector>
#include <mutex>

TimerState MusicService::getTimerState() const {
	std::shared_lock lock(timerMtx);
	return timerState;
}

void MusicService::setTimerEnabled(bool enabled) {
	timerEnabled.store(enabled);
	{
		std::unique_lock lock(timerMtx);
		timerState.enabled = enabled;
		if (enabled && timerState.remaining_seconds == std::nullopt) {
			timerState.remaining_seconds = timerState.duration_minutes * 60;
		} else if (!enabled) {
			timerState.remaining_seconds = std::nullopt;
		}
	}
	resetTimer();
}

void MusicService::setTimerDuration(int minutes) {
	{
		std::unique_lock lock(timerMtx);
		timerState.duration_minutes = minutes;
		if (timerState.enabled && timerState.remaining_seconds == std::nullopt) {
			timerState.remaining_seconds = minutes * 60;
		}
	}
	resetTimer();
}

void MusicService::resetTimer() {
	std::unique_lock lock(timerMtx);
	if (timerState.enabled) {
		timerState.remaining_seconds = timerState.duration_minutes * 60;
	}
}

void MusicService::startTimerThread() {
	timerThreadRunning.store(true);
	timerThread = std::thread([this]() {
		while (timerThreadRunning.load()) {
			std::this_thread::sleep_for(std::chrono::seconds(1));

			if (timerEnabled.load()) {
				std::unique_lock lock(timerMtx);
				if (timerState.enabled && timerState.remaining_seconds.has_value() && timerState.remaining_seconds.value() > 0) {
					timerState.remaining_seconds = timerState.remaining_seconds.value() - 1;

					if (timerState.remaining_seconds.value() == 0) {
						// Timer expired - pause playback
						timerState.enabled = false;
						timerState.remaining_seconds = std::nullopt;
						timerEnabled.store(false);

						// Pause the player
						if (player.isPlaying()) {
							player.toggle();
							onSongStatusChange(SongStatus{getVolume()});
						}
					}
				}
			}
		}
	});
}

struct MusicService::Helper {
	static void playSong(MusicService& self, const Song& song) {
		std::function<void(const SongStatus&)> callback;

		{
			std::lock_guard lock(self.mtx);
			self.currentSong = song;
			callback = self.onSongStatusChange;

			std::println("Playing {} by {}", song.name, song.artist);

			self.player.playSound(song.path, [&] {
				{
					std::shared_lock lock(self.mtx);
					std::println("{} by {} is done playing", self.currentSong->name, self.currentSong->artist);
				}

				self.playNextSong();
			});
		}

		callback(SongStatus{ song.name, song.artist, true, self.getVolume() });
	}

	static Song songToPlay(MusicService& self, const std::vector<Song>& songs, bool forward) {
		if (songs.empty()) {
			throw std::runtime_error("There are no songs to play");
		}

		std::shared_lock lock(self.mtx);

		if (!self.currentSong) {
			return songs[0];
		}

		const Song& currentSongPlaying = self.currentSong.value();

		auto it = std::find_if(songs.begin(), songs.end(), [&currentSongPlaying](const Song& song) {
			return song.artist == currentSongPlaying.artist && song.name == currentSongPlaying.name;
		});

		if (it == songs.end()) {
			return songs[0];
		}
		else {
			int index = static_cast<int>(std::distance(songs.begin(), it));

			int direction = forward ? 1 : -1;
			int size = static_cast<int>(songs.size());

			int nextIndex = (((index + direction) % size) + size) % size;
			return songs[nextIndex];
		}
	}
};

Song MusicService::playNextSong(bool forward) {
	auto songs = musicRepository.fetchAllSongs();

	Song toPlay = Helper::songToPlay(*this, songs, forward);
	Helper::playSong(*this, toPlay);

	return toPlay;
}

bool MusicService::play(const std::string& name, const std::string& artist) {
	std::optional<Song> song = musicRepository.fetchSong(name, artist);

	if (song) {
		Helper::playSong(*this, song.value());
		return true;
	}
	else {
		std::println("Song {} by {} not found", name, artist);
		return false;
	}
}