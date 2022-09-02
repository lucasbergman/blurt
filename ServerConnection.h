#pragma once
#include "mumble.ServerConnection.g.h"

#include "ControlSocket.h"
#include "winrt/Windows.Foundation.h"

namespace winrt::blurt::mumble::implementation {

struct ServerConnection : ServerConnectionT<ServerConnection> {
   public:
    ServerConnection() = default;
    ~ServerConnection() { Close(); }

    Windows::Foundation::IAsyncAction Connect(hstring host, hstring port, hstring userName,
                                              hstring password);
    void Close() noexcept;

    event_token ConnectionSucceeded(mumble::NetworkMessage const& handler);
    void ConnectionSucceeded(event_token const& token) noexcept;
    event_token ConnectionFailed(mumble::NetworkMessage const& handler);
    void ConnectionFailed(event_token const& token) noexcept;
    event_token ConnectionClosed(mumble::NetworkMessage const& handler);
    void ConnectionClosed(event_token const& token) noexcept;
    event_token PacketReceived(mumble::NetworkMessage const& handler);
    void PacketReceived(event_token const& token) noexcept;
    event_token AudioPacketReceived(mumble::AudioMessage const& handler);
    void AudioPacketReceived(event_token const& token) noexcept;

   private:
    Windows::Foundation::IAsyncAction SendPings();
    Windows::Foundation::IAsyncAction ReadControlPackets();

    bool closed_{false};
    Windows::Foundation::IAsyncAction ping_task_, read_task_;
    ControlSocket socket_;
    event<mumble::NetworkMessage> event_conn_succeeded_;
    event<mumble::NetworkMessage> event_conn_failed_;
    event<mumble::NetworkMessage> event_conn_closed_;
    event<mumble::NetworkMessage> event_packet_recv_;
    event<mumble::AudioMessage> audio_packet_recv_;
};
}  // namespace winrt::blurt::mumble::implementation

namespace winrt::blurt::mumble::factory_implementation {
struct ServerConnection : ServerConnectionT<ServerConnection, implementation::ServerConnection> {};
}  // namespace winrt::blurt::mumble::factory_implementation
