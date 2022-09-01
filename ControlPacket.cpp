#include "pch.h"

#include "ControlPacket.h"

#include <limits>
#include <map>
#include <sstream>
#include "google/protobuf/dynamic_message.h"

namespace winrt::blurt::mumble::implementation {

ControlPacketType ControlPacketTypeOf(std::uint16_t value) {
    if (value > internal::kMaxPacketTypeValue)
        throw PacketParseError{"out of range control packet type"};
    return static_cast<ControlPacketType>(value);
}

const std::string ToString(ControlPacketType t) {
    static std::map<ControlPacketType, std::string> m{
        {ControlPacketType::INVALID, "INVALID"},
        {ControlPacketType::Version, "Version"},
        {ControlPacketType::UDPTunnel, "UDPTunnel"},
        {ControlPacketType::Authenticate, "Authenticate"},
        {ControlPacketType::Ping, "Ping"},
        {ControlPacketType::Reject, "Reject"},
        {ControlPacketType::ServerSync, "ServerSync"},
        {ControlPacketType::ChannelRemove, "ChannelRemove"},
        {ControlPacketType::ChannelState, "ChannelState"},
        {ControlPacketType::UserRemove, "UserRemove"},
        {ControlPacketType::UserState, "UserState"},
        {ControlPacketType::BanList, "BanList"},
        {ControlPacketType::TextMessage, "TextMessage"},
        {ControlPacketType::PermissionDenied, "PermissionDenied"},
        {ControlPacketType::ACL, "ACL"},
        {ControlPacketType::QueryUsers, "QueryUsers"},
        {ControlPacketType::CryptSetup, "CryptSetup"},
        {ControlPacketType::ContextActionModify, "ContextActionModify"},
        {ControlPacketType::ContextAction, "ContextAction"},
        {ControlPacketType::UserList, "UserList"},
        {ControlPacketType::VoiceTarget, "VoiceTarget"},
        {ControlPacketType::PermissionQuery, "PermissionQuery"},
        {ControlPacketType::CodecVersion, "CodecVersion"},
        {ControlPacketType::UserStats, "UserStats"},
        {ControlPacketType::RequestBlob, "RequestBlob"},
        {ControlPacketType::ServerConfig, "ServerConfig"},
        {ControlPacketType::SuggestConfig, "SuggestConfig"},
    };
    auto it = m.find(t);
    if (it == m.end()) return std::string{"INVALID"};
    return it->second;
}

std::string ControlPacket::DebugString() const {
    static std::map<ControlPacketType, const google::protobuf::Descriptor*> m{
        {ControlPacketType::Version, MumbleProto::Version::GetDescriptor()},
        {ControlPacketType::UDPTunnel, nullptr},  // special non-protobuf message type
        {ControlPacketType::Authenticate, MumbleProto::Authenticate::GetDescriptor()},
        {ControlPacketType::Ping, MumbleProto::Ping::GetDescriptor()},
        {ControlPacketType::Reject, MumbleProto::Reject::GetDescriptor()},
        {ControlPacketType::ServerSync, MumbleProto::ServerSync::GetDescriptor()},
        {ControlPacketType::ChannelRemove, MumbleProto::ChannelRemove::GetDescriptor()},
        {ControlPacketType::ChannelState, MumbleProto::ChannelState::GetDescriptor()},
        {ControlPacketType::UserRemove, MumbleProto::UserRemove::GetDescriptor()},
        {ControlPacketType::UserState, MumbleProto::UserState::GetDescriptor()},
        {ControlPacketType::BanList, MumbleProto::BanList::GetDescriptor()},
        {ControlPacketType::TextMessage, MumbleProto::TextMessage::GetDescriptor()},
        {ControlPacketType::PermissionDenied, MumbleProto::PermissionDenied::GetDescriptor()},
        {ControlPacketType::ACL, MumbleProto::ACL::GetDescriptor()},
        {ControlPacketType::QueryUsers, MumbleProto::QueryUsers::GetDescriptor()},
        {ControlPacketType::CryptSetup, MumbleProto::CryptSetup::GetDescriptor()},
        {ControlPacketType::ContextActionModify, MumbleProto::ContextActionModify::GetDescriptor()},
        {ControlPacketType::ContextAction, MumbleProto::ContextAction::GetDescriptor()},
        {ControlPacketType::UserList, MumbleProto::UserList::GetDescriptor()},
        {ControlPacketType::VoiceTarget, MumbleProto::VoiceTarget::GetDescriptor()},
        {ControlPacketType::PermissionQuery, MumbleProto::PermissionQuery::GetDescriptor()},
        {ControlPacketType::CodecVersion, MumbleProto::CodecVersion::GetDescriptor()},
        {ControlPacketType::UserStats, MumbleProto::UserStats::GetDescriptor()},
        {ControlPacketType::RequestBlob, MumbleProto::RequestBlob::GetDescriptor()},
        {ControlPacketType::ServerConfig, MumbleProto::ServerConfig::GetDescriptor()},
        {ControlPacketType::SuggestConfig, MumbleProto::SuggestConfig::GetDescriptor()},
        {ControlPacketType::PluginDataTransmission,
         MumbleProto::PluginDataTransmission::GetDescriptor()},
    };
    auto it = m.find(type_);
    assert(it != m.end());

    std::stringstream result;
    result << ToString(type_) << ": ";

    if (type_ == ControlPacketType::UDPTunnel) {
        result << msg_.size() << " bytes";
        return result.str();
    }

    assert(it->second);
    google::protobuf::DynamicMessageFactory factory;
    auto* protomsg = factory.GetPrototype(it->second);
    std::unique_ptr<google::protobuf::Message> msg{protomsg->New()};
    if (msg->ParseFromArray(msg_.data(), Size<int>())) {
        result << msg->Utf8DebugString();
    } else {
        result << "protobuf parse failed";
    }
    return result.str();
}

}  // namespace winrt::blurt::mumble::implementation
