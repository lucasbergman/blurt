#include "pch.h"

#include "AudioPacket.h"

#include <limits>
#include <sstream>

namespace winrt::blurt::mumble::implementation {

namespace {
constexpr auto kMaxPacketTypeValue = static_cast<int8_t>(AudioPacketType::Opus);

// Reads through a Mumble protocol voice datagram from a view over the
// chunk of bytes sent over the wire. Consume*() operations throw an
// out_of_range exception if there aren't enough bytes left to fulfill the
// request or (in the case of ConsumeVarInt()) if the input is bogus.
class AudioPacketReader {
   public:
    AudioPacketReader(const std::vector<std::uint8_t>& bytes)
        : data_{bytes.data()}, count_{static_cast<std::uint32_t>(bytes.size())} {}

    std::uint32_t Remaining() const { return count_; }

    std::uint8_t ConsumeByte() {
        if (count_ == 0) {
            throw std::out_of_range{"no more bytes"};
        }
        std::uint8_t result = *data_;
        data_++;
        count_--;
        return result;
    }

    // Returns a copy of the next N bytes; throws if there aren't enough
    // available remaining bytes
    template <typename SizeT>
    std::vector<std::uint8_t> ConsumeBytes(SizeT count) {
        if (count > count_) {
            throw std::out_of_range{"insufficient bytes remaining"};
        }
        std::vector<std::uint8_t> result{data_, data_ + count};
        data_ += count;
        count_ -= count;
        return result;
    }

    // Consume a 64-bit varint from the packet; throws if the read would run
    // off the end or if the input is otherwise malformed.
    //
    // Parsing code for varints is derived from Mumble[1], which is
    // copyright The Mumble Developers and used under license. Full license
    // text is available in our LICENSE file and the Mumble website[2].
    //
    // 1. https://github.com/mumble-voip/mumble/blob/1d45d99/src/PacketDataStream.h
    // 2. https://www.mumble.info/LICENSE
    std::uint64_t ConsumeVarInt() {
        uint64_t result;

        auto v = ConsumeByteAsUInt64();
        if ((v & 0x80) == 0) {
            // 1-byte encoding, positive, 7 bits
            result = v & 0x7f;
        } else if ((v & 0xc0) == 0x80) {
            // 2-byte encoding, positive, 14 bits
            result = (v & 0x3f) << 8 | ConsumeByteAsUInt64();
        } else if ((v & 0xe0) == 0xc0) {
            // 3-byte encoding, positive, 21 bits
            result = (v & 0x1f) << 16 | ConsumeByteAsUInt64() << 8 | ConsumeByteAsUInt64();
        } else if ((v & 0xf0) == 0xe0) {
            // 4-byte encoding, positive, 28 bits
            result = (v & 0x0f) << 24 | ConsumeByteAsUInt64() << 16 | ConsumeByteAsUInt64() << 8 |
                     ConsumeByteAsUInt64();
        } else if ((v & 0xf0) == 0xf0) {
            switch (v & 0xfc) {
                case 0xf0:
                    // 5-byte encoding, positive, 32 bits
                    result = ConsumeByteAsUInt64() << 24 | ConsumeByteAsUInt64() << 16 |
                             ConsumeByteAsUInt64() << 8 | ConsumeByteAsUInt64();
                    break;
                case 0xf4:
                    // 9-byte encoding, positive, 64 bits
                    result = ConsumeByteAsUInt64() << 56 | ConsumeByteAsUInt64() << 48 |
                             ConsumeByteAsUInt64() << 40 | ConsumeByteAsUInt64() << 32 |
                             ConsumeByteAsUInt64() << 24 | ConsumeByteAsUInt64() << 16 |
                             ConsumeByteAsUInt64() << 8 | ConsumeByteAsUInt64();
                    break;
                case 0xf8:
                    // following is a two's-complement, negative varint
                    result = ~(ConsumeVarInt());
                    break;
                case 0xfc:
                    // 1-byte encoding, negative, 2 bits (really??)
                    result = ~(v & 0x03);
                    break;
                default:
                    throw std::out_of_range{"bogus varint format"};
            }
        } else {
            throw std::out_of_range{"bogus varint format"};
        }
        return result;
    }

    template <typename T>
    T ConsumeAndNarrowVarInt() {
        std::uint64_t val = ConsumeVarInt();
        if (val > std::numeric_limits<T>::max()) {
            throw std::out_of_range{"varint too wide"};
        }
        return static_cast<T>(val);
    }

   private:
    inline std::uint64_t ConsumeByteAsUInt64() { return ConsumeByte(); }

    const std::uint8_t* data_;
    std::uint32_t count_;
};

void WriteVarIntTo(std::vector<uint8_t>& v, uint64_t n) {
    if ((n & 0x8000000000000000LL) && (~n < 0x100000000LL)) {
        // two's-complement negative
        n = ~n;
        if (n <= 0x3) {
            // shortcut encoding for -1 to -4 (really??)
            v.push_back((n & 0x3) | 0xfc);
            return;
        } else {
            v.push_back(0xf8);
            // fall through to encode ~n
        }
    }
    if (n < 0x80) {
        // 1-byte encoding, positive, 7 bits
        v.push_back(n & 0x7f);
    } else if (n < 0x4000) {
        // 2-byte encoding, positive, 14 bits
        v.push_back(((n >> 8) & 0xff) | 0x80);
        v.push_back(n & 0xff);
    } else if (n < 0x200000) {
        // 3-byte encoding, positive, 21 bits
        v.push_back(((n >> 16) & 0xff) | 0xc0);
        v.push_back((n >> 8) & 0xff);
        v.push_back(n & 0xff);
    } else if (n < 0x10000000) {
        // 4-byte encoding, positive, 28 bits
        v.push_back(((n >> 24) & 0xff) | 0xe0);
        v.push_back((n >> 16) & 0xff);
        v.push_back((n >> 8) & 0xff);
        v.push_back(n & 0xff);
    } else if (n < 0x100000000ll) {
        // 5-byte encoding, positive, 32 bits
        v.push_back(0xf0);
        v.push_back((n >> 24) & 0xff);
        v.push_back((n >> 16) & 0xff);
        v.push_back((n >> 8) & 0xff);
        v.push_back(n & 0xff);
    } else {
        // 9-byte encoding, positive, 64 bits
        v.push_back(0xf4);
        v.push_back((n >> 56) & 0xff);
        v.push_back((n >> 48) & 0xff);
        v.push_back((n >> 40) & 0xff);
        v.push_back((n >> 32) & 0xff);
        v.push_back((n >> 24) & 0xff);
        v.push_back((n >> 16) & 0xff);
        v.push_back((n >> 8) & 0xff);
        v.push_back(n & 0xff);
    }
}

AudioPacketType TypeOf(std::uint8_t n) {
    if (n > kMaxPacketTypeValue) {
        std::stringstream ss;
        ss << "invalid datagram type value " << static_cast<int>(n);
        throw std::out_of_range(ss.str());
    }
    return static_cast<AudioPacketType>(n);
}
}  // namespace

AudioPacket::AudioPacket(const std::vector<std::uint8_t>& bytes, bool contains_sender) {
    try {
        AudioPacketReader reader{bytes};
        auto first_byte = reader.ConsumeByte();
        type_ = TypeOf((first_byte & 0xe0) >> 5);
        target_ = first_byte & 0x1f;

        if (contains_sender)
            sender_session_ = reader.ConsumeAndNarrowVarInt<decltype(sender_session_)>();

        frame_seq_ = reader.ConsumeVarInt();
        auto len_and_terminator = reader.ConsumeAndNarrowVarInt<std::uint16_t>();

        // This part of the format guarantees that the payload length is
        // expressible in 13 bits, so using uint16 for payload size is safe
        std::uint16_t len = len_and_terminator & 0x1fff;

        is_terminator_ = (len_and_terminator & 0x2000) != 0;
        auto remaining = reader.Remaining();
        if (remaining != len && remaining != len + 12u) {
            throw AudioParseFailure("invalid number of bytes remaining in datagram");
        }
        has_position_info_ = remaining != len;
        payload_ = reader.ConsumeBytes(len);
    } catch (const std::out_of_range& e) {
        std::stringstream ss;
        ss << "failed datagram packet parse: " << e.what();
        throw AudioParseFailure(ss.str());
    }
}

AudioPacket::AudioPacket(AudioPacketType type, uint32_t frame_seq,
                         std::vector<std::uint8_t>&& encoded_bytes)
    : type_{type},
      target_{0},
      frame_seq_{frame_seq},
      sender_session_{0},
      is_terminator_{false},
      has_position_info_{false},
      payload_{encoded_bytes} {}

const std::vector<std::uint8_t> AudioPacket::EncodeOutgoing() const {
    if (type_ != AudioPacketType::Opus) throw hresult_not_implemented{};
    if (payload_.size() > 0x1fff) throw std::out_of_range{"Audio payload size overflows 13 bits"};

    std::vector<uint8_t> result;
    uint8_t type_and_target = (static_cast<uint8_t>(type_) << 5) | (target_ & 0x1f);
    result.push_back(type_and_target);

    WriteVarIntTo(result, frame_seq_);

    uint16_t len_and_terminator = payload_.size() & 0x1fff;
    if (is_terminator_) len_and_terminator |= 0x2000;
    WriteVarIntTo(result, len_and_terminator);
    result.insert(result.end(), payload_.begin(), payload_.end());
    return result;
}

hstring AudioPacket::DebugString() {
    std::stringstream ss;
    ss << "AudioPacket("
       << "Type=" << static_cast<int>(type_) << ", "
       << "Target=" << target_ << ", "
       << "SenderSession=" << sender_session_ << ", "
       << "FrameSequence=" << frame_seq_ << ", "
       << "IsTerminator=" << is_terminator_ << ", "
       << "HasPositionInfo=" << has_position_info_ << ", "
       << "Payload[" << payload_.size() << "]"
       << ")";
    return to_hstring(ss.str());
}

}  // namespace winrt::blurt::mumble::implementation
