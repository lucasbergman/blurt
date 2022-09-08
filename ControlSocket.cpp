#include "pch.h"

#include "ControlSocket.h"

#include <winerror.h>
#include "winrt/Windows.Networking.h"
#include "winrt/Windows.Storage.Streams.h"

namespace winrt::blurt::mumble::implementation {

namespace foundation = winrt::Windows::Foundation;
namespace net = winrt::Windows::Networking;
namespace sockets = winrt::Windows::Networking::Sockets;
namespace streams = winrt::Windows::Storage::Streams;

ControlSocket::ControlSocket() : open_{false} {
    socket_.Control().QualityOfService(sockets::SocketQualityOfService::LowLatency);
}

void ControlSocket::Close() {
    if (!open_) return;
    socket_.Close();
    open_ = false;
}

foundation::IAsyncAction ControlSocket::ConnectAsync(const winrt::hstring& host,
                                                     const winrt::hstring& port) {
    net::EndpointPair endpoint{nullptr, L"", net::HostName{host}, port};
    // TODO: handle connect exceptions
    co_await socket_.ConnectAsync(endpoint, sockets::SocketProtectionLevel::Tls12);
    open_ = true;
}

foundation::IAsyncOperation<blurt::mumble::WireMessage> ControlSocket::ReadPacketAsync() {
    streams::DataReader reader{socket_.InputStream()};
    reader.ByteOrder(streams::ByteOrder::BigEndian);
    {
        auto read_op = reader.LoadAsync(6);
        auto bytes_read = co_await read_op;
        if (bytes_read != 6) {
            if (read_op.ErrorCode() != S_OK) {
                winrt::throw_hresult(read_op.ErrorCode());
            }
            // TODO: Handle remote end close properly
            throw std::exception{"remote end closed"};
        }
    }
    const auto msg_type = reader.ReadUInt16();
    const auto msg_len = reader.ReadUInt32();
    auto bytes_to_read = msg_len;
    auto buf = std::make_unique<std::uint8_t[]>(msg_len);
    auto* b = buf.get();
    while (bytes_to_read > 0) {
        auto read_op = reader.LoadAsync(bytes_to_read);
        auto n = co_await read_op;
        if (read_op.ErrorCode() != S_OK) {
            winrt::throw_hresult(read_op.ErrorCode());
        }
        if (n == 0) {
            // TODO: Handle remote end close properly
            throw std::exception{"remote end closed"};
        }
        reader.ReadBytes({b, b + n});
        b += n;
        bytes_to_read -= n;
    }
    reader.DetachStream();
    co_return {msg_type, {buf.get(), b}};
}

foundation::IAsyncAction ControlSocket::WritePacketAsync(ControlPacket&& packet) {
    streams::DataWriter writer{socket_.OutputStream()};
    writer.ByteOrder(streams::ByteOrder::BigEndian);

    writer.WriteUInt16(packet.TypeAsUInt());
    writer.WriteUInt32(packet.PayloadSize());
    writer.WriteBytes(packet.Bytes());
    co_await writer.StoreAsync();
    writer.DetachStream();
}

}  // namespace winrt::blurt::mumble::implementation
