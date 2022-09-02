#pragma once

#include <cstdint>
#include <mutex>
#include <vector>
#include "AudioBuffer.h"
#include "AudioPacket.h"
#include "opus/opus.h"
#include "winrt/Windows.Media.Audio.h"

namespace winrt::blurt::audio::implementation {
class OpusDecoder {
   public:
    OpusDecoder();
    ~OpusDecoder();

    std::int32_t DecodeToBuffer(const std::vector<std::uint8_t>& encoded);
    std::int32_t DecodeToBuffer(const mumble::AudioPacket& packet);
    winrt::Windows::Media::AudioFrame ConsumeFrameFromBuffer(std::uint16_t samples);

   private:
    struct ::OpusDecoder* decoder_{nullptr};
    std::mutex mutex_;
    _Guarded_by_(mutex_) AudioBuffer<float> buffer_;
};
}  // namespace winrt::blurt::audio::implementation
