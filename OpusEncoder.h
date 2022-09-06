#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <opus/opus.h>
#include <vector>
#include "AudioBuffer.h"
#include "AudioParams.h"
#include "winrt/Windows.Media.Audio.h"
#include "winrt/base.h"

namespace winrt::blurt::audio::implementation {

class OpusFrameSize {
   public:
    static OpusFrameSize Of10ms() { return OpusFrameSize{std::chrono::milliseconds{10}}; }
    static OpusFrameSize Of20ms() { return OpusFrameSize{std::chrono::milliseconds{20}}; }
    static OpusFrameSize Of40ms() { return OpusFrameSize{std::chrono::milliseconds{40}}; }
    static OpusFrameSize Of80ms() { return OpusFrameSize{std::chrono::milliseconds{80}}; }

    std::chrono::milliseconds Duration() const { return duration_; }

   private:
    OpusFrameSize(std::chrono::milliseconds duration) : duration_{duration} {}
    const std::chrono::milliseconds duration_;
};

class OpusEncoder {
   public:
    // Create a new Opus encoder with the given desired frame size
    OpusEncoder(AudioSetup audio_setup, OpusFrameSize frame_size);
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
    AudioSetup audio_setup_;
    const std::int32_t samples_per_frame_;
    std::recursive_mutex mutex_;
    _Guarded_by_(mutex_) AudioBuffer<float> pcm_buffer_;
    _Guarded_by_(mutex_) std::unique_ptr<std::uint8_t[]> encoding_buffer_;
    winrt::event<winrt::delegate<std::vector<std::uint8_t>>> encoded_audio_ready_;
};
}  // namespace winrt::blurt::audio::implementation
