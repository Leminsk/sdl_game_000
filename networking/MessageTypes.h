#pragma once

#include <cstdint>

enum class MessageTypes : uint32_t {
	ServerAccept,
	ServerDeny,
	ServerPing,
    ClientPing,
	MessageAll,
	ServerMessage,
	ClientGetOwnID,
    ClientWarnDisconnect,
    GetUsersStatus,
    UsersStatus,
};