// Lightweight declaration for AudioPlayer using PIMPL to hide heavy
// miniaudio implementation details from consumers. This reduces rebuild
// scopes when miniaudio changes and avoids including miniaudio headers in
// every translation unit that includes this header.

#pragma once

#include <string>
#include <functional>
#include <string_view>
#include <memory>

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
    void play_sound(const std::string& path, std::function<void(void)> fn);

    // Toggle between pause/resume for the current sound. Returns a
    // human-readable status string (string_view refers to a static literal).
    std::string_view toggle();

    void pause();
    void resume();
};
