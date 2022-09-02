#pragma once

#include "mumble.AudioPacket.g.h"

#include <cstdint>
#include <stdexcept>
#include <vector>
#include "winrt/base.h"

namespace winrt::blurt::mumble::implementation {

struct AudioParseFailure : std::invalid_argument {
    AudioParseFailure(std::string s) : std::invalid_argument(s) {}
};

struct AudioPacket : AudioPacketT<AudioPacket> {
   public:
    // Parse an audio packet from a chunk of bytes; can throw AudioParseFailure
    AudioPacket(const std::vector<std::uint8_t>& bytes);

    // Move a chunk of encoded bytes into a new audio packet
    AudioPacket(mumble::AudioPacketType type, std::uint32_t frame_seq,
                std::vector<std::uint8_t>&& encoded_bytes);

    mumble::AudioPacketType Type() const { return type_; }
    std::uint32_t Target() const { return target_; }
    std::uint32_t SenderSession() const { return sender_session_; }
    std::uint64_t FrameSequence() const { return frame_seq_; }
    bool IsTerminator() const { return is_terminator_; }
    bool HasPositionInfo() const { return has_position_info_; }
    uint16_t PayloadSize() {
        // See comment in the constructor for why this is safe
        return static_cast<std::uint16_t>(payload_.size());
    }
    const std::vector<std::uint8_t>& Payload() const { return payload_; }
    const std::vector<std::uint8_t> Encode() const;
    winrt::hstring DebugString();

   private:
    mumble::AudioPacketType type_;
    std::uint32_t target_{0};
    std::uint32_t sender_session_{0};
    std::uint64_t frame_seq_;
    bool is_terminator_;
    bool has_position_info_;
    std::vector<std::uint8_t> payload_;
};
}  // namespace winrt::blurt::mumble::implementation
