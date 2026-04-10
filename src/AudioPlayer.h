// Lightweight declaration for AudioPlayer using PIMPL to hide heavy
// miniaudio implementation details from consumers. This reduces rebuild
// scopes when miniaudio changes and avoids including miniaudio headers in
// every translation unit that includes this header.

#pragma once

#include <string>
#include <functional>
#include <memory>
#include <future>

class AudioPlayer {
    struct Impl;
    std::unique_ptr<Impl> impl;

public:
    AudioPlayer();
    ~AudioPlayer();

    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;

    // Enqueue a request to play the given path. The worker thread will own
    // the sound instance and will invoke the provided callback when the
    // sound ends (the callback is invoked on the worker thread).
    std::future<void> playSound(std::string path, std::function<void(void)> fn);

    std::future<bool> toggle();

    std::future<void> pause();
    std::future<void> resume();

    bool isPlaying();

	float getVolume();
    void setVolume(float volume);
};
