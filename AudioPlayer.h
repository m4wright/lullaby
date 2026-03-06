// Lullabies.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <functional>

extern "C"
{
#include "miniaudio.c"
}


class AudioEngine {
	std::unique_ptr<ma_engine> engine = std::make_unique<ma_engine>();

public:
	AudioEngine() {
		if (ma_engine_init(nullptr, engine.get()) != MA_SUCCESS) {
			throw std::runtime_error("Failed to init engine");
		}
	}

    ma_engine& get() noexcept {
        return *engine;
    }

	~AudioEngine() {
		ma_engine_uninit(engine.get());
	}

	AudioEngine(const AudioEngine&) = delete;
	AudioEngine& operator=(const AudioEngine&) = delete;
};

class Sound {
	bool isInitialized = false;
    ma_sound sound{};

public:
    Sound() = delete;

    Sound(AudioEngine& engine, const std::string& filePath) {
        ma_result result = ma_sound_init_from_file(
            &engine.get(),
            filePath.c_str(),
            0,
            nullptr,
            nullptr,
            &sound);
        
            
        if (result != MA_SUCCESS) {
            throw std::runtime_error("Failed to initialize ma_sound");
        }

        isInitialized = true;
    }

    ~Sound() {
        cleanup();
    }

    ma_sound& get() {
        return sound;
    }

    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;

    Sound(Sound&& other) = delete;
    Sound& operator=(Sound&& other) = delete;

    void start() {
        check();
        ma_sound_start(&sound);
    }

    void stop() {
        check();
        ma_sound_stop(&sound);
    }

    bool isPlaying() const {
        return isInitialized && ma_sound_is_playing(&sound);
    }

private:

    void cleanup() {
        if (isInitialized) {
            ma_sound_uninit(&sound);
            isInitialized = false;
        }
    }

    void check() const {
        if (!isInitialized) {
            throw std::logic_error("Sound not initialized");
        }
    }
};


class AudioPlayer {
    AudioEngine engine{};
    std::unique_ptr<Sound> sound;
    std::function<void(void)> callback = [] {};
	
public:
	void play_sound(const std::string& path, std::function<void(void)> fn) {
        sound = std::make_unique<Sound>(engine, path);
        //this->callback = fn;
        sound->start();

		ma_sound_set_end_callback(&sound->get(), [](void* userData, ma_sound* sound) {
			AudioPlayer* player = static_cast<AudioPlayer*>(userData);
            player->callback();
		}, static_cast<void*>(this));
	}

	void pause() {
        if (sound) {
            sound->stop();
        }
	}

	void resume() {
        if (sound) {
            sound->start();
        }
	}
};
