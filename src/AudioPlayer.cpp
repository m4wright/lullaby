#include "AudioPlayer.h"
#include "SingleThreadExecutor.h"

#include <string>
#include <functional>
#include <utility>
#include <memory>
#include <optional>
#include <print>
#include <atomic>


#ifdef UNIT_TEST
#include "tests/miniaudio_mocks.h"
#else
extern "C" {
#include "third_party/miniaudio.c"
}
#endif

namespace {
    class AudioEngine {
        ma_engine engine{};
    public:
        AudioEngine() {
            if (ma_engine_init(nullptr, &engine) != MA_SUCCESS) {
                throw std::runtime_error("Failed to init engine");
            }
        }
        ma_engine* get() noexcept { return &engine; }
        ~AudioEngine() { ma_engine_uninit(&engine); }
        AudioEngine(const AudioEngine&) = delete;
        AudioEngine& operator=(const AudioEngine&) = delete;
    };

    class Sound {
        ma_sound sound{};

    public:
        Sound() = delete;
        Sound(AudioEngine& engine, const std::string& filePath) {
            ma_result result = ma_sound_init_from_file(engine.get(), filePath.c_str(), 0, nullptr, nullptr, &sound);
            if (result != MA_SUCCESS) {
                std::println("Failed to initialize ma_sound with error code {}", static_cast<int>(result));
                throw std::runtime_error("Failed to initialize ma_sound");
            }
        }

        ~Sound() {
            ma_sound_uninit(&sound);
        }
        ma_sound* get() { return &sound; }

        Sound(const Sound&) = delete;
        Sound& operator=(const Sound&) = delete;
        Sound(Sound&&) = delete;
        Sound& operator=(Sound&&) = delete;

        void start() { ma_sound_start(&sound); }
        void stop() { ma_sound_stop(&sound); }
        bool isPlaying() const { return ma_sound_is_playing(&sound); }
    };
}

struct AudioPlayer::Impl {
    AudioEngine engine{};
    std::optional<Sound> sound{};
    SingleThreadExecutor executor{};
    std::function<void()> callback = [] {};
    std::atomic<float> volume = 1.0f;

    std::future<void> playSound(std::string path, std::function<void(void)> cb) {
        return executor.submit([this, path = std::move(path), cb = std::move(cb)] {
            sound.emplace(engine, std::move(path));
            ma_sound_set_end_callback(sound->get(), [](void* userData, ma_sound*) {
                auto* impl = static_cast<Impl*>(userData);
                impl->executor.submit([impl] {           // end callback posts back onto executor
                    impl->callback();
                });
            }, this);

            callback = cb;
            sound->start();
        });
    }

    std::future<bool> toggle() {
        return executor.submit([this] {
            if (!sound) return false;
            if (sound->isPlaying()) { sound->stop(); return false; }
            else { sound->start(); return true; }
         });
    }

    std::future<void> pause() { 
        return executor.submit([this] {
            if (sound) {
                sound->stop();
            }
        });
    }
    std::future<void> resume() { 
        return executor.submit([this] {
            if (sound) {
                sound->start();
            }
        });
    }

    template <typename F>
    auto get(F&& f) {
        auto future = executor.submit([f = std::move(f)] {
            return f();
        });

        auto status = future.wait_for(std::chrono::seconds(5));

        if (status == std::future_status::ready) {
            return future.get();
        }
        throw std::runtime_error("Unable to get the object from AudioPlayer::Impl, the task didn't complete in time");
    }

    bool isPlaying() {
        return get([this] {
            return sound && sound->isPlaying();
        });
    }


    float getVolume() {
        return volume;
    }

    void setVolume(float volume) {
        this->volume = volume;
        executor.submit([this, volume] {
            ma_engine_set_volume(engine.get(), volume);
        });
	}
    
};

AudioPlayer::AudioPlayer() : impl(std::make_unique<Impl>()) {}
AudioPlayer::~AudioPlayer() = default;
std::future<void> AudioPlayer::playSound(std::string path, std::function<void(void)> fn) { return impl->playSound(std::move(path), std::move(fn)); }
std::future<bool> AudioPlayer::toggle() { return impl->toggle(); }
std::future<void> AudioPlayer::pause() { return impl->pause(); }
std::future<void> AudioPlayer::resume() { return impl->resume(); }
bool AudioPlayer::isPlaying() { return impl->isPlaying(); }

float AudioPlayer::getVolume() { return impl->getVolume(); }
void AudioPlayer::setVolume(float volume) { impl->setVolume(volume); }
