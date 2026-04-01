#include <gtest/gtest.h>

#include "../src/MusicSerializer.h"

#ifndef UNIT_TEST
static_assert(false, "Tests must be compiled with UNIT_TEST defined");
#endif
TEST(Sanity, OnePlusOne) {
#ifdef UNIT_TEST
    EXPECT_EQ(1 + 2, 3);
#else
    EXPECT_EQ(1 + 1, 3);
#endif
}

const std::vector<Song> songs{
    {.name = "Song A", .artist = "Artist A", .path = "/tmp/path/to/a.mp3" },
    {.name = "Song B", .artist = "Artist A", .path = "/tmp/path/to/b.mp3" },
    {.name = "Song C", .artist = "Artist B", .path = "/tmp/path/to/c.mp3" }
};

TEST(Serialize, SerializeSongs) {
    std::string songJson = to_string(songs);

    std::string expectedJson = R"([{"artist":"Artist A","name":"Song A"},{"artist":"Artist A","name":"Song B"},{"artist":"Artist B","name":"Song C"}])";
    EXPECT_EQ(expectedJson, songJson);
}

TEST(Serialize, SerializeNotPlayingStatus) {
    SongStatus defaultNotPlayingStatus{};

    std::string expectedJson = R"({"event":"message","playing":false})";

    EXPECT_EQ(expectedJson, to_string(defaultNotPlayingStatus));
}

TEST(Serialize, SerializePlayingStatus) {
    SongStatus songStatus{ "Song A", "Artist A", true };

    std::string expectedJson = R"({"artist":"Artist A","event":"message","name":"Song A","playing":true})";

    EXPECT_EQ(expectedJson, to_string(songStatus));
}

TEST(Serialize, SerializePausedStatus) {
    SongStatus songStatus{ "Song A", "Artist A", false };

    std::string expectedJson = R"({"artist":"Artist A","event":"message","name":"Song A","playing":false})";

    EXPECT_EQ(expectedJson, to_string(songStatus));
}

TEST(Serialize, SerializeSongListAndStatus_NotPlaying) {
    SongStatus songStatus{};

    std::string expectedJson = R"({"now_playing":{"event":"message","playing":false},"songs":[{"artist":"Artist A","name":"Song A"},{"artist":"Artist A","name":"Song B"},{"artist":"Artist B","name":"Song C"}]})";

    EXPECT_EQ(expectedJson, to_string(songs, songStatus));
}

TEST(Serialize, SerializeSongListAndStatus_Playing) {
    SongStatus songStatus{"Song A", "Artist A", true};

    std::string expectedJson = R"({"now_playing":{"artist":"Artist A","event":"message","name":"Song A","playing":true},"songs":[{"artist":"Artist A","name":"Song A"},{"artist":"Artist A","name":"Song B"},{"artist":"Artist B","name":"Song C"}]})";

    EXPECT_EQ(expectedJson, to_string(songs, songStatus));
}

