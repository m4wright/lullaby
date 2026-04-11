#include "../src/AudioPlayer.h"


#include <gtest/gtest.h>

#include <future>
#include <chrono>
#include <atomic>

struct PlayWithWait {
	std::promise<bool> callbackCalledPromise{};
	std::future<bool> callbackCalledFuture = callbackCalledPromise.get_future();

	PlayWithWait(AudioPlayer& player, const std::string& path = "/tmp/a.mp3") {
		player.playSound(path, [&] {
			callbackCalledPromise.set_value(true);
		}).get();
	}

	bool wait() {
		std::future_status status = callbackCalledFuture.wait_for(std::chrono::seconds(1));
		return status == std::future_status::ready && callbackCalledFuture.get();
	}
};


TEST(AudioPlayer, PlaySong_Complete) {
	AudioPlayer player;

	PlayWithWait waiter{ player };
	EXPECT_TRUE(player.isPlaying());
	bool soundCompleted = waiter.wait();

	EXPECT_TRUE(soundCompleted);
	EXPECT_FALSE(player.isPlaying());
}

TEST(AudioPlayer, PlayPauseSongComplete) {
	AudioPlayer player;

	PlayWithWait waiter{ player };
	EXPECT_TRUE(player.isPlaying());

	player.pause().get();
	EXPECT_FALSE(player.isPlaying());

	player.resume().get();
	EXPECT_TRUE(player.isPlaying());

	bool soundCompleted = waiter.wait();
	EXPECT_FALSE(player.isPlaying());
}

TEST(AudioPlayer, Toggle) {
	AudioPlayer player;

	PlayWithWait waiter{ player };
	EXPECT_TRUE(player.isPlaying());

	player.toggle().get();
	EXPECT_FALSE(player.isPlaying());

	player.toggle().get();
	EXPECT_TRUE(player.isPlaying());
}

TEST(AudioPlayer, PlayAnotherWhilePlaying) {
	AudioPlayer player;

	std::atomic<bool> songEndCalled = false;

	player.playSound("/tmp/fake/A.mp3", [&] {
		songEndCalled = true;
	}).get();
	EXPECT_TRUE(player.isPlaying());

	player.playSound("/tmp/fake/B.mp3", [] {}).get();
	EXPECT_TRUE(player.isPlaying());
	EXPECT_FALSE(songEndCalled);
}

TEST(AudioPlayer, SetAndGetVolume) {
	AudioPlayer player;

	player.setVolume(0.5f);
	EXPECT_FLOAT_EQ(0.5f, player.getVolume());
}

TEST(AudioPlayer, ChangeVolumeNextSong) {
	AudioPlayer player;

	player.setVolume(0.3f);
	player.playSound("/tmp/fake/B.mp3", [] {}).get();
	EXPECT_FLOAT_EQ(0.3f, player.getVolume());
}
