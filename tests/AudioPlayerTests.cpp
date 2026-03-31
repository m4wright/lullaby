#include "../src/AudioPlayer.h"


#include <gtest/gtest.h>

#include <future>
#include <chrono>

struct PlayWithWait {
	std::promise<bool> callbackCalledPromise{};
	std::future<bool> callbackCalledFuture = callbackCalledPromise.get_future();

	PlayWithWait(AudioPlayer& player, const std::string& path = "/tmp/a.mp3") {
		player.playSound(path, [&] {
			callbackCalledPromise.set_value(true);
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

	player.pause(); 
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	EXPECT_FALSE(player.isPlaying());

	player.resume();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	EXPECT_TRUE(player.isPlaying());

	bool soundCompleted = waiter.wait();
	EXPECT_FALSE(player.isPlaying());
}

TEST(AudioPlayer, Toggle) {
	AudioPlayer player;

	PlayWithWait waiter{ player };
	EXPECT_TRUE(player.isPlaying());

	player.toggle();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	EXPECT_FALSE(player.isPlaying());

	player.toggle();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	EXPECT_TRUE(player.isPlaying());
}
