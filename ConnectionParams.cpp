#include "pch.h"

#include "ConnectionParams.h"

#include "ConnectionParams.g.cpp"

namespace winrt::blurt::implementation {
namespace data = Windows::UI::Xaml::Data;

void ConnectionParams::Host(hstring const& host) {
    if (host != host_) {
        host_ = host;
        property_changed_(*this, data::PropertyChangedEventArgs{L"Host"});
    }
}

void ConnectionParams::Port(hstring const& port) {
    if (port != port_) {
        port_ = port;
        property_changed_(*this, data::PropertyChangedEventArgs{L"Port"});
    }
}

void ConnectionParams::UserName(hstring const& user_name) {
    if (user_name != user_name_) {
        user_name_ = user_name;
        property_changed_(*this, data::PropertyChangedEventArgs{L"UserName"});
    }
}

void ConnectionParams::Password(hstring const& password) {
    if (password != password_) {
        password_ = password;
        property_changed_(*this, data::PropertyChangedEventArgs{L"Password"});
    }
}

event_token ConnectionParams::PropertyChanged(data::PropertyChangedEventHandler const& handler) {
    return property_changed_.add(handler);
}

void ConnectionParams::PropertyChanged(event_token const& token) noexcept {
    property_changed_.remove(token);
}

}  // namespace winrt::blurt::implementation
