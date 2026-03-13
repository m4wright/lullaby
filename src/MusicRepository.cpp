#include "MusicRepository.h"

#include "third_party/sqlite3.h"

#include <algorithm>
#include <stdexcept>
#include <print>

MusicRepository::~MusicRepository() = default;

MusicRepository::MusicRepository(MusicRepository&&) noexcept = default;
MusicRepository& MusicRepository::operator=(MusicRepository&&) noexcept = default;


struct MusicRepository::Impl {
	std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db{ nullptr, sqlite3_close };

	Impl(const std::string& dbPath) {
		sqlite3* rawDb = nullptr;
		if (sqlite3_open(dbPath.c_str(), &rawDb) != SQLITE_OK) {
			throw std::runtime_error("Failed to open database");
		}

		db.reset(rawDb);
	}

	// TODO: Query for the song directly instead of fetching all songs and filtering in memory
	std::optional<Song> fetchSong(const std::string& name, const std::string& artist) {
		for (const Song& song : fetchAllSongs()) {
			if (song.name == name && song.artist == artist) {
				return song;
			}
		}

		return std::nullopt;
	}

	std::vector<Song> fetchAllSongs() {
		char* errorMessage = nullptr;
		std::vector<Song> result{};

		int returnCode = sqlite3_exec(db.get(), "SELECT name, artist, path FROM songs;", [](void* data, int argc, char** argv, char** colNames) {
			std::vector<Song>* songs = static_cast<std::vector<Song>*>(data);
			songs->emplace_back(argv[0], argv[1], argv[2]);

			return 0;
		}, &result, &errorMessage);

		if (returnCode != SQLITE_OK) {
			std::string errorMsg = errorMessage ? errorMessage : "Unknown error";
			std::println("Error executing query: {}", errorMsg);
			sqlite3_free(errorMessage);
			throw std::runtime_error("Failed to execute query: " + errorMsg);
		}

		return result;
	}
};

MusicRepository::MusicRepository(const std::string& dbPath) : impl(std::make_unique<Impl>(dbPath)) {}

std::vector<Song> MusicRepository::fetchAllSongs() {
	return impl->fetchAllSongs();
}

std::optional<Song> MusicRepository::fetchSong(const std::string& name, const std::string& artist) {
	return impl->fetchSong(name, artist);
}