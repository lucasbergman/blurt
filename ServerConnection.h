#pragma once

#include <cstdint>
#include <vector>
#include "AudioPacket.h"
#include "ControlSocket.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/base.h"

namespace winrt::blurt::mumble::implementation {
class ServerConnection {
   public:
    ServerConnection() = default;
    ~ServerConnection() { Close(); }

    Windows::Foundation::IAsyncAction Connect(hstring host, hstring port, hstring userName,
                                              hstring password);
    Windows::Foundation::IAsyncAction SendAudioAsync(std::vector<std::uint8_t>&& bytes);
    void Close() noexcept;

    winrt::event_token ConnectionSucceeded(winrt::delegate<winrt::hstring> const& handler);
    void ConnectionSucceeded(winrt::event_token const& token) noexcept;
    winrt::event_token ConnectionFailed(winrt::delegate<winrt::hstring> const& handler);
    void ConnectionFailed(winrt::event_token const& token) noexcept;
    winrt::event_token ConnectionClosed(winrt::delegate<winrt::hstring> const& handler);
    void ConnectionClosed(winrt::event_token const& token) noexcept;
    winrt::event_token PacketReceived(winrt::delegate<winrt::hstring> const& handler);
    void PacketReceived(winrt::event_token const& token) noexcept;
    winrt::event_token AudioPacketReceived(winrt::delegate<const AudioPacket&> const& handler);
    void AudioPacketReceived(winrt::event_token const& token) noexcept;

   private:
    Windows::Foundation::IAsyncAction SendPings();
    Windows::Foundation::IAsyncAction ReadControlPackets();

    bool closed_{false};
    Windows::Foundation::IAsyncAction ping_task_, read_task_;
    ControlSocket socket_;
    std::uint32_t audio_frame_seq_{0};
    winrt::event<winrt::delegate<winrt::hstring>> event_conn_succeeded_;
    winrt::event<winrt::delegate<winrt::hstring>> event_conn_failed_;
    winrt::event<winrt::delegate<winrt::hstring>> event_conn_closed_;
    winrt::event<winrt::delegate<winrt::hstring>> event_packet_recv_;
    winrt::event<winrt::delegate<const AudioPacket&>> audio_packet_recv_;
};
}  // namespace winrt::blurt::mumble::implementation
