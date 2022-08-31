#pragma once
#include "ConnectionViewModel.g.h"

#include "ConnectionParams.h"

namespace winrt::blurt::implementation {
struct ConnectionViewModel : ConnectionViewModelT<ConnectionViewModel> {
    ConnectionViewModel() { params_ = make<implementation::ConnectionParams>(); }
    winrt::blurt::ConnectionParams Params() const noexcept { return params_; };

   private:
    winrt::blurt::ConnectionParams params_{nullptr};
};
}  // namespace winrt::blurt::implementation

namespace winrt::blurt::factory_implementation {
struct ConnectionViewModel
    : ConnectionViewModelT<ConnectionViewModel, implementation::ConnectionViewModel> {};
}  // namespace winrt::blurt::factory_implementation
