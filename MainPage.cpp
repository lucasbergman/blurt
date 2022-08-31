#include "pch.h"

#include "MainPage.h"

#include "MainPage.g.cpp"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::blurt::implementation {
MainPage::MainPage() { view_model_ = make<implementation::ConnectionViewModel>(); }

void MainPage::Connect_Click(IInspectable const&, RoutedEventArgs const&) {
    throw hresult_not_implemented();
}
}  // namespace winrt::blurt::implementation
