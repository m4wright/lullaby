#include <gtest/gtest.h>

#include "MusicService.h"
#include "MusicRepository.h"
#include "Song.h"
#include "third_party/sqlite3.h"
#include <Windows.h>
#include <stdexcept>

#include <thread>
#include <chrono>
#include <atomic>

// A simple mock AudioPlayer that doesn't use miniaudio but simulates playback
// by invoking the end callback after a short delay.
class MockAudioPlayer : public AudioPlayer {
public:
    MockAudioPlayer() : playing(false) {}

    void play_sound(const std::string& path, std::function<void(void)> fn) override {
        playing = true;
        // Spawn a thread to simulate playback end
        std::thread([this, fn = std::move(fn)](){
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            playing = false;
            try { fn(); } catch(...) {}
        }).detach();
    }

    bool toggle() override {
        playing = !playing;
        return playing;
    }
    void pause() override { playing = false; }
    void resume() override { playing = true; }
    bool isPlaying() override { return playing; }

private:
    std::atomic<bool> playing;
};

// Helper to create an in-memory sqlite DB for tests and populate it
static std::string create_test_db() {
    // Use a temporary filename for an on-disk DB. sqlite3 supports in-memory via ":memory:"
    // but multiple connections won't see the same in-memory DB; production code opens a path,
    // so create a file in the temp directory.
    char path[MAX_PATH];
    if (!GetTempFileNameA(".", "lull", 0, path)) {
        throw std::runtime_error("Failed to create temp db path");
    }

    std::string dbPath = std::string(path) + ".db";
    // Initialize DB with simple songs table
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
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

    sqlite3_close(db);
    return dbPath;
}

TEST(MusicServiceIntegration, PlayByNameArtist) {
    std::string dbPath = create_test_db();
    MusicRepository repo(dbPath);

    auto player = std::make_unique<MockAudioPlayer>();
    MusicService svc(std::move(repo), std::move(player));

    std::atomic<bool> callbackCalled{false};
    svc.setOnSongStatusChange([&](const SongStatus& st){
        if (st.name == "A" && st.artist == "Artist1" && st.playing) callbackCalled = true;
    });

    bool started = svc.play("A", "Artist1");
    EXPECT_TRUE(started);

    // Wait up to 500ms for the mock player to start and callback
    for (int i = 0; i < 50 && !callbackCalled.load(); ++i) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(callbackCalled.load());
}

TEST(MusicServiceIntegration, PlayNextWraps) {
    std::string dbPath = create_test_db();
    MusicRepository repo(dbPath);

    auto player = std::make_unique<MockAudioPlayer>();
    MusicService svc(std::move(repo), std::move(player));

    // Play last song and call next, it should wrap to first
    bool started = svc.play("C", "Artist3");
    EXPECT_TRUE(started);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    Song next = svc.playNextSong();
    EXPECT_EQ(next.name, "A");
}
