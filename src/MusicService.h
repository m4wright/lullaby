#pragma once

#include "AudioPlayer.h"
#include "MusicRepository.h"
#include "Song.h"
#include "SongStatus.h"

#include <vector>
#include <optional>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <optional>

struct TimerState {
	bool enabled = false;
	int duration_minutes = 15;
	std::optional<int> remaining_seconds = std::nullopt;
};

class MusicService {
	std::shared_mutex mtx;
	AudioPlayer player{};
	MusicRepository musicRepository;
	std::optional<Song> currentSong{};
	std::function<void(const SongStatus&)> onSongStatusChange = [](const SongStatus&) {};

	// Timer state - uses atomic for enabled flag, mutex-protected for rest
	std::atomic<bool> timerEnabled{false};
	mutable std::shared_mutex timerMtx;
	TimerState timerState{};
	std::thread timerThread;
	std::atomic<bool> timerThreadRunning{false};

	void startTimerThread();
	void resetTimer();

	struct Helper;

	Song playNextSong(bool forward);

	void setCurrentSong(Song song) {
		std::lock_guard lock(mtx);
		currentSong = std::move(song);
	}

	// This method should be called with the lock held
	SongStatus determineSongStatus() {
		return currentSong.has_value()
			? SongStatus(currentSong->name, currentSong->artist, isPlaying(), getVolume())
			: SongStatus(getVolume());
	}
	
public:
	MusicService(MusicRepository&& repository) : musicRepository(std::move(repository)) {
#ifndef UNIT_TEST
		startTimerThread();
#endif
	}

	void setOnSongStatusChange(std::function<void(const SongStatus&)> callback) {
		std::lock_guard lock(mtx);
		onSongStatusChange = std::move(callback);
	}

	Song playNextSong() {
		resetTimer();
		return playNextSong(true);
	}

	Song playPreviousSong() {
		resetTimer();
		return playNextSong(false);
	}

	bool toggle() {
		bool isPlaying = player.toggle().get();

		resetTimer();

		std::shared_lock lock(mtx);
		if (currentSong) {
			onSongStatusChange(SongStatus{ currentSong->name, currentSong->artist, isPlaying, getVolume()});
		}
		else {
			onSongStatusChange(SongStatus{getVolume()});
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

	void setVolume(int volume) {
		player.setVolume(static_cast<float>(volume) / 100.0f);

		std::shared_lock lock(mtx);
		onSongStatusChange(determineSongStatus());
	}

	int getVolume() {
		return static_cast<int>(player.getVolume() * 100.0f);
	}

	// Timer methods
	bool isTimerEnabled() const { return timerEnabled.load(); }
	TimerState getTimerState() const;
	void setTimerEnabled(bool enabled);
	void setTimerDuration(int minutes);
	TimerState getTimerStateAndResetIfActive() const;
};
