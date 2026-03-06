#pragma once

#include "AudioPlayer.h"
#include "MusicRepository.h"

#include <memory>
#include <vector>
#include <print>
#include <mutex>





class MusicService {
	AudioPlayer player{};
	MusicRepository musicRepository{};
	std::optional<Song> currentSong{};

	std::mutex playerMutex;

	void playSong(Song song) {
		std::lock_guard<std::mutex> lock(playerMutex);
		std::println("Mutex aqcuired");

		currentSong = std::move(song);

		player.play_sound(currentSong->path, [&] {
			std::println("{} by {} is done playing", currentSong->name, currentSong->artist);

			playNextSong();
		});
	}

	const Song& songToPlay(const std::vector<Song>& songs) {
		if (songs.empty()) {
			throw std::exception("There are no songs to play");
		}
		if (!currentSong) {
			return songs[0];
		}
		
		const Song& currentSongPlaying = currentSong.value();

		auto it = std::find_if(songs.begin(), songs.end(), [&currentSongPlaying](const Song& song) {
			return song.artist == currentSongPlaying.artist && song.name == currentSongPlaying.name;
		});

		if (it == songs.end()) {
			return songs[0];
		}
		else {
			auto index = std::distance(songs.begin(), it);
			auto nextIndex = (index + 1) % songs.size();
			return songs[nextIndex];
		}
		
	}

	void playNextSong() {
		auto songs = musicRepository.fetchAllSongs();

		const Song& toPlay = songToPlay(songs);
		std::println("Playing {} by {}", toPlay.name, toPlay.artist);
		playSong(toPlay);
	}
	
public:
	void autoPlay() {
		playNextSong();
	}
};