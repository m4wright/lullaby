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

		std::vector<Song> result {
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
