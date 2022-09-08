#include "pch.h"

#include "ServerConnection.h"

#include <chrono>
#include <sstream>
#include <utility>
#include "AudioPacket.h"
#include "ControlPacket.h"

namespace winrt::blurt::mumble::implementation {

namespace foundation = winrt::Windows::Foundation;

foundation::IAsyncAction ServerConnection::SendPings() {
    while (true) {
        // When the user requests a connection close, this coroutine gets
        // canceled and destroyed when it's suspended here
        co_await std::chrono::seconds(10);
        MumbleProto::Ping ping;
        auto now = std::chrono::system_clock::now();
        ping.set_timestamp(
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
        socket_.WritePacketAsync(ControlPacket::From(ping));
    }
}

foundation::IAsyncAction ServerConnection::ReadControlPackets() {
    try {
        while (true) {
            mumble::WireMessage wire_packet = co_await socket_.ReadPacketAsync();
            // TODO: What happens on a read exception?
            ControlPacket packet{std::move(wire_packet)};
            if (packet.Type() == ControlPacketType::UDPTunnel) {
                audio_packet_recv_(packet.ResolveAudioPacket());
            } else {
                event_packet_recv_(L"packet received: " + winrt::to_hstring(packet.DebugString()));
            }
        }
    } catch (const winrt::hresult_canceled&) {
        co_return;
    } catch (const winrt::hresult_error& e) {
        std::wstringstream ss;
        ss << "failed to read control packet: " << e.message().c_str();
        event_conn_failed_(winrt::hstring{ss.str()});
    }
}

foundation::IAsyncAction ServerConnection::Connect(hstring host, hstring port, hstring userName,
                                                   hstring password) {
    try {
        co_await socket_.ConnectAsync(host, port);

        {
            mumble::WireMessage wire_packet = co_await socket_.ReadPacketAsync();
            // TODO: What happens on a read exception?
            ControlPacket packet{std::move(wire_packet)};
            if (packet.Type() != ControlPacketType::Version) {
                event_conn_failed_(L"server did not send the required version message; giving up");
                co_return;
            }

            try {
                auto version = packet.ResolveProto<ControlPacketType::Version>();
                event_packet_recv_(L"packet received: " + winrt::to_hstring(packet.DebugString()));
            } catch (const PacketParseError&) {
                // TODO: Handle errors
                event_conn_failed_(L"bogus control message from server instead of version");
                throw;
            }
        }

        MumbleProto::Version my_version;
        my_version.set_version((1 << 16) + (2 << 8) + 4);  // 1.2.4
        my_version.set_os("UWP");
        my_version.set_release("Blurt 0.0.0");
        co_await socket_.WritePacketAsync(ControlPacket::From(my_version));

        MumbleProto::Authenticate auth;
        auth.set_username(winrt::to_string(userName));
        auth.set_password(winrt::to_string(password));
        auth.set_opus(true);
        co_await socket_.WritePacketAsync(ControlPacket::From(auth));

        // TODO: Handle auth failure

        event_conn_succeeded_(L"success");
        ping_task_ = SendPings();
        read_task_ = ReadControlPackets();

    } catch (const winrt::hresult_error& e) {
        std::wstringstream ss;
        ss << "connection setup failed: " << e.message().c_str();
        event_conn_failed_(winrt::hstring{ss.str()});
    }
}

foundation::IAsyncAction ServerConnection::SendAudioAsync(std::vector<std::uint8_t>&& bytes) {
    AudioPacket ap(AudioPacketType::Opus, audio_frame_seq_, std::move(bytes));
    audio_frame_seq_ += 2;  // TODO: magical constant only works for 20-ms audio frames
    co_await socket_.WritePacketAsync(ControlPacket::From(std::move(ap)));
}

void ServerConnection::Close() noexcept {
    if (closed_) return;
    ping_task_.Cancel();
    read_task_.Cancel();
    socket_.Close();
    event_conn_closed_(L"closed");
    closed_ = true;
}

winrt::event_token ServerConnection::ConnectionSucceeded(
    winrt::delegate<winrt::hstring> const& handler) {
    return event_conn_succeeded_.add(handler);
}
void ServerConnection::ConnectionSucceeded(winrt::event_token const& token) noexcept {
    event_conn_succeeded_.remove(token);
}
winrt::event_token ServerConnection::ConnectionFailed(
    winrt::delegate<winrt::hstring> const& handler) {
    return event_conn_failed_.add(handler);
}
void ServerConnection::ConnectionFailed(winrt::event_token const& token) noexcept {
    event_conn_succeeded_.remove(token);
}
winrt::event_token ServerConnection::ConnectionClosed(
    winrt::delegate<winrt::hstring> const& handler) {
    return event_conn_closed_.add(handler);
}
void ServerConnection::ConnectionClosed(winrt::event_token const& token) noexcept {
    event_conn_closed_.remove(token);
}
winrt::event_token ServerConnection::PacketReceived(
    winrt::delegate<winrt::hstring> const& handler) {
    return event_packet_recv_.add(handler);
}
void ServerConnection::PacketReceived(winrt::event_token const& token) noexcept {
    event_packet_recv_.remove(token);
}
winrt::event_token ServerConnection::AudioPacketReceived(
    winrt::delegate<const AudioPacket&> const& handler) {
    return audio_packet_recv_.add(handler);
}
void ServerConnection::AudioPacketReceived(winrt::event_token const& token) noexcept {
    audio_packet_recv_.remove(token);
}
}  // namespace winrt::blurt::mumble::implementation
