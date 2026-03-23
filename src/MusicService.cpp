#include "MusicService.h"

#include <print>
#include <vector>

struct MusicService::Helper {
	static void playSong(MusicService& self, const Song& song) {
		std::lock_guard lock(self.mtx);
		self.currentSong = song;

		std::println("Playing {} by {}", song.name, song.artist);
		
        self.player->play_sound(song.path, [&] {
			{
				std::shared_lock lock(self.mtx);
				std::println("{} by {} is done playing", self.currentSong->name, self.currentSong->artist);
			}

			self.playNextSong();
		});

		self.onSongStatusChange(SongStatus{song.name, song.artist, true});
	}

	static const Song& songToPlay(MusicService& self, const std::vector<Song>& songs, bool forward) {
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
			auto index = std::distance(songs.begin(), it);

			int direction = forward ? 1 : -1;
			auto size = songs.size();

			auto nextIndex = (((index + direction) % size) + size) % size;
			return songs[nextIndex];
		}
	}
};

Song MusicService::playNextSong(bool forward) {
	auto songs = musicRepository.fetchAllSongs();

	const Song& toPlay = Helper::songToPlay(*this, songs, forward);
	Song returnSong = toPlay;
	Helper::playSong(*this, toPlay);

	return returnSong;
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