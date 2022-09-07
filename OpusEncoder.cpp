#include "pch.h"

#include "OpusEncoder.h"

namespace winrt::blurt::audio::implementation {
namespace media = winrt::Windows::Media;

namespace {
constexpr auto kMaxRecommendedOpusFrameSize = 4000;
}  // namespace

OpusEncoder::OpusEncoder(AudioSetup audio_setup, OpusFrameSize frame_size)
    : samples_per_frame_{audio_setup.TotalSamplesPer(frame_size.Duration())},
      audio_setup_{audio_setup},
      pcm_buffer_{samples_per_frame_ * 50},
      encoding_buffer_{new std::uint8_t[kMaxRecommendedOpusFrameSize]} {
    int err;
    encoder_ = opus_encoder_create(audio_setup_.SamplesPerChannelPerSecond(),
                                   audio_setup_.NumChannels(), OPUS_APPLICATION_AUDIO, &err);
    if (err != OPUS_OK) abort();
    opus_encoder_ctl(encoder_, OPUS_SET_BITRATE(40000));
    opus_encoder_ctl(encoder_, OPUS_SET_VBR(0));
}

OpusEncoder::~OpusEncoder() { opus_encoder_destroy(encoder_); }

void OpusEncoder::BufferRawAudio(media::AudioFrame const& frame) {
    auto read_buffer = frame.LockBuffer(media::AudioBufferAccessMode::Read);
    std::int32_t input_len_bytes = read_buffer.Length();
    std::int32_t input_samples = input_len_bytes / sizeof(float);

    std::lock_guard lock{mutex_};
    if (pcm_buffer_.WriteCapacity() < input_samples) {
        // TODO: warn about overfull audio send buffer
        throw std::exception{"audio send buffer is overfull"};
    }

    auto frame_ref = read_buffer.CreateReference();
    pcm_buffer_.WriteSamplesFrom(reinterpret_cast<const float*>(frame_ref.data()), input_samples);
    if (pcm_buffer_.ReadCapacity() < samples_per_frame_) return;

    auto encoded_bytes =
        opus_encode_float(encoder_, pcm_buffer_.GetReadSourceFor(samples_per_frame_),
                          samples_per_frame_ / audio_setup_.NumChannels(), encoding_buffer_.get(),
                          kMaxRecommendedOpusFrameSize);
    if (encoded_bytes <= 0) {
        // TODO: log Opus encoding error
        throw std::exception{"Opus encoder error"};
    }
    assert(encoded_bytes < kMaxRecommendedOpusFrameSize);
    std::vector<std::uint8_t> result{encoding_buffer_.get(),
                                     encoding_buffer_.get() + encoded_bytes};
    encoded_audio_ready_(result);
}

}  // namespace winrt::blurt::audio::implementation
