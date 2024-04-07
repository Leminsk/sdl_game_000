#pragma once

#include <olcPGEX_Network.h>
#include <MessageTypes.h>

class Client : public olc::net::client_interface<MessageTypes> {
    public:
        void PingServer();
        void MessageAll();
        void ClientGetOwnID();
        void WarnDisconnect();
        void GetUsersStatus();
        void ClientPingResponse(olc::net::message<MessageTypes>& msg);
};