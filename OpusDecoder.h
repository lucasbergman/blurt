#pragma once

#include <cstdint>
#include <mutex>
#include "AudioBuffer.h"
#include "AudioParams.h"
#include "ByteChunk.h"
#include "opus/opus.h"
#include "winrt/Windows.Media.Audio.h"

namespace winrt::blurt::audio::implementation {
class OpusDecoder {
   public:
    OpusDecoder(AudioSetup);
    ~OpusDecoder();

    // Decode the given audio bytes to the internal buffer. This is thread-safe
    // to be used concurrently with ConsumeFrame().
    std::int32_t DecodeToBuffer(const ByteChunk& encoded);

    // Read PCM audio from the internal buffer, up to the given number of
    // samples per channel; returns nullptr if the buffer is empty. This
    // method is thread-safe for concurrent use with DecodeToBuffer(). Any PCM
    // audio returned from this method is no longer available to be consumed.
    Windows::Media::AudioFrame ConsumeAudio(std::int32_t samples_per_chan);

   private:
    struct ::OpusDecoder* decoder_{nullptr};
    AudioSetup audio_setup_;
    std::mutex mutex_;
    _Guarded_by_(mutex_) AudioBuffer<float> buffer_;
};
}  // namespace winrt::blurt::audio::implementation
