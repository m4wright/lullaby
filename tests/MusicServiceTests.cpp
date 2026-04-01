#include <gtest/gtest.h>

#include "mock_repository.h"
#include "../src/MusicService.h"

#include <mutex>
#include <latch>


TEST(MusicServiceTest, Play) {
	TempMusicRepository tempRepository{};
	MusicService musicService{ MusicRepository{tempRepository.path()} };
	std::mutex mtx;

	int numTimesStatusChangeCalled = 0;
	int numSongs = musicService.getAllSongs().size();
	std::latch fullSongRound{ numSongs + 1 };

	musicService.setOnSongStatusChange([&](const SongStatus& status) {
		std::lock_guard<std::mutex> lock(mtx);
		if (numTimesStatusChangeCalled == 0) {
			auto expectedStatus = SongStatus{ "A", "Artist1", true };
			EXPECT_EQ(expectedStatus, status);
		}
		else if (numTimesStatusChangeCalled == 1) {
			auto expectedStatus = SongStatus{ "B", "Artist2", true };
			EXPECT_EQ(expectedStatus, status);
		}
		else if (numTimesStatusChangeCalled == 2) {
			auto expectedStatus = SongStatus{ "C", "Artist3", true };
			EXPECT_EQ(expectedStatus, status);
		}
		else if (numTimesStatusChangeCalled == 3) {
			auto expectedStatus = SongStatus{ "A", "Artist1", true };
			EXPECT_EQ(expectedStatus, status);
		}

		numTimesStatusChangeCalled++;
		fullSongRound.count_down();
	});

	Song song = musicService.playNextSong();
	Song expectedSong{ .name = "A", .artist = "Artist1", .path = "/tmp/a.mp3" };
	EXPECT_EQ(expectedSong, song);

	fullSongRound.wait();

	EXPECT_EQ(numSongs + 1, numTimesStatusChangeCalled);
}

TEST(MusicServiceTest, ToggleTest) {
	TempMusicRepository tempRepository{};
	MusicService musicService{ MusicRepository{tempRepository.path()} };

	musicService.playNextSong();

	bool isPlaying = musicService.toggle();
	EXPECT_FALSE(isPlaying);

	isPlaying = musicService.toggle();
	EXPECT_TRUE(isPlaying);
}

TEST(MusicServiceTest, PlayNextAndPreviousSongTest) {
	TempMusicRepository tempRepository{};
	MusicService musicService{ MusicRepository{tempRepository.path()} };

	musicService.playNextSong();
	auto song = musicService.playNextSong();

	auto expectedSong = Song{ .name = "B", .artist = "Artist2", .path = "/tmp/b.mp3" };
	EXPECT_EQ(expectedSong, song);

	song = musicService.playPreviousSong();
	auto expectedSongFirstInList = Song{ .name = "A", .artist = "Artist1", .path = "/tmp/a.mp3" };
	EXPECT_EQ(expectedSongFirstInList, song);

	song = musicService.playPreviousSong();
	auto expectedSongPreviousWrapped = Song{ .name = "C", .artist = "Artist3", .path = "/tmp/c.mp3" };
	EXPECT_EQ(expectedSongPreviousWrapped, song);

	song = musicService.playNextSong();
	EXPECT_EQ(expectedSongFirstInList, song);
}

TEST(MusicServiceTest, StatusTest) {
	TempMusicRepository tempRepository{};
	MusicService musicService{ MusicRepository{tempRepository.path()} };

	musicService.playNextSong();

	auto status = musicService.getCurrentStatus();
	auto expectedStatus = SongStatus("A", "Artist1", true);
	EXPECT_EQ(expectedStatus, status);

	musicService.toggle();
	auto expectedPausedStatus = SongStatus("A", "Artist1", false);
	status = musicService.getCurrentStatus();
	EXPECT_EQ(expectedPausedStatus, status);
}
