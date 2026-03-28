#include "../src/AudioPlayer.h"


#include <gtest/gtest.h>

#include <future>
#include <chrono>


TEST(AudioPlayer, PlaySong) {
	AudioPlayer player;

	std::promise<bool> callbackCalledPromise;
	std::future<bool> callbackCalledFuture = callbackCalledPromise.get_future();

	// A fake path that isn't used in the mock miniaudio implementation
	player.play_sound("/tmp/a.mp3", [&] {
		callbackCalledPromise.set_value(true);
	});

	std::future_status status = callbackCalledFuture.wait_for(std::chrono::seconds(1));

	EXPECT_EQ(std::future_status::ready, status);
	EXPECT_EQ(true, callbackCalledFuture.get());
}
