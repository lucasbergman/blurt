#pragma once

#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace winrt::blurt {

// Very lightly encapsulates a contiguous run of uint8 with storage managed by
// std::vector under the hood. The only features that really matter here are
// (1) length is guaranteed to fit in int32 and (2) a ByteChunk can't be
// implicitly copied, so it's a bit more obvious whether storage is being
// copied or moved.
class ByteChunk {
   public:
    ByteChunk(ByteChunk&&) = default;
    ByteChunk(std::vector<std::uint8_t>&& bytes) : bytes_{std::move(bytes)} {
        if (bytes_.size() > std::numeric_limits<std::int32_t>::max())
            throw std::overflow_error{"implausibly enormous byte chunk"};
    }

    static ByteChunk CopyOf(const std::string& s) {
        if (s.size() > std::numeric_limits<std::int32_t>::max())
            throw std::overflow_error{"implausibly enormous byte chunk"};
        return ByteChunk{reinterpret_cast<const std::uint8_t*>(s.data()),
                         static_cast<std::int32_t>(s.size())};
    }

    const std::vector<std::uint8_t>& Bytes() const { return bytes_; }
    operator const std::uint8_t*() const { return &bytes_[0]; }
    std::int32_t size() const { return static_cast<std::int32_t>(bytes_.size()); }
    auto begin() const { return bytes_.cbegin(); }
    auto end() const { return bytes_.cend(); }

    // Disable accidental copying (and copy-assignment)
    ByteChunk(const ByteChunk&) = delete;
    ByteChunk& operator=(const ByteChunk&) = delete;

   private:
    ByteChunk(const std::uint8_t* data, std::int32_t len) : bytes_{data, data + len} {}

    const std::vector<std::uint8_t> bytes_;
};
}  // namespace winrt::blurt
