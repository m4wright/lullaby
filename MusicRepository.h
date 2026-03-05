#pragma once

#include <string>
#include <algorithm>
#include <vector>

struct Song {
	std::string name;
	std::string artist;
	std::string path;
};


class MusicRepository {
public:
	std::vector<Song> fetchAllSongs() {
		std::vector<Song> result {
			{.name = "Harvard", .artist = "Harvard", .path = "C:\\Users\\m4_wr\\Music\\localmusic\\harvard.mp3"},
			{.name = "Complicated", .artist = "Mac Miller", .path = "C:\\Users\\m4_wr\\Music\\localmusic\\Mac Miller\\02. Complicated.mp3"},
			{.name = "Circles", .artist = "Mac Miller", .path = "C:\\Users\\m4_wr\\Music\\localmusic\\Mac Miller\\01. Circles.mp3"},
		};

		std::sort(result.begin(), result.end(), [](const Song& a, const Song& b) {
			if (a.artist != b.artist) {
				return a.artist < b.artist;
			}

			return a.name < b.name;
		});

		return result;
	}

};