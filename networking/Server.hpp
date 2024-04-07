#pragma once

#include <olcPGEX_Network.h>
#include <MessageTypes.h>

class Server : public olc::net::server_interface<MessageTypes> {
    public:
        Server(uint16_t nPort);
    protected:
        std::string ip = "[::ffff:127.0.0.1]"; // dummy ip localhost
        uint16_t port = 30000;
        uint32_t clients_amount = 0;
        std::unordered_map<uint32_t, double> clients_ping = {};
        std::unordered_map<uint32_t, bool> requested_ping = {};
        virtual bool OnClientConnect(std::shared_ptr<olc::net::connection<MessageTypes>> client);
        virtual void OnClientDisconnect(std::shared_ptr<olc::net::connection<MessageTypes>> client);
        virtual void OnMessage(std::shared_ptr<olc::net::connection<MessageTypes>> client, olc::net::message<MessageTypes>& msg);
};