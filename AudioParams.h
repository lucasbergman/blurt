#pragma once

#include <chrono>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace winrt::blurt::audio {

// Mumble sends and receives audio in 10-ms "frames." So for example, if you
// send a single 40-ms packet of Opus-encoded audio, the frame sequence counter
// in that packet's header should be 4 higher than the last one.
constexpr std::chrono::duration kMumbleFrameDuration = std::chrono::milliseconds{10};

class SampleRate {
   public:
    static SampleRate Of8KHz() { return SampleRate{8000}; }
    static SampleRate Of12KHz() { return SampleRate{12000}; }
    static SampleRate Of16KHz() { return SampleRate{16000}; }
    static SampleRate Of24KHz() { return SampleRate{24000}; }
    static SampleRate Of48KHz() { return SampleRate{48000}; }

    std::int32_t Per(std::chrono::milliseconds ms) const {
        auto result = per_sec_ / 1000 * ms.count();
        if (result > std::numeric_limits<std::int32_t>::max())
            throw std::overflow_error{"preposterous audio sample calculation"};
        return static_cast<std::int32_t>(result);
    }
    std::uint16_t PerSecond() const { return per_sec_; }

   private:
    SampleRate(std::uint16_t per_sec) : per_sec_{per_sec} {}
    const std::uint16_t per_sec_;
};

class Channels {
   public:
    static Channels Mono() { return Channels{1}; }
    static Channels Stereo() { return Channels{2}; }
    auto Num() const { return num_; }

   private:
    Channels(std::uint8_t num) : num_{num} {}
    const std::uint8_t num_;
};

class AudioSetup {
   public:
    AudioSetup(SampleRate sample_rate, Channels channels)
        : sample_rate_{sample_rate}, channels_{channels} {}
    auto NumChannels() const { return channels_.Num(); }
    auto SamplesPerChannelPerSecond() const { return sample_rate_.PerSecond(); }
    std::int32_t SamplesPerChannelPer(std::chrono::milliseconds ms) const {
        return sample_rate_.Per(ms);
    }
    std::int32_t TotalSamplesPer(std::chrono::milliseconds ms) const {
        return sample_rate_.Per(ms) * channels_.Num();
    }

   private:
    const SampleRate sample_rate_;
    const Channels channels_;
};

}  // namespace winrt::blurt::audio
