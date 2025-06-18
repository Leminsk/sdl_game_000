#pragma once

#include <olcPGEX_Network.h>
#include <MessageTypes.h>

class Server : public olc::net::server_interface<MessageTypes> {
    public:
        Server(uint16_t nPort=50000, std::string server_name="PLACEHOLDER");
        ~Server() {};
    protected:
        std::string name; // dummy ip localhost
        uint16_t port;
        uint32_t clients_amount = 0;
        std::unordered_map<uint32_t, double> clients_ping = {};
        std::unordered_map<uint32_t, bool> requested_ping = {};
        virtual bool OnClientConnect(std::shared_ptr<olc::net::connection<MessageTypes>> client);
        virtual void OnClientDisconnect(std::shared_ptr<olc::net::connection<MessageTypes>> client);
        virtual void OnMessage(std::shared_ptr<olc::net::connection<MessageTypes>> client, olc::net::message<MessageTypes>& msg);
};