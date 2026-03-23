// AudioPlayer interface. Production implementation lives in AudioPlayer.cpp
// and can be created with AudioPlayer::create_default(). Tests can provide
// a custom subclass to simulate playback without using miniaudio.

#pragma once

#include <string>
#include <functional>
#include <memory>

class AudioPlayer {
public:
    virtual ~AudioPlayer() = default;

    // Enqueue a request to play the given path. The provided callback will
    // be invoked when the sound ends.
    virtual void play_sound(const std::string& path, std::function<void(void)> fn) = 0;

    virtual bool toggle() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual bool isPlaying() = 0;

    // Factory for production implementation.
    static std::unique_ptr<AudioPlayer> create_default();
};
