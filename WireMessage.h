#pragma once
#include "mumble.WireMessage.g.h"

#include <limits>
#include <utility>

namespace winrt::blurt::mumble::implementation {
struct WireMessage : WireMessageT<WireMessage> {
    WireMessage(uint16_t type_number, array_view<uint8_t const> bytes)
        : type_{type_number}, bytes_{bytes.begin(), bytes.end()} {}
    uint16_t TypeNumber() const { return type_; }
    uint32_t PayloadSize() const {
        // Guaranteed safe: array_view.size is uint32
        return static_cast<uint32_t>(bytes_.size());
    }
    std::vector<uint8_t>&& MovePayload() { return std::move(bytes_); }

   private:
    const uint16_t type_;
    std::vector<uint8_t> bytes_;
};
}  // namespace winrt::blurt::mumble::implementation

namespace winrt::blurt::mumble::factory_implementation {
struct WireMessage : WireMessageT<WireMessage, implementation::WireMessage> {};
}  // namespace winrt::blurt::mumble::factory_implementation
