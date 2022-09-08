#include "pch.h"

#include "AudioSystem.h"

#include "winrt/Windows.Devices.Enumeration.h"
#include "winrt/Windows.Media.Capture.h"
#include "winrt/Windows.Media.Devices.h"
#include "winrt/Windows.Media.MediaProperties.h"
#include "winrt/Windows.Media.Render.h"

namespace winrt::blurt::implementation {

namespace {
float RMSLevel(winrt::Windows::Media::AudioFrame const& frame) {
    auto read_buffer = frame.LockBuffer(winrt::Windows::Media::AudioBufferAccessMode::Read);
    auto input_len_bytes = read_buffer.Length();
    std::uint32_t input_samples = input_len_bytes / sizeof(float);
    auto frame_ref = read_buffer.CreateReference();
    const float* pcm = reinterpret_cast<const float*>(frame_ref.data());

    float sum{0};
    for (std::uint32_t i = 0; i < input_samples; i++) sum += (pcm[i] * pcm[i]);
    return std::sqrt(sum / input_samples);
}

namespace winrtaudio = Windows::Media::Audio;
}  // namespace

Windows::Foundation::IAsyncAction AudioSystem::SetUp() {
    opus_encoder_.EncodedAudioReady(
        [this](std::vector<std::uint8_t> bytes) { event_encoded_capture_ready_(bytes); });

    {
        Windows::Devices::Enumeration::DeviceInformation output_dev{nullptr};

        auto output_settings =
            winrtaudio::AudioGraphSettings{Windows::Media::Render::AudioRenderCategory::GameChat};
        output_settings.QuantumSizeSelectionMode(
            winrtaudio::QuantumSizeSelectionMode::LowestLatency);
        if (output_dev) output_settings.PrimaryRenderDevice(output_dev);
        auto output_graph_result = co_await winrtaudio::AudioGraph::CreateAsync(output_settings);
        if (output_graph_result.Status() != winrtaudio::AudioGraphCreationStatus::Success)
            throw std::exception{"audio graph create failed"};
        output_graph_ = output_graph_result.Graph();
        output_graph_.EncodingProperties().ChannelCount(output_setup_.NumChannels());
        output_graph_.EncodingProperties().SampleRate(output_setup_.SamplesPerChannelPerSecond());

        auto device_result = co_await output_graph_.CreateDeviceOutputNodeAsync();
        if (device_result.Status() != winrtaudio::AudioDeviceNodeCreationStatus::Success)
            throw std::exception{"device output node create failed"};
        auto device_output = device_result.DeviceOutputNode();
        auto raw_input = output_graph_.CreateFrameInputNode(output_graph_.EncodingProperties());
        raw_input.AddOutgoingConnection(device_output);
        raw_input.QuantumStarted({this, &AudioSystem::OutputAudioGraph_QuantumStarted});
        output_graph_.Start();
    }

    {
        Windows::Devices::Enumeration::DeviceInformation capture_dev{nullptr};

        auto capture_settings =
            winrtaudio::AudioGraphSettings{Windows::Media::Render::AudioRenderCategory::GameChat};
        capture_settings.QuantumSizeSelectionMode(
            winrtaudio::QuantumSizeSelectionMode::LowestLatency);
        auto capture_graph_result = co_await winrtaudio::AudioGraph::CreateAsync(capture_settings);
        if (capture_graph_result.Status() != winrtaudio::AudioGraphCreationStatus::Success)
            throw std::exception{"capturing audio graph create failed"};
        capture_graph_ = capture_graph_result.Graph();
        capture_graph_.EncodingProperties().ChannelCount(capture_setup_.NumChannels());
        capture_graph_.EncodingProperties().SampleRate(capture_setup_.SamplesPerChannelPerSecond());

        auto device_result = co_await capture_graph_.CreateDeviceInputNodeAsync(
            Windows::Media::Capture::MediaCategory::GameChat, capture_graph_.EncodingProperties(),
            capture_dev);
        if (device_result.Status() != winrtaudio::AudioDeviceNodeCreationStatus::Success)
            throw std::exception{"device input node create failed"};
        capture_output_ = capture_graph_.CreateFrameOutputNode(capture_graph_.EncodingProperties());
        device_result.DeviceInputNode().AddOutgoingConnection(capture_output_);
        capture_graph_.QuantumStarted({this, &AudioSystem::CaptureAudioGraph_QuantumStarted});
        capture_graph_.Start();
    }
}

void AudioSystem::DecodeForOutput(const ByteChunk& encoded_bytes) {
    opus_decoder_.DecodeToBuffer(encoded_bytes);
}

void AudioSystem::OutputAudioGraph_QuantumStarted(
    winrtaudio::AudioFrameInputNode node,
    winrtaudio::FrameInputNodeQuantumStartedEventArgs const& args) {
    // Some useful sample code for raw audio frame production in UWP
    // apps is at <https://blurt.chat/l/DCiiSQxo>, from noted
    // treasure Raymond Chen

    // Note that args.RequiredSamples is actually samples per channel:
    // https://blurt.chat/l/8e73WHLf
    auto samples = args.RequiredSamples();
    if (samples == 0) return;
    auto frame = opus_decoder_.ConsumeAudio(samples);
    if (frame == nullptr) return;
    node.AddFrame(frame);
}

void AudioSystem::CaptureAudioGraph_QuantumStarted(winrtaudio::AudioGraph graph,
                                                   Windows::Foundation::IInspectable) {
    auto frame = capture_output_.GetFrame();
    if (frame == nullptr) return;
    std::optional<Windows::Foundation::TimeSpan> duration{frame.Duration()};
    if (!duration.has_value()) return;
    if (duration.value().count() == 0) return;

    opus_encoder_.BufferRawAudio(frame);
}

}  // namespace winrt::blurt::implementation
