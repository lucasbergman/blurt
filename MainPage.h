#pragma once

#include "MainPage.g.h"

#include "AudioSystem.h"
#include "ConnectionViewModel.h"
#include "ServerConnection.h"
#include "winrt/Windows.Foundation.h"

namespace winrt::blurt::implementation {
struct MainPage : MainPageT<MainPage> {
    MainPage();
    blurt::ConnectionViewModel MainViewModel() const noexcept { return view_model_; }
    Windows::Foundation::IAsyncAction Connect_Click(Windows::Foundation::IInspectable const& sender,
                                                    Windows::UI::Xaml::RoutedEventArgs const& args);

   private:
    blurt::ConnectionViewModel view_model_{nullptr};
    blurt::mumble::implementation::ServerConnection connection_;
    AudioSystem audio_system_;
};
}  // namespace winrt::blurt::implementation

namespace winrt::blurt::factory_implementation {
struct MainPage : MainPageT<MainPage, implementation::MainPage> {};
}  // namespace winrt::blurt::factory_implementation
