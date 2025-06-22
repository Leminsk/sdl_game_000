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
	ClientGetOwnColor,
    ClientWarnDisconnect,
    GetUsersStatus,
    UsersStatus,
	ServerState_Colors,
	ServerState_Drones,
	ClientState_Drones
};