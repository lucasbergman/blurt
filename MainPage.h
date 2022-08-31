﻿#pragma once

#include "MainPage.g.h"

namespace winrt::blurt::implementation {
struct MainPage : MainPageT<MainPage> {
    MainPage() {
        // Xaml objects should not call InitializeComponent during construction.
        // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
    }

    int32_t MyProperty();
    void MyProperty(int32_t value);

    void ClickHandler(Windows::Foundation::IInspectable const& sender,
                      Windows::UI::Xaml::RoutedEventArgs const& args);
};
}  // namespace winrt::blurt::implementation

namespace winrt::blurt::factory_implementation {
struct MainPage : MainPageT<MainPage, implementation::MainPage> {};
}  // namespace winrt::blurt::factory_implementation
