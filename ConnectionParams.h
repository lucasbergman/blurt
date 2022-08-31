#pragma once
#include "ConnectionParams.g.h"

namespace winrt::blurt::implementation {
struct ConnectionParams : ConnectionParamsT<ConnectionParams> {
    ConnectionParams() = default;

    hstring Host() const noexcept { return host_; };
    void Host(hstring const& value);
    hstring Port() const noexcept { return port_; };
    void Port(hstring const& value);
    hstring UserName() const noexcept { return user_name_; };
    void UserName(hstring const& value);
    hstring Password() const noexcept { return password_; };
    void Password(hstring const& value);

    event_token PropertyChanged(
        winrt::Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
    void PropertyChanged(event_token const& token) noexcept;

   private:
    hstring host_;
    hstring port_{L"64738"};
    hstring user_name_;
    hstring password_;
    event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> property_changed_;
};
}  // namespace winrt::blurt::implementation

namespace winrt::blurt::factory_implementation {
struct ConnectionParams : ConnectionParamsT<ConnectionParams, implementation::ConnectionParams> {};
}  // namespace winrt::blurt::factory_implementation
