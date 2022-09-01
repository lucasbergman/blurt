#include "pch.h"

#include "MainPage.h"

#include "MainPage.g.cpp"

#include <debugapi.h>

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace {
void OnMessage(hstring msg) { OutputDebugString((msg + L"\n").c_str()); }
}  // namespace

namespace winrt::blurt::implementation {
MainPage::MainPage() { view_model_ = make<implementation::ConnectionViewModel>(); }

Windows::Foundation::IAsyncAction MainPage::Connect_Click(IInspectable const&,
                                                          RoutedEventArgs const&) {
    connection_ = make<mumble::implementation::ServerConnection>();
    const auto& params = view_model_.Params();
    connection_.ConnectionSucceeded(OnMessage);
    connection_.ConnectionFailed(OnMessage);
    connection_.ConnectionClosed(OnMessage);
    connection_.PacketReceived(OnMessage);
    co_await connection_.Connect(params.Host(), params.Port(), params.UserName(),
                                 params.Password());
}
}  // namespace winrt::blurt::implementation
