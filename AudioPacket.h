#pragma once

#include <cstdint>
#include <stdexcept>
#include <utility>
#include "ByteChunk.h"
#include "winrt/base.h"

namespace winrt::blurt::mumble::implementation {

struct AudioParseFailure : std::invalid_argument {
    AudioParseFailure(std::string s) : std::invalid_argument(s) {}
};

enum class AudioPacketType {
    CELTAlpha = 0,
    Ping = 1,
    Speex = 2,
    CELTBeta = 3,
    Opus = 4,
};

// Represents an audio packet sent in Mumble's legacy datagram format.
// Future versions of the protocol will use protobuf encoding for this;
// that'll be nice, but it's going to while before we can rely on that being
// supported. I've tried to make the underlying integer representations
// here forward-compatible with the new protobuf definition.
//
// Realistically, this only supports Opus for audio right now.
class AudioPacket {
   public:
    // Parse an incoming audio packet; can throw AudioParseFailure
    static AudioPacket FromIncomingBytes(const ByteChunk& bytes) { return FromBytes(bytes, true); }

    // Parse an outgoing audio packet; can throw AudioParseFailure
    static AudioPacket FromOutgoingBytes(const ByteChunk& bytes) { return FromBytes(bytes, false); }

    // Move a chunk of encoded bytes into a new audio packet
    AudioPacket(AudioPacketType type, std::uint32_t target, std::uint64_t frame_seq,
                std::uint32_t sender_session, bool is_terminator, bool has_position_info,
                ByteChunk&& encoded_bytes)
        : type_{type},
          target_{target},
          frame_seq_{frame_seq},
          sender_session_{sender_session},
          is_terminator_{is_terminator},
          has_position_info_{has_position_info},
          payload_{std::move(encoded_bytes)} {}

    AudioPacket(AudioPacketType type, std::uint64_t frame_seq, ByteChunk&& encoded_bytes)
        : AudioPacket{type, 0, frame_seq, 0, false, false, std::move(encoded_bytes)} {}

    AudioPacketType Type() const { return type_; }
    std::uint32_t Target() const { return target_; }
    std::uint32_t SenderSession() const { return sender_session_; }
    std::uint64_t FrameSequence() const { return frame_seq_; }
    bool IsTerminator() const { return is_terminator_; }
    bool HasPositionInfo() const { return has_position_info_; }
    std::uint16_t PayloadSize() {
        // See comment in the constructor for why this is safe
        return static_cast<std::uint16_t>(payload_.size());
    }
    const ByteChunk& Payload() const { return payload_; }
    ByteChunk EncodeOutgoing() const;
    winrt::hstring DebugString();

   private:
    static AudioPacket FromBytes(const ByteChunk& bytes, bool contains_sender);

    AudioPacketType type_;
    std::uint32_t target_{0};
    std::uint32_t sender_session_{0};
    std::uint64_t frame_seq_;
    bool is_terminator_;
    bool has_position_info_;
    ByteChunk payload_;
};

}  // namespace winrt::blurt::mumble::implementation
