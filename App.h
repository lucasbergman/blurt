#pragma once
#include "App.xaml.g.h"

namespace winrt::blurt::implementation {
struct App : AppT<App> {
    App();
    void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const&);
    void OnSuspending(IInspectable const&, Windows::ApplicationModel::SuspendingEventArgs const&);
    void OnNavigationFailed(IInspectable const&,
                            Windows::UI::Xaml::Navigation::NavigationFailedEventArgs const&);
};
}  // namespace winrt::blurt::implementation
