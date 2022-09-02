#include "pch.h"

#include "OpusDecoder.h"

#include <algorithm>
#include <chrono>

namespace winrt::blurt::audio::implementation {
namespace media = winrt::Windows::Media;

namespace {
constexpr bool kAssumeStereo = true;
constexpr std::int32_t kSampleRate = 48000;
constexpr std::uint8_t kNumChannels = kAssumeStereo ? 2 : 1;

// The maximum Opus can encode in one frame is 60 ms of audio.
// (48000 Hz) * (60 ms) is 2880 samples per channel, so a 2-channel frame
// decodes to at most 5760 floats, a bit over 11 KiB.
constexpr auto kMaxFrameDuration = std::chrono::milliseconds(60);
constexpr auto kMaxFrameSizeInFloatsPerChan =
    kMaxFrameDuration * kSampleRate / std::chrono::seconds(1);
constexpr auto kMaxFrameSizeInFloats = kMaxFrameSizeInFloatsPerChan * kNumChannels;
}  // namespace

OpusDecoder::OpusDecoder() : buffer_{kMaxFrameSizeInFloats * 50} {
    int err;
    decoder_ = opus_decoder_create(kSampleRate, kNumChannels, &err);
    if (err != OPUS_OK) abort();
}

OpusDecoder::~OpusDecoder() { opus_decoder_destroy(decoder_); }

std::int32_t OpusDecoder::DecodeToBuffer(const std::vector<std::uint8_t>& input) {
    assert(input.size() < std::numeric_limits<std::int32_t>::max());
    auto input_size = static_cast<std::int32_t>(input.size());

    int samples_per_chan = opus_decoder_get_nb_samples(decoder_, input.data(), input_size);
    if (samples_per_chan == OPUS_INVALID_PACKET)
        throw std::exception{"OpusDecoder::Decode: bogus audio packet"};
    if (samples_per_chan == OPUS_BAD_ARG)
        throw std::exception{"OpusDecoder::Decode: truncated audio packet; probable bug"};
    if (samples_per_chan <= 0)
        throw std::exception{"OpusDecoder::Decode: zany result from opus_decoder_get_nb_samples()"};

    auto needed_floats = samples_per_chan * kNumChannels;
    std::lock_guard lock{mutex_};
    if (buffer_.WriteCapacity() < needed_floats) {
        // We're out of buffer space, so tell Opus that we're dropping the packet
        opus_decode_float(decoder_, nullptr, input_size, nullptr, samples_per_chan, 0);
        return 0;
    }
    auto samples =
        opus_decode_float(decoder_, input.data(), input_size, buffer_.GetWriteDest(needed_floats),
                          samples_per_chan, 0 /* decode_fec */);
    if (samples <= 0) throw std::exception{"OpusDecoder::Decode: decode step failed; possible bug"};
    if (samples < samples_per_chan)
        throw std::exception{"OpusDecoder::Decode: decode step returned too few samples"};
    assert(samples == samples_per_chan);
    return samples;
}

media::AudioFrame OpusDecoder::ConsumeFrame(std::uint16_t samples_per_chan) {
    std::lock_guard lock{mutex_};
    auto buffered_samples = static_cast<std::uint32_t>(buffer_.ReadCapacity());
    auto samples_to_read =
        std::min<std::uint32_t>(buffered_samples, samples_per_chan * kNumChannels);
    if (samples_to_read == 0) return nullptr;

    // AudioFrame constructor is capacity in bytes:
    // https://docs.microsoft.com/en-us/uwp/api/windows.media.audioframe.-ctor
    media::AudioFrame frame{samples_to_read * sizeof(float)};
    auto buffer = frame.LockBuffer(media::AudioBufferAccessMode::Write);
    auto buffer_ref = buffer.CreateReference();
    float* dest = reinterpret_cast<float*>(buffer_ref.data());
    buffer_.ReadSamplesTo(dest, samples_to_read);
    return frame;
}

}  // namespace winrt::blurt::audio::implementation
