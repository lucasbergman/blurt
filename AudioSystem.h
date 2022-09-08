#pragma once

#include <vector>
#include "ByteChunk.h"
#include "OpusDecoder.h"
#include "OpusEncoder.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.Media.Audio.h"
#include "winrt/Windows.Media.h"
#include "winrt/base.h"

namespace winrt::blurt::implementation {

class AudioSystem {
   public:
    AudioSystem() = default;
    Windows::Foundation::IAsyncAction SetUp();
    void DecodeForOutput(const ByteChunk& encoded_bytes);

    winrt::event_token EncodedCaptureReady(
        winrt::delegate<std::vector<std::uint8_t>> const& handler) {
        return event_encoded_capture_ready_.add(handler);
    }
    void EncodedCaptureReady(winrt::event_token const& token) noexcept {
        event_encoded_capture_ready_.remove(token);
    }

   private:
    void OutputAudioGraph_QuantumStarted(
        Windows::Media::Audio::AudioFrameInputNode,
        Windows::Media::Audio::FrameInputNodeQuantumStartedEventArgs const&);
    void AudioSystem::CaptureAudioGraph_QuantumStarted(Windows::Media::Audio::AudioGraph graph,
                                                       Windows::Foundation::IInspectable);

    winrt::event<winrt::delegate<std::vector<std::uint8_t>>> event_encoded_capture_ready_;
    Windows::Media::Audio::AudioGraph output_graph_{nullptr};
    const blurt::audio::AudioSetup output_setup_{blurt::audio::SampleRate::Of48KHz(),
                                                 blurt::audio::Channels::Stereo()};
    Windows::Media::Audio::AudioGraph capture_graph_{nullptr};
    const blurt::audio::AudioSetup capture_setup_{blurt::audio::SampleRate::Of48KHz(),
                                                  blurt::audio::Channels::Stereo()};
    Windows::Media::Audio::AudioFrameOutputNode capture_output_{nullptr};
    blurt::audio::implementation::OpusDecoder opus_decoder_{output_setup_};
    blurt::audio::implementation::OpusEncoder opus_encoder_{
        capture_setup_, blurt::audio::implementation::OpusFrameSize::Of20ms()};
};

}  // namespace winrt::blurt::implementation
