#pragma once

#include "MusicSerializer.h"

#include <concepts>
#include <type_traits>
#include <shared_mutex>
#include <condition_variable>

template <class T>
concept WritableSink = requires(T sink, const std::string& data) {
	{ sink.is_writable() } -> std::same_as<bool>;
	{ sink.write(data.data(), data.size()) } -> std::same_as<bool>;
};


class MusicStatusUpdater {
	std::shared_mutex mtx{};
	std::condition_variable_any cv{};

	std::string currentStatusStr = "data: " + to_string(SongStatus{}) + "\n\n";

	bool update(WritableSink auto& sink, const std::string& status) {
		if (!sink.is_writable()) {
			return false;
		}
		return sink.write(status.data(), status.size());
	}

public:
	// Immediately send the current status.
	// Then wait until there is an update. Once there is an update, return true to indicate
	// to send the status again
	bool waitForUpdate(WritableSink auto& sink) {
		std::shared_lock lock(mtx);
		
		if (!update(sink, currentStatusStr)) {
			return false;
		}

		cv.wait(lock);
		return true;
	}

	std::string_view currentStatusString() const {
		return currentStatusStr;
	}

	void updateStatus(const SongStatus& status) {
		std::unique_lock lock(mtx);
		currentStatusStr = "data: " + to_string(status) + "\n\n";
		cv.notify_all();
	}
};