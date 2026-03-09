#pragma once

#include "AudioPlayer.h"
#include "MusicRepository.h"
#include "Song.h"

#include <vector>
#include <optional>
#include <shared_mutex>




class MusicService {
	std::shared_mutex mtx;
	AudioPlayer player{};
	MusicRepository musicRepository;
	std::optional<Song> currentSong{};

	struct Helper;

	Song playNextSong(bool forward);

	void setCurrentSong(Song song) {
		std::lock_guard lock(mtx);
		currentSong = std::move(song);
	}
	
public:

	MusicService(MusicRepository&& repository) : musicRepository(std::move(repository)) {}

	Song playNextSong() {
		return playNextSong(true);
	}

	Song playPreviousSong() {
		return playNextSong(false);
	}
	

	void autoPlay() {
		playNextSong();
	}

	void pause() {
		player.pause();
	}

	void resume() {
		player.resume();
	}

	std::string_view toggle() {
		return player.toggle();
	}

	bool play(std::string_view name, std::string_view artist);

	MusicRepository& getMusicRepository() {
		return musicRepository;
	}

	std::vector<Song> getAllSongs() {
		return musicRepository.fetchAllSongs();
	}

	const std::optional<Song>& getCurrentSong() {
		std::shared_lock lock(mtx);
		return currentSong;
	}

	bool isPlaying() {
		return player.isPlaying();
	}
};
