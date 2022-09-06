#include "pch.h"

#include "OpusEncoder.h"

#include <limits>
#include <memory>

namespace winrt::blurt::audio::implementation {
namespace media = winrt::Windows::Media;

namespace {
constexpr std::int16_t kMaxRecommendedOpusFrameSize = 4000;

constexpr bool kAssumeStereo = true;
constexpr std::int32_t kSampleRate = 48000;
constexpr std::uint8_t kNumChannels = kAssumeStereo ? 2 : 1;
}  // namespace

OpusEncoder::OpusEncoder(OpusFrameSize frame_size)
    : frame_samples_{static_cast<std::uint16_t>(kNumChannels * static_cast<uint16_t>(frame_size))},
      pcm_buffer_{frame_samples_ * 50} {
    int err;
    encoder_ = opus_encoder_create(kSampleRate, kNumChannels, OPUS_APPLICATION_AUDIO, &err);
    if (err != OPUS_OK) abort();
    opus_encoder_ctl(encoder_, OPUS_SET_BITRATE(40000));
    opus_encoder_ctl(encoder_, OPUS_SET_VBR(0));
}

OpusEncoder::~OpusEncoder() { opus_encoder_destroy(encoder_); }

void OpusEncoder::BufferRawAudio(media::AudioFrame const& frame) {
    auto read_buffer = frame.LockBuffer(media::AudioBufferAccessMode::Read);
    auto input_len_bytes = read_buffer.Length();
    std::uint32_t input_samples = input_len_bytes / sizeof(float);
    auto input_samples_per_chan = input_samples / kNumChannels;

    std::lock_guard lock{mutex_};
    if (pcm_buffer_.WriteCapacity() < input_samples) {
        // TODO: warn about overfull audio send buffer
        throw std::exception{"audio send buffer is overfull"};
    }

    auto frame_ref = read_buffer.CreateReference();
    pcm_buffer_.WriteSamplesFrom(reinterpret_cast<const float*>(frame_ref.data()), input_samples);
    if (pcm_buffer_.ReadCapacity() < frame_samples_) return;

    auto encoded_buffer = std::make_unique<std::uint8_t[]>(kMaxRecommendedOpusFrameSize);
    auto encoded_bytes = opus_encode_float(encoder_, pcm_buffer_.GetReadSourceFor(frame_samples_),
                                           frame_samples_ / kNumChannels, encoded_buffer.get(),
                                           kMaxRecommendedOpusFrameSize);
    if (encoded_bytes <= 0) {
        // TODO: log Opus encoding error
        throw std::exception{"Opus encoder error"};
    }
    assert(encoded_bytes < kMaxRecommendedOpusFrameSize);
    std::vector<std::uint8_t> result{encoded_buffer.get(), encoded_buffer.get() + encoded_bytes};
    encoded_audio_ready_(result);
}

}  // namespace winrt::blurt::audio::implementation
