#pragma once

#include "ControlPacket.h"
#include "WireMessage.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.Networking.Sockets.h"

namespace winrt::blurt::mumble::implementation {

// A thin abstraction over the Mumble protocol TCP control socket
class ControlSocket {
   public:
    ControlSocket();
    ~ControlSocket() { Close(); }

    // TODO: Work out how close should work
    void Close();

    // Connect to the given host and port asynchronously
    //
    // TODO: Handle errors properly
    Windows::Foundation::IAsyncAction ConnectAsync(const winrt::hstring& host,
                                                   const winrt::hstring& port);

    // Read the next control message off the wire
    //
    // TODO: Handle errors properly
    Windows::Foundation::IAsyncOperation<blurt::mumble::WireMessage> ReadPacketAsync();

    // Write a control packet to the wire
    //
    // TODO: Handle errors properly
    Windows::Foundation::IAsyncAction WritePacketAsync(ControlPacket&& packet);

   private:
    bool open_;
    Windows::Networking::Sockets::StreamSocket socket_;
};

}  // namespace winrt::blurt::mumble::implementation
