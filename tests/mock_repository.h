#pragma once

#include <filesystem>
#include <string>
#include <random>
#include <fstream>
#include "../src/third_party/sqlite3.h"

struct TempFileDeleter {
    void operator()(std::filesystem::path* p) const noexcept {
        if (p && !p->empty()) {
            std::error_code ec;
            std::filesystem::remove(*p, ec);
        }
        delete p;
    }
};

struct TemporaryFile {
    std::filesystem::path path;

    TemporaryFile(std::string_view extension = "") {
        auto tempDir = std::filesystem::temp_directory_path();

        std::random_device rd;
        std::uniform_int_distribution<uint64_t> dist;

        path = tempDir / ("tmp_" + std::to_string(dist(rd)) + std::string(extension));
        std::ofstream ofs(path);
        if (!ofs) {
            throw std::runtime_error("Failed to create temp file");
        }
    }

    TemporaryFile(const TemporaryFile&) = delete;
    TemporaryFile& operator=(const TemporaryFile&) = delete;

    ~TemporaryFile() {
        if (!path.empty()) {
            std::error_code ec;
            std::filesystem::remove(path, ec);
        }
    }
};




struct TempMusicRepository {
    TemporaryFile tempFile;
    sqlite3* db = nullptr;

    TempMusicRepository() : tempFile{ ".db" } {
        auto rc = sqlite3_open(tempFile.path.string().c_str(), &db);
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Failed to open test db");
        }

        const char* createSql = "CREATE TABLE songs(name TEXT, artist TEXT, path TEXT);";
        char* err = nullptr;
        if (sqlite3_exec(db, createSql, nullptr, nullptr, &err) != SQLITE_OK) {
            std::string e = err ? err : "unknown";
            sqlite3_free(err);
            sqlite3_close(db);
            throw std::runtime_error("Failed to create table: " + e);
        }

        const char* insertSql = "INSERT INTO songs(name, artist, path) VALUES ('A','Artist1','/tmp/a.mp3'),('B','Artist2','/tmp/b.mp3'),('C','Artist3','/tmp/c.mp3');";
        if (sqlite3_exec(db, insertSql, nullptr, nullptr, &err) != SQLITE_OK) {
            std::string e = err ? err : "unknown";
            sqlite3_free(err);
            sqlite3_close(db);
            throw std::runtime_error("Failed to insert rows: " + e);
        }
    }

    std::string path() const {
        return tempFile.path.string();
    }

    ~TempMusicRepository() {
        sqlite3_close(db);
    }

    TempMusicRepository(const TempMusicRepository&) = delete;
    TempMusicRepository& operator=(const TempMusicRepository&) = delete;
};
