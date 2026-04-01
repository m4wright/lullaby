#pragma once

#include "AudioPlayer.h"
#include "MusicRepository.h"
#include "Song.h"
#include "SongStatus.h"

#include <vector>
#include <optional>
#include <shared_mutex>



class MusicService {
	std::shared_mutex mtx;
	AudioPlayer player{};
	MusicRepository musicRepository;
	std::optional<Song> currentSong{};
	std::function<void(const SongStatus&)> onSongStatusChange = [](const SongStatus&) {};

	struct Helper;

	Song playNextSong(bool forward);

	void setCurrentSong(Song song) {
		std::lock_guard lock(mtx);
		currentSong = std::move(song);
	}

	// This method should be called with the lock held
	SongStatus determineSongStatus() {
		return currentSong.has_value()
			? SongStatus(currentSong->name, currentSong->artist, isPlaying())
			: SongStatus();
	}
	
public:
	MusicService(MusicRepository&& repository) : musicRepository(std::move(repository)) {}

	void setOnSongStatusChange(std::function<void(const SongStatus&)> callback) {
		std::lock_guard lock(mtx);
		onSongStatusChange = std::move(callback);
	}

	Song playNextSong() {
		return playNextSong(true);
	}

	Song playPreviousSong() {
		return playNextSong(false);
	}

	bool toggle() {
		bool isPlaying = player.toggle().get();

		std::shared_lock lock(mtx);
		if (currentSong) {
			onSongStatusChange(SongStatus{ currentSong->name, currentSong->artist, isPlaying });
		}
		else {
			onSongStatusChange(SongStatus{});
		}

		return isPlaying;
	}

	bool play(const std::string& name, const std::string& artist);

	MusicRepository& getMusicRepository() {
		return musicRepository;
	}

	std::vector<Song> getAllSongs() {
		return musicRepository.fetchAllSongs();
	}

	bool addSong(const std::string& name, const std::string& artist, const std::string& path) {
		return musicRepository.addSong(name, artist, path);
	}

	bool updateSongPath(const std::string& name, const std::string& artist, const std::string& newPath) {
		return musicRepository.updateSongPath(name, artist, newPath);
	}

	bool deleteSong(const std::string& name, const std::string& artist) {
		return musicRepository.deleteSong(name, artist);
	}

	bool isPlaying() {
		return player.isPlaying();
	}

	SongStatus getCurrentStatus() {
		std::shared_lock lock(mtx);
		return determineSongStatus();
	}
};
