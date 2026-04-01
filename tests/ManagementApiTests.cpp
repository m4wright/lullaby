#include <gtest/gtest.h>

#include <string>
#include <string_view>

#include "../src/MusicRepository.h"
#include "mock_repository.h"

namespace {
    TempMusicRepository tempRepository{};
    MusicRepository repository(tempRepository.path());
}

TEST(ManagementApi, AddSong) {
    bool added = repository.addSong("TestSong", "TestArtist", "/tmp/test.mp3");
    EXPECT_TRUE(added);

    std::optional<Song> song = repository.fetchSong("TestSong", "TestArtist");
    EXPECT_TRUE(song.has_value());
    EXPECT_EQ("TestSong", song->name);
    EXPECT_EQ("TestArtist", song->artist);
    EXPECT_EQ("/tmp/test.mp3", song->path);
}

TEST(ManagementApi, AddSongDuplicate) {
    // First add should succeed
    bool added1 = repository.addSong("DuplicateSong", "DuplicateArtist", "/tmp/dup1.mp3");
    EXPECT_TRUE(added1);

    // Second add with same name/artist should fail
    bool added2 = repository.addSong("DuplicateSong", "DuplicateArtist", "/tmp/dup2.mp3");
    EXPECT_FALSE(added2);
}

TEST(ManagementApi, UpdateSongPath) {
    // Add a song first
    repository.addSong("UpdateTest", "UpdateArtist", "/tmp/original.mp3");

    // Update the path
    bool updated = repository.updateSongPath("UpdateTest", "UpdateArtist", "/tmp/updated.mp3");
    EXPECT_TRUE(updated);

    // Verify the path was updated
    std::optional<Song> song = repository.fetchSong("UpdateTest", "UpdateArtist");
    ASSERT_TRUE(song.has_value());
    EXPECT_EQ("/tmp/updated.mp3", song->path);
}

TEST(ManagementApi, UpdateSongPathNotFound) {
    // Try to update a song that doesn't exist
    bool updated = repository.updateSongPath("NonExistent", "NonExistent", "/tmp/fake.mp3");
    EXPECT_FALSE(updated);
}

TEST(ManagementApi, DeleteSong) {
    // Add a song first
    repository.addSong("DeleteTest", "DeleteArtist", "/tmp/delete.mp3");

    // Delete it
    bool deleted = repository.deleteSong("DeleteTest", "DeleteArtist");
    EXPECT_TRUE(deleted);

    // Verify it's gone
    std::optional<Song> song = repository.fetchSong("DeleteTest", "DeleteArtist");
    EXPECT_FALSE(song.has_value());
}

TEST(ManagementApi, DeleteSongNotFound) {
    // Try to delete a song that doesn't exist
    bool deleted = repository.deleteSong("NonExistent", "NonExistent");
    EXPECT_FALSE(deleted);
}

TEST(ManagementApi, AddAndUpdateThenVerify) {
    std::string name = "ChainTest";
    std::string artist = "ChainArtist";
    std::string path1 = "/tmp/chain1.mp3";
    std::string path2 = "/tmp/chain2.mp3";

    // Add
    EXPECT_TRUE(repository.addSong(name, artist, path1));

    // Update
    EXPECT_TRUE(repository.updateSongPath(name, artist, path2));

    // Verify final state
    std::optional<Song> song = repository.fetchSong(name, artist);
    ASSERT_TRUE(song.has_value());
    EXPECT_EQ(path2, song->path);

    // Delete
    EXPECT_TRUE(repository.deleteSong(name, artist));

    // Verify deleted
    EXPECT_FALSE(repository.fetchSong(name, artist).has_value());
}
