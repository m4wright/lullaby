#include <gtest/gtest.h>

#include "mock_repository.h"
#include "../src/MusicService.h"

#include <mutex>
#include <latch>


namespace {
	TempMusicRepository tempRepository{};
}

TEST(MusicServiceTest, Play) {
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