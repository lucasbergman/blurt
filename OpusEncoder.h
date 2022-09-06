#pragma once

#include <cstdint>
#include <mutex>
#include <opus/opus.h>
#include <vector>
#include "AudioBuffer.h"
#include "winrt/Windows.Media.Audio.h"
#include "winrt/base.h"

namespace winrt::blurt::audio::implementation {

enum class OpusFrameSize { Chunk10ms = 480, Chunk20ms = 960, Chunk40ms = 1920, Chunk80ms = 2880 };

class OpusEncoder {
   public:
    // Create a new Opus encoder with the given desired frame size
    OpusEncoder(OpusFrameSize frame_size);
    ~OpusEncoder();

    // Read raw audio into the encoder's buffer. If this input frame adds
    // enough to the buffer that the buffer has enough audio to fill this
    // encoder's frame size, then encoding is performed and the
    // EncodedAudioReady event is fired.
    void BufferRawAudio(winrt::Windows::Media::AudioFrame const& frame);

    // TODO: Avoid copying the byte vector so much?
    event_token EncodedAudioReady(delegate<std::vector<std::uint8_t>> const& handler) {
        return encoded_audio_ready_.add(handler);
    }
    void EncodedAudioReady(winrt::event_token const& token) noexcept {
        encoded_audio_ready_.remove(token);
    }

   private:
    struct ::OpusEncoder* encoder_{nullptr};
    const std::uint16_t frame_samples_;
    std::recursive_mutex mutex_;
    _Guarded_by_(mutex_) AudioBuffer<float> pcm_buffer_;
    winrt::event<winrt::delegate<std::vector<std::uint8_t>>> encoded_audio_ready_;
};
}  // namespace winrt::blurt::audio::implementation
