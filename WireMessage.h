#pragma once
#include "mumble.WireMessage.g.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace winrt::blurt::mumble::implementation {

struct WireMessage : WireMessageT<WireMessage> {
   public:
    WireMessage(std::uint16_t type_number, winrt::array_view<std::uint8_t const> bytes)
        : type_{type_number}, bytes_{bytes.begin(), bytes.end()} {}
    std::uint16_t TypeNumber() const { return type_; }
    std::uint32_t PayloadSize() const {
        // Guaranteed safe: array_view.size is uint32
        return static_cast<std::uint32_t>(bytes_.size());
    }
    std::vector<std::uint8_t>&& StealPayload() { return std::move(bytes_); }

   private:
    const std::uint16_t type_;
    std::vector<uint8_t> bytes_;
};

}  // namespace winrt::blurt::mumble::implementation

namespace winrt::blurt::mumble::factory_implementation {
struct WireMessage : WireMessageT<WireMessage, implementation::WireMessage> {};
}  // namespace winrt::blurt::mumble::factory_implementation
