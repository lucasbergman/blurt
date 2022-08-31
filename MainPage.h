#pragma once

#include "MainPage.g.h"

#include "ConnectionViewModel.h"

namespace winrt::blurt::implementation {
struct MainPage : MainPageT<MainPage> {
    MainPage();
    winrt::blurt::ConnectionViewModel MainViewModel() const noexcept { return view_model_; }
    void Connect_Click(Windows::Foundation::IInspectable const& sender,
                       Windows::UI::Xaml::RoutedEventArgs const& args);

   private:
    winrt::blurt::ConnectionViewModel view_model_{nullptr};
};
}  // namespace winrt::blurt::implementation

namespace winrt::blurt::factory_implementation {
struct MainPage : MainPageT<MainPage, implementation::MainPage> {};
}  // namespace winrt::blurt::factory_implementation
