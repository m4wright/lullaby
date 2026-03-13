#include "MusicRepository.h"

#include "third_party/sqlite3.h"

#include <algorithm>
#include <stdexcept>
#include <print>
#include <mutex>

MusicRepository::~MusicRepository() = default;

MusicRepository::MusicRepository(MusicRepository&&) noexcept = default;
MusicRepository& MusicRepository::operator=(MusicRepository&&) noexcept = default;


struct MusicRepository::Impl {
	std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db{ nullptr, sqlite3_close };
	std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt{ nullptr, sqlite3_finalize };
	std::mutex mtx;

	Impl(const std::string& dbPath) {
		sqlite3* rawDb = nullptr;
		if (sqlite3_open(dbPath.c_str(), &rawDb) != SQLITE_OK) {
			throw std::runtime_error("Failed to open database");
		}

		db.reset(rawDb);

		sqlite3_stmt* rawStmt = nullptr;
		const char* query = "SELECT name, artist, path FROM songs WHERE name = ? AND artist = ?;";
		if (sqlite3_prepare_v3(db.get(), query, -1, SQLITE_PREPARE_PERSISTENT, &rawStmt, nullptr) != SQLITE_OK) {
			throw std::runtime_error("Failed to prepare statement");
		}
		stmt.reset(rawStmt);
	}

	std::optional<Song> fetchSong(const std::string& name, const std::string& artist) {
		// Prevent multiple threads from using the same prepared statement simultaneously
		std::lock_guard lock(mtx);

		sqlite3_reset(stmt.get());

		sqlite3_bind_text(stmt.get(), 1, name.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt.get(), 2, artist.c_str(), -1, SQLITE_TRANSIENT);

		std::optional<Song> result;

		int rc = sqlite3_step(stmt.get());
		if (rc != SQLITE_ROW) {
			return result;
		}

		const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 2));

		if (path) {
			result.emplace(Song{ name, artist, std::string(path) });
		}

		return result;
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