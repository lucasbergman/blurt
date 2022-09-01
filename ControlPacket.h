#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include "Mumble.pb.h"

namespace winrt::blurt::mumble::implementation {

// Make the underlying type int32, since packet types cross the wire as uint16,
// and conversion from uint16 to int32 is always safe.
enum class ControlPacketType : std::int32_t {
    INVALID = -1,
    Version = 0,
    UDPTunnel = 1,
    Authenticate = 2,
    Ping = 3,
    Reject = 4,
    ServerSync = 5,
    ChannelRemove = 6,
    ChannelState = 7,
    UserRemove = 8,
    UserState = 9,
    BanList = 10,
    TextMessage = 11,
    PermissionDenied = 12,
    ACL = 13,
    QueryUsers = 14,
    CryptSetup = 15,
    ContextActionModify = 16,
    ContextAction = 17,
    UserList = 18,
    VoiceTarget = 19,
    PermissionQuery = 20,
    CodecVersion = 21,
    UserStats = 22,
    RequestBlob = 23,
    ServerConfig = 24,
    SuggestConfig = 25,
    PluginDataTransmission = 26,
};

namespace internal {
constexpr std::int32_t kMaxPacketTypeValue =
    static_cast<std::int32_t>(ControlPacketType::PluginDataTransmission);
}

// Throws PacketParseError for out-of-range values
ControlPacketType ControlPacketTypeOf(std::uint16_t value);

const std::string ToString(ControlPacketType t);

class PacketParseError : public std::exception {
   public:
    PacketParseError(const char* const msg) : std::exception{msg} {}
};

class ControlPacket {
   public:
    ControlPacket(ControlPacketType t, std::vector<std::uint8_t>&& msg) : type_{t}, msg_{msg} {}

    // Make a new ControlPacket by taking a copy of a string
    ControlPacket(ControlPacketType t, std::string s)
        : ControlPacket(t, std::vector<std::uint8_t>{s.data(), s.data() + s.size()}) {}

    ControlPacketType Type() const { return type_; }
    std::uint16_t TypeAsUInt() const { return static_cast<std::uint16_t>(type_); }
    const std::vector<std::uint8_t>& Bytes() const { return msg_; }
    std::string DebugString() const;

    template <typename T>
    T Size() const {
        assert(msg_.size() < std::numeric_limits<T>::max());
        return static_cast<T>(msg_.size());
    }

    // Resolve<ControlPacketType::T>() parses the packet's payload into the
    // protobuf message of type MumbleProto::T; throws PacketParseError if the
    // byte payload doesn't parse
#define RESOLVE_PROTO_IMPL(T)                                                           \
    template <ControlPacketType Ty>                                                     \
    std::enable_if_t<Ty == ControlPacketType::T, MumbleProto::T> ResolveProto() const { \
        MumbleProto::T result;                                                          \
        assert(msg_.size() < std::numeric_limits<int>::max());                          \
        if (!result.ParseFromArray(msg_.data(), static_cast<int>(msg_.size())))         \
            throw PacketParseError("invalid protobuf message");                         \
        return result;                                                                  \
    }

    RESOLVE_PROTO_IMPL(Version)
    // UDPTunnel intentionally omitted: it's special; see below
    RESOLVE_PROTO_IMPL(Authenticate)
    RESOLVE_PROTO_IMPL(Ping)
    RESOLVE_PROTO_IMPL(Reject)
    RESOLVE_PROTO_IMPL(ServerSync)
    RESOLVE_PROTO_IMPL(ChannelRemove)
    RESOLVE_PROTO_IMPL(ChannelState)
    RESOLVE_PROTO_IMPL(UserRemove)
    RESOLVE_PROTO_IMPL(UserState)
    RESOLVE_PROTO_IMPL(BanList)
    RESOLVE_PROTO_IMPL(TextMessage)
    RESOLVE_PROTO_IMPL(PermissionDenied)
    RESOLVE_PROTO_IMPL(ACL)
    RESOLVE_PROTO_IMPL(QueryUsers)
    RESOLVE_PROTO_IMPL(CryptSetup)
    RESOLVE_PROTO_IMPL(ContextActionModify)
    RESOLVE_PROTO_IMPL(ContextAction)
    RESOLVE_PROTO_IMPL(UserList)
    RESOLVE_PROTO_IMPL(VoiceTarget)
    RESOLVE_PROTO_IMPL(PermissionQuery)
    RESOLVE_PROTO_IMPL(CodecVersion)
    RESOLVE_PROTO_IMPL(UserStats)
    RESOLVE_PROTO_IMPL(RequestBlob)
    RESOLVE_PROTO_IMPL(ServerConfig)
    RESOLVE_PROTO_IMPL(SuggestConfig)
    RESOLVE_PROTO_IMPL(PluginDataTransmission)

    // From(proto) creates a ControlPacket from a protobuf message.
    //
    // TODO: Avoid the extra copy here
#define FROM_PROTO_IMPL(T)                                                     \
    static ControlPacket From(const MumbleProto::T& proto) {                   \
        return ControlPacket(ControlPacketType::T, proto.SerializeAsString()); \
    }

    FROM_PROTO_IMPL(Version)
    // UDPTunnel intentionally omitted; its payload isn't a proto
    FROM_PROTO_IMPL(Authenticate)
    FROM_PROTO_IMPL(Ping)
    FROM_PROTO_IMPL(Reject)
    FROM_PROTO_IMPL(ServerSync)
    FROM_PROTO_IMPL(ChannelRemove)
    FROM_PROTO_IMPL(ChannelState)
    FROM_PROTO_IMPL(UserRemove)
    FROM_PROTO_IMPL(UserState)
    FROM_PROTO_IMPL(BanList)
    FROM_PROTO_IMPL(TextMessage)
    FROM_PROTO_IMPL(PermissionDenied)
    FROM_PROTO_IMPL(ACL)
    FROM_PROTO_IMPL(QueryUsers)
    FROM_PROTO_IMPL(CryptSetup)
    FROM_PROTO_IMPL(ContextActionModify)
    FROM_PROTO_IMPL(ContextAction)
    FROM_PROTO_IMPL(UserList)
    FROM_PROTO_IMPL(VoiceTarget)
    FROM_PROTO_IMPL(PermissionQuery)
    FROM_PROTO_IMPL(CodecVersion)
    FROM_PROTO_IMPL(UserStats)
    FROM_PROTO_IMPL(RequestBlob)
    FROM_PROTO_IMPL(ServerConfig)
    FROM_PROTO_IMPL(SuggestConfig)
    FROM_PROTO_IMPL(PluginDataTransmission)

   private:
    const ControlPacketType type_;
    const std::vector<std::uint8_t> msg_;
};

}  // namespace winrt::blurt::mumble::implementation
