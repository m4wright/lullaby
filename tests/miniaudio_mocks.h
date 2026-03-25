#pragma once

#include <thread>
#include <chrono>

enum ma_result {
	MA_SUCCESS
};




struct ma_engine {};
ma_result ma_engine_init(void*, ma_engine* engine) {
	return MA_SUCCESS;
}
void ma_engine_uninit(ma_engine* engine) {}




struct ma_sound {};
ma_result ma_sound_init_from_file(ma_engine* engine, const char* path, int, void*, void*, ma_sound* sound) {
	return MA_SUCCESS;
}
void ma_sound_uninit(ma_sound* sound) {}
void ma_sound_start(ma_sound* sound) {}
void ma_sound_stop(ma_sound* sound) {}
bool ma_sound_is_playing(const ma_sound* sound) {
	return true;
}

typedef void (*ma_sound_end_proc)(void* pUserData, ma_sound* pSound);
ma_result ma_sound_set_end_callback(ma_sound* sound, ma_sound_end_proc callback, void* userData) {
	std::thread([&] {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		callback(userData, sound);
	}).detach();

	return MA_SUCCESS;
}