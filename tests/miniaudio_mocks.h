#pragma once

#include <thread>
#include <chrono>
#include <vector>
#include <mutex>

enum ma_result {
	MA_SUCCESS
};




struct ma_engine {};
ma_result ma_engine_init(void*, ma_engine* engine) {
	return MA_SUCCESS;
}
void ma_engine_uninit(ma_engine* engine) {}


enum class SoundEvent {
	INIT,
	UNINIT,
	START,
	STOP,
	TOGGLE
};

struct ma_sound {
	std::mutex mtx;
	std::vector<SoundEvent> events;
};

ma_result ma_sound_init_from_file(ma_engine* engine, const char* path, int, void*, void*, ma_sound* sound) {
	std::lock_guard<std::mutex> lock(sound->mtx);
	sound->events.push_back(SoundEvent::INIT);
	return MA_SUCCESS;
}
void ma_sound_uninit(ma_sound* sound) {
	std::lock_guard<std::mutex> lock(sound->mtx);
	sound->events.push_back(SoundEvent::UNINIT);
}
void ma_sound_start(ma_sound* sound) {
	std::lock_guard<std::mutex> lock(sound->mtx);
	sound->events.push_back(SoundEvent::START);
}
void ma_sound_stop(ma_sound* sound) {
	std::lock_guard<std::mutex> lock(sound->mtx);
	sound->events.push_back(SoundEvent::STOP);
}

// Returns true if a valid sequence of events has occurred leading to playing
// That means the sound is started and play has been called more recently than pause
bool ma_sound_is_playing(const ma_sound* sound) {
	std::lock_guard<std::mutex> lock(const_cast<ma_sound*>(sound)->mtx);

	bool isInitialized = false;
	bool isPlaying = false;

	for (const SoundEvent& event : sound->events) {
		switch (event) {
		case SoundEvent::INIT:
			isInitialized = true;
			break;
		case SoundEvent::UNINIT:
			isInitialized = false;
			break;
		case SoundEvent::START:
			isPlaying = true;
			break;
		case SoundEvent::STOP:
			isPlaying = false;
			break;
		case SoundEvent::TOGGLE:
			isPlaying = !isPlaying;
			break;
		}
	}

	return isInitialized && isPlaying;
}

typedef void (*ma_sound_end_proc)(void* pUserData, ma_sound* pSound);
ma_result ma_sound_set_end_callback(ma_sound* sound, ma_sound_end_proc callback, void* userData) {
	std::thread([=] {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		{
			std::lock_guard<std::mutex> lock(sound->mtx);
			sound->events.push_back(SoundEvent::STOP);
		}
		callback(userData, sound);
	}).detach();

	return MA_SUCCESS;
}