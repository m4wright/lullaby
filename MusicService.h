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

		std::println("Playing {} by {}", currentSong->name, currentSong->artist);

		player.play_sound(currentSong->path, [&] {
			std::println("{} by {} is done playing", currentSong->name, currentSong->artist);

			playNextSong();
		});
	}

	const Song& songToPlay(const std::vector<Song>& songs, bool forward) {
		if (songs.empty()) {
			throw std::runtime_error("There are no songs to play");
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

			int direction = forward ? 1 : -1;
			auto size = songs.size();

			auto nextIndex = (((index + direction) % size) + size) % size;
			return songs[nextIndex];
		}
		
	}

	Song playNextSong(bool forward) {
		auto songs = musicRepository.fetchAllSongs();

		const Song& toPlay = songToPlay(songs, forward);
		Song returnSong = toPlay;
		playSong(toPlay);

		return returnSong;
	}
	
public:

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

	bool play(std::string_view name, std::string_view artist) {
		auto songs = musicRepository.fetchAllSongs();
		auto it = std::find_if(songs.begin(), songs.end(), [&name, &artist](const Song& song) {
			return song.artist == artist && song.name == name;
		});
		if (it != songs.end()) {
			playSong(*it);
			return true;
		}
		else {
			std::println("Song {} by {} not found", name, artist);
			return false;
		}
	}

	MusicRepository& getMusicRepository() {
		return musicRepository;
	}

	std::vector<Song> getAllSongs() const {
		return musicRepository.fetchAllSongs();
	}
};
