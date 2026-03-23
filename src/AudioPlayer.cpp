#include "AudioPlayer.h"

#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <utility>
#include <atomic>
#include <memory>
#include <optional>
#include <print>

extern "C" {
#include "third_party/miniaudio.c"
}

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
        bool isInitialized = false;
        ma_sound sound{};

    public:
        Sound() = delete;
        Sound(AudioEngine& engine, const std::string& filePath) {
            ma_result result = ma_sound_init_from_file(engine.get(), filePath.c_str(), 0, nullptr, nullptr, &sound);
            if (result != MA_SUCCESS) {
                std::println("Failed to initialize ma_sound with error code {}", static_cast<int>(result));
                throw std::runtime_error("Failed to initialize ma_sound");
            }
            isInitialized = true;
        }

        ~Sound() {
            if (isInitialized) {
                ma_sound_uninit(&sound);
            }
        }
        ma_sound* get() { return &sound; }

        Sound(const Sound&) = delete;
        Sound& operator=(const Sound&) = delete;
        Sound(Sound&&) = delete;
        Sound& operator=(Sound&&) = delete;

        void start() { if (!isInitialized) throw std::logic_error("Sound not initialized"); ma_sound_start(&sound); }
        void stop() { if (!isInitialized) throw std::logic_error("Sound not initialized"); ma_sound_stop(&sound); }
        bool isPlaying() const { return isInitialized && ma_sound_is_playing(&sound); }
    };
}

// Keep the production implementation local. Implement a concrete class
// `ProdAudioPlayer` that satisfies the `AudioPlayer` interface.

struct ProdAudioPlayerImpl {
    AudioEngine engine{};
    std::optional<Sound> sound{};
    std::function<void(void)> callback = []{};

    enum class CmdType { Play, Pause, Resume, Next, Stop };
    struct Command { CmdType type = CmdType::Stop; std::string path; std::function<void()> callback; Command() = default; Command(CmdType t, std::string p = {}, std::function<void()> cb = {}) : type(t), path(std::move(p)), callback(std::move(cb)) {} };

    std::thread worker;
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<Command> queue;
    std::atomic<bool> nextRequested{false};

    ProdAudioPlayerImpl() { worker = std::thread([this]{ worker_loop(); }); }
    ~ProdAudioPlayerImpl() {
        { std::lock_guard lock(mtx); queue.emplace(Command{CmdType::Stop}); }
        cv.notify_one();
        if (worker.joinable()) worker.join();
    }

    void play_sound(const std::string& path, std::function<void(void)> fn) {
        { std::lock_guard lock(mtx); queue.emplace(Command{CmdType::Play, path, std::move(fn)}); }
        cv.notify_one();
    }

    bool toggle() {
        bool isPlaying;

        std::lock_guard lock(mtx);
        if (sound) {
            if (sound->isPlaying()) { isPlaying = false; queue.emplace(Command{CmdType::Pause}); }
            else { isPlaying = true; queue.emplace(Command{CmdType::Resume}); }
        }
        else { isPlaying = false; }
        cv.notify_one();
        return isPlaying;
    }

    void pause() { std::lock_guard lock(mtx); queue.emplace(Command{CmdType::Pause}); cv.notify_one(); }
    void resume() { std::lock_guard lock(mtx); queue.emplace(Command{CmdType::Resume}); cv.notify_one(); }

    bool isPlaying() {
        std::lock_guard lock(mtx);
		return sound.has_value() && sound->isPlaying();
    }

    void worker_loop() {
        for (;;) {
            Command cmd;
            {
                std::unique_lock lock(mtx);
                cv.wait(lock, [this]{ return !queue.empty() || nextRequested.load(std::memory_order_acquire); });
                if (nextRequested.exchange(false, std::memory_order_acq_rel)) { cmd = Command{CmdType::Next}; }
                else { cmd = std::move(queue.front()); queue.pop(); }
            }
            if (cmd.type == CmdType::Stop) break;
            if (cmd.type == CmdType::Play) {
                if (sound) { sound->stop(); }
                callback = cmd.callback ? cmd.callback : []{};
                sound.emplace(engine, cmd.path);
                ma_sound_set_end_callback(sound->get(), [](void* userData, ma_sound* /*s*/) {
                    ProdAudioPlayerImpl* impl = static_cast<ProdAudioPlayerImpl*>(userData);
                    impl->nextRequested.store(true, std::memory_order_release);
                    impl->cv.notify_one();
                }, this);
                sound->start();
            }
            else if (cmd.type == CmdType::Next) { try { callback(); } catch (...) { } }
            else if (cmd.type == CmdType::Pause) { if (sound) sound->stop(); }
            else if (cmd.type == CmdType::Resume) { if (sound) sound->start(); }
        }
        if (sound) { sound->stop(); sound.reset(); }
    }
};

class ProdAudioPlayer : public AudioPlayer {
public:
    ProdAudioPlayer() : impl(std::make_unique<ProdAudioPlayerImpl>()) {}
    ~ProdAudioPlayer() override = default;

    void play_sound(const std::string& path, std::function<void(void)> fn) override { impl->play_sound(path, std::move(fn)); }
    bool toggle() override { return impl->toggle(); }
    void pause() override { impl->pause(); }
    void resume() override { impl->resume(); }
    bool isPlaying() override { return impl->isPlaying(); }

private:
    std::unique_ptr<ProdAudioPlayerImpl> impl;
};

std::unique_ptr<AudioPlayer> AudioPlayer::create_default() {
    return std::make_unique<ProdAudioPlayer>();
}
