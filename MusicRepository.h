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
	std::vector<Song> fetchAllSongs() const {

		// TODO: This is a very hacky solution until the list of songs are moved to sqlite db.
		std::vector<Song> result {
#ifdef linux
			{.name = "Estrellita Guardiana", .artist = "Canciones de Cuna para Soñar", .path = "/home/mathew/Music/New/10 Estrellita Guardiana.mp3"},
			{.name = "Melodía de Ángel", .artist = "Canciones de Cuna para Soñar", .path = "/home/mathew/Music/New/05 Melodía de Ángel.mp3"},
			{.name = "El Viaje del Osito", .artist = "Canciones de Cuna para Soñar", .path = "/home/mathew/Music/New/07 El Viaje del Osito.mp3"},
			{.name = "Susurros de la Luna", .artist = "Canciones de Cuna para Soñar", .path = "/home/mathew/Music/New/09 Susurros de la Luna.mp3"},
			{.name = "El Bosque que Duerme", .artist = "Canciones de Cuna para Soñar", .path = "/home/mathew/Music/New/03 El Bosque que Duerme.mp3"},
			{.name = "Cuna de Luz", .artist = "Canciones de Cuna para Soñar", .path = "/home/mathew/Music/New/06 Cuna de Luz.mp3"},
			{.name = "Caricias de Nube", .artist = "Canciones de Cuna para Soñar", .path = "/home/mathew/Music/New/08 Caricias de Nube.mp3"},
			{.name = "Canción del Abrazo", .artist = "Canciones de Cuna para Soñar", .path = "/home/mathew/Music/New/02 Canción del Abrazo.mp3"},
			{.name = "Sueño de Mariposa", .artist = "Canciones de Cuna para Soñar", .path = "/home/mathew/Music/New/01 Sueño de Mariposa.mp3"},
			{.name = "Besitos de Algodón", .artist = "Canciones de Cuna para Soñar", .path = "/home/mathew/Music/New/04 Besitos de Algodón.mp3"}
#endif

#ifdef _WIN32

			{.name = "Long Chirp", .artist = "A Test", .path = "C:\\Users\\m4_wr\\Downloads\\longchirp-88445.mp3"},
			{.name = "Harvard", .artist = "Harvard", .path = "C:\\Users\\m4_wr\\Music\\localmusic\\harvard.mp3"},
			{.name = "Complicated", .artist = "Mac Miller", .path = "C:\\Users\\m4_wr\\Music\\localmusic\\Mac Miller\\02. Complicated.mp3"},
			{.name = "Circles", .artist = "Mac Miller", .path = "C:\\Users\\m4_wr\\Music\\localmusic\\Mac Miller\\01. Circles.mp3"}

#endif
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
