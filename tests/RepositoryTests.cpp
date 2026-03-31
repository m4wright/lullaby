#include <gtest/gtest.h>

#include <string>
#include <string_view>

#include "../src/MusicRepository.h"
#include "mock_repository.h"

namespace {
    TempMusicRepository tempRepository{};
    MusicRepository repository(tempRepository.path());
}

TEST(Repository, FetchSong) {
    std::optional<Song> songAOpt = repository.fetchSong("A", "Artist1");
    EXPECT_TRUE(songAOpt.has_value());

    Song expectedSong{ .name = "A", .artist = "Artist1", .path = "/tmp/a.mp3" };
    EXPECT_EQ(expectedSong, songAOpt.value());
}

TEST(Repository, FetchSongNotFound) {
    std::optional<Song> songAOpt = repository.fetchSong("FAKE_SONG", "FAKE_ARTIST");
    EXPECT_FALSE(songAOpt.has_value());
}

TEST(Repository, FetchAllSongs) {
    std::vector<Song> songs = repository.fetchAllSongs();
    std::vector<Song> expectedSongs = {
        {.name = "A", .artist = "Artist1", .path = "/tmp/a.mp3"},
        {.name = "B", .artist = "Artist2", .path = "/tmp/b.mp3"},
        {.name = "C", .artist = "Artist3", .path = "/tmp/c.mp3"},
    };

    EXPECT_EQ(expectedSongs, songs);
}
