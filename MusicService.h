#pragma once

#include "AudioPlayer.h"
#include "MusicRepository.h"

#include <memory>
#include <vector>
#include <print>





class MusicService {
	AudioPlayer player{};
	MusicRepository musicRepository{};
	std::optional<Song> currentSong{};

	void playSong(Song song) {
		currentSong = std::move(song);

		player.play_sound(currentSong->path, [](AudioPlayer& player) {
			std::printf("Song is done playing\n");
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
			int index = std::distance(songs.begin(), it);
			int nextIndex = (index + 1) % songs.size();
			return songs[nextIndex];
		}
		
	}
	
public:
	void autoPlay() {
		auto songs = musicRepository.fetchAllSongs();
		
		const Song& toPlay = songToPlay(songs);
		std::println("Playing {} by {}", toPlay.name, toPlay.artist);
		playSong(toPlay);
	}
};