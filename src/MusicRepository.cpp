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

	std::optional<Song> fetchSongUnlocked(const std::string& name, const std::string& artist) {
		sqlite3_reset(stmt.get());

		sqlite3_bind_text(stmt.get(), 1, name.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt.get(), 2, artist.c_str(), -1, SQLITE_STATIC);

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

	std::optional<Song> fetchSong(const std::string& name, const std::string& artist) {
		// Prevent multiple threads from using the same prepared statement simultaneously
		std::lock_guard lock(mtx);
		return fetchSongUnlocked(name, artist);
	}

	std::vector<Song> fetchAllSongs() {
		std::lock_guard lock(mtx);

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

	bool addSong(const std::string& name, const std::string& artist, const std::string& path) {
		std::lock_guard lock(mtx);

		// Check if song already exists (use unlocked version since we already hold the lock)
		auto existing = fetchSongUnlocked(name, artist);
		if (existing.has_value()) {
			return false;
		}

		const char* query = "INSERT INTO songs (name, artist, path) VALUES (?, ?, ?);";
		char* errorMessage = nullptr;

		sqlite3_stmt* stmt = nullptr;
		if (sqlite3_prepare_v2(db.get(), query, -1, &stmt, nullptr) != SQLITE_OK) {
			std::println("Failed to prepare INSERT statement");
			return false;
		}

		sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, artist.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, path.c_str(), -1, SQLITE_STATIC);

		int rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		if (rc != SQLITE_DONE) {
			std::println("Failed to insert song: {}", sqlite3_errmsg(db.get()));
			return false;
		}

		return true;
	}

	bool updateSongPath(const std::string& name, const std::string& artist, const std::string& newPath) {
		std::lock_guard lock(mtx);

		const char* query = "UPDATE songs SET path = ? WHERE name = ? AND artist = ?;";
		sqlite3_stmt* stmt = nullptr;

		if (sqlite3_prepare_v2(db.get(), query, -1, &stmt, nullptr) != SQLITE_OK) {
			std::println("Failed to prepare UPDATE statement");
			return false;
		}

		sqlite3_bind_text(stmt, 1, newPath.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, artist.c_str(), -1, SQLITE_STATIC);

		int rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		if (rc != SQLITE_DONE) {
			std::println("Failed to update song: {}", sqlite3_errmsg(db.get()));
			return false;
		}

		// Check if any row was actually updated
		return sqlite3_changes(db.get()) > 0;
	}

	bool deleteSong(const std::string& name, const std::string& artist) {
		std::lock_guard lock(mtx);

		const char* query = "DELETE FROM songs WHERE name = ? AND artist = ?;";
		sqlite3_stmt* stmt = nullptr;

		if (sqlite3_prepare_v2(db.get(), query, -1, &stmt, nullptr) != SQLITE_OK) {
			std::println("Failed to prepare DELETE statement");
			return false;
		}

		sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, artist.c_str(), -1, SQLITE_STATIC);

		int rc = sqlite3_step(stmt);
		sqlite3_finalize(stmt);

		if (rc != SQLITE_DONE) {
			std::println("Failed to delete song: {}", sqlite3_errmsg(db.get()));
			return false;
		}

		// Check if any row was actually deleted
		return sqlite3_changes(db.get()) > 0;
	}
};

MusicRepository::MusicRepository(const std::string& dbPath) : impl(std::make_unique<Impl>(dbPath)) {}

std::vector<Song> MusicRepository::fetchAllSongs() {
	return impl->fetchAllSongs();
}

std::optional<Song> MusicRepository::fetchSong(const std::string& name, const std::string& artist) {
	return impl->fetchSong(name, artist);
}

bool MusicRepository::addSong(const std::string& name, const std::string& artist, const std::string& path) {
	return impl->addSong(name, artist, path);
}

bool MusicRepository::updateSongPath(const std::string& name, const std::string& artist, const std::string& newPath) {
	return impl->updateSongPath(name, artist, newPath);
}

bool MusicRepository::deleteSong(const std::string& name, const std::string& artist) {
	return impl->deleteSong(name, artist);
}