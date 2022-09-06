#include "pch.h"

#include "MainPage.h"

#include "MainPage.g.cpp"

#include <debugapi.h>
#include <utility>
#include <vector>

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace {
void OnMessage(hstring msg) { OutputDebugString((msg + L"\n").c_str()); }
}  // namespace

namespace winrt::blurt::implementation {
MainPage::MainPage() { view_model_ = make<implementation::ConnectionViewModel>(); }

Windows::Foundation::IAsyncAction MainPage::Connect_Click(IInspectable const&,
                                                          RoutedEventArgs const&) {
    audio_system_.SetUp();

    const auto& params = view_model_.Params();
    connection_.ConnectionSucceeded(OnMessage);
    connection_.ConnectionFailed(OnMessage);
    connection_.ConnectionClosed(OnMessage);
    connection_.PacketReceived(OnMessage);
    connection_.AudioPacketReceived([this](mumble::implementation::AudioPacket packet) {
        audio_system_.DecodeForOutput(packet.Payload());
    });
    co_await connection_.Connect(params.Host(), params.Port(), params.UserName(),
                                 params.Password());
    audio_system_.EncodedCaptureReady([this](std::vector<std::uint8_t> bytes) {
        // TODO: this move is a bug; can't have more than one handler
        connection_.SendAudioAsync(std::move(bytes));
    });
}

}  // namespace winrt::blurt::implementation
