// Lullabies.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <utility>
#include <atomic>
#include <print>


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
	    std::println("Failed to initialize ma_sound with error code {}", static_cast<int>(result));
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

    enum class CmdType { Play, Pause, Resume, Next, Stop };
    struct Command {
        CmdType type = CmdType::Stop;
        std::string path;
        std::function<void()> callback;
        Command() = default;
        Command(CmdType t, std::string p = {}, std::function<void()> cb = {}) : type(t), path(std::move(p)), callback(std::move(cb)) {}
    };

    std::thread worker;
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<Command> queue;
    std::atomic<bool> nextRequested{false};

public:
    AudioPlayer() {
        worker = std::thread([this] { worker_loop(); });
    }

    ~AudioPlayer() {
        // Signal the worker to stop
        {
            std::lock_guard lock(mtx);
            queue.emplace(Command{CmdType::Stop});
        }
        cv.notify_one();
        if (worker.joinable()) worker.join();
    }

    // Enqueue a request to play the given path. The worker thread will own
    // the Sound instance and will invoke the provided callback when the
    // sound ends (the callback is invoked on the worker thread).
    void play_sound(const std::string& path, std::function<void(void)> fn) {
        {
            std::lock_guard lock(mtx);
            queue.emplace(Command{CmdType::Play, path, std::move(fn)});
        }
        cv.notify_one();
    }

    std::string_view toggle() {

        std::string_view result;

        std::lock_guard lock(mtx);
        if (sound) {
            if (sound->isPlaying()) {
				result = "Song paused";
                queue.emplace(Command{CmdType::Pause});
            } else {
				result = "Song resumed";
                queue.emplace(Command{CmdType::Resume});
			}
        }
        else {
            result = "Nothing was playing";
        }
		cv.notify_one();

        return result;
    }

    void pause() {
        std::lock_guard lock(mtx);
        queue.emplace(Command{CmdType::Pause});
        cv.notify_one();
    }

    void resume() {
        std::lock_guard lock(mtx);
        queue.emplace(Command{CmdType::Resume});
        cv.notify_one();
    }

private:
    void worker_loop() {
        for (;;) {
            Command cmd;
            {
                std::unique_lock lock(mtx);
                cv.wait(lock, [this]{ return !queue.empty() || nextRequested.load(std::memory_order_acquire); });

                if (nextRequested.exchange(false, std::memory_order_acq_rel)) {
                    cmd = Command{CmdType::Next};
                } else {
                    cmd = std::move(queue.front());
                    queue.pop();
                }
            }

            if (cmd.type == CmdType::Stop) {
                break;
            }

            if (cmd.type == CmdType::Play) {
                // stop and destroy any existing sound first
                if (sound) {
                    sound->stop();
                    sound.reset();
                }

                // store callback to invoke when this sound ends
                callback = cmd.callback ? cmd.callback : []{};

                // create and start the new sound on the worker thread
                sound = std::make_unique<Sound>(engine, cmd.path);

                // The end callback is invoked from miniaudio's audio thread.
                // Keep it minimal: just enqueue a Next command for the worker
                // thread to process.
                ma_sound_set_end_callback(&sound->get(), [](void* userData, ma_sound* /*s*/) {
                    AudioPlayer* player = static_cast<AudioPlayer*>(userData);
                    // Only set an atomic flag and notify the worker. Avoid heap
                    // allocations / locking from the audio thread.
                    player->nextRequested.store(true, std::memory_order_release);
                    player->cv.notify_one();
                }, this);

                sound->start();
            }
            else if (cmd.type == CmdType::Next) {
                // Invoke the stored callback on the worker thread. The
                // callback may enqueue further commands (e.g. play_sound)
                // which is safe.
                try {
                    callback();
                } catch (...) {
                    // swallow exceptions to keep worker alive
                }
            }
            else if (cmd.type == CmdType::Pause) {
                if (sound) sound->stop();
            }
            else if (cmd.type == CmdType::Resume) {
                if (sound) sound->start();
            }
        }

        // Clean up sound if any
        if (sound) {
            sound->stop();
            sound.reset();
        }
    }
};
