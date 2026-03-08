#pragma once

#include "AudioPlayer.h"
#include "MusicRepository.h"
#include "Song.h"

#include <vector>
#include <optional>




class MusicService {
	AudioPlayer player{};
	MusicRepository musicRepository;
	std::optional<Song> currentSong{};

	struct Helper;

	Song playNextSong(bool forward);
	
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
};
