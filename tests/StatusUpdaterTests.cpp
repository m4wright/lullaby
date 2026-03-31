#include <gtest/gtest.h>

#include "../src/MusicStatusUpdater.h"

#include <vector>
#include <string>
#include <thread>

struct MockWritableSink {
	std::vector<std::string> messages;

	constexpr bool is_writable() const {
		return true;
	}

	bool write(const char* data, size_t size) {
		messages.emplace_back(std::string(data, size));
		return true;
	}
};

const static std::string expectedFirstEmptyStatus = R"(data: {"event":"message","playing":false}

)";

const static std::string expectedCurrentStatusString = R"(data: {"artist":"Artist1","event":"message","name":"Song A","playing":true}

)";

const static std::string expectedLastStatusString = R"(data: {"artist":"Artist1","event":"message","name":"Song C","playing":true}

)";

const static std::vector<SongStatus> statuses{
	SongStatus("Song A", "Artist1", true),

	// Simulates pausing the song
	SongStatus("Song A", "Artist1", false),
	// Simulates resuming
	SongStatus("Song A", "Artist1", true),

	SongStatus("Song B", "Artist2", true),
	SongStatus("Song C", "Artist1", true)
};

const static std::vector<std::string> statusStrings{
	expectedFirstEmptyStatus,
	R"(data: {"artist":"Artist1","event":"message","name":"Song A","playing":true}

)",
	R"(data: {"artist":"Artist1","event":"message","name":"Song A","playing":false}

)",
	R"(data: {"artist":"Artist1","event":"message","name":"Song A","playing":true}

)",
	R"(data: {"artist":"Artist2","event":"message","name":"Song B","playing":true}

)"
};

TEST(StatusUpdater, InitialStatusTest) {
	MusicStatusUpdater statusUpdater;
	EXPECT_EQ(expectedFirstEmptyStatus, statusUpdater.currentStatusString());
}

TEST(StatusUpdater, WaitForUpdateTest) {
	MusicStatusUpdater statusUpdater;
	MockWritableSink sink;

	{
		auto waitingThread = std::jthread([&] {
			bool updated = statusUpdater.waitForUpdate(sink);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		statusUpdater.updateStatus(SongStatus("Song A", "Artist1", true));
	}

	EXPECT_EQ(1, sink.messages.size());

	EXPECT_EQ(expectedFirstEmptyStatus, sink.messages[0]);
	EXPECT_EQ(expectedCurrentStatusString, statusUpdater.currentStatusString());
}

TEST(StatusUpdater, MultipleUpdatesTest) {
	MusicStatusUpdater statusUpdater;
	MockWritableSink sink;

	{
		auto waitingThread = std::jthread([&] {
			for (int i = 0; i < statuses.size(); i++) {
				statusUpdater.waitForUpdate(sink);
			}
		});

		for (const auto& status : statuses) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			statusUpdater.updateStatus(status);
		}
	}

	EXPECT_EQ(statusStrings, sink.messages);
	EXPECT_EQ(expectedLastStatusString, statusUpdater.currentStatusString());
}

TEST(StatusUpdater, WritableSinkIsClosedTest) {
	struct ClosedSink {
		bool is_writable() {
			return false;
		}

		bool write(const char* data, size_t size) {
			return false;
		}
	};

	MusicStatusUpdater statusUpdater;
	ClosedSink sink{};

	bool updated = statusUpdater.waitForUpdate(sink);
	EXPECT_FALSE(updated);
}

TEST(StatusUpdater, WriteUnsuccessfulTest) {
	struct ClosedSink {
		// Is writable is true
		bool is_writable() {
			return true;
		}

		// Write is not successful
		bool write(const char* data, size_t size) {
			return false;
		}
	};

	MusicStatusUpdater statusUpdater;
	ClosedSink sink{};

	bool updated = statusUpdater.waitForUpdate(sink);
	EXPECT_FALSE(updated);
}

TEST(StatusUpdater, FirstWriteSuccessSecondWriteFailTest) {
	struct SinkThatCloses {
		bool writable = true;

		// Is writable is true
		bool is_writable() {
			return true;
		}

		// Write is successful the first time only
		bool write(const char* data, size_t size) {
			bool currentlyWritable = writable;
			writable = false;
			return currentlyWritable;
		}
	};

	MusicStatusUpdater statusUpdater;
	SinkThatCloses sink{};

	{
		auto waitingThread = std::jthread([&] {
			bool firstUpdate = statusUpdater.waitForUpdate(sink);
			EXPECT_TRUE(firstUpdate);
		});

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		statusUpdater.updateStatus(SongStatus("Song A", "Artist1", true));
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	bool updated = statusUpdater.waitForUpdate(sink);
	EXPECT_FALSE(updated);
}