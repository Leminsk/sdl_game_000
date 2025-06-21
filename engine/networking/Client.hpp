#pragma once

#include "olcPGEX_Network.h"
#include "MessageTypes.h"

class Client : public olc::net::client_interface<MessageTypes> {
    public:
        Client() {}
        ~Client() {}

        void PingServer() {
            olc::net::message<MessageTypes> msg;
            msg.header.id = MessageTypes::ServerPing;

            // Caution with this...
            std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();		

            msg << timeNow;
            Send(msg);
        }

        void MessageAll() {
            olc::net::message<MessageTypes> msg;
            msg.header.id = MessageTypes::MessageAll;		
            Send(msg);
        }

        void ClientGetOwnID() {
            olc::net::message<MessageTypes> msg;
            msg.header.id = MessageTypes::ClientGetOwnID;
            Send(msg);
        }

        void WarnDisconnect() {
            olc::net::message<MessageTypes> msg;
            msg.header.id = MessageTypes::ClientWarnDisconnect;
            Send(msg);
        }

        void GetUsersStatus(){
            olc::net::message<MessageTypes> msg;
            msg.header.id = MessageTypes::GetUsersStatus;
            Send(msg);
        }

        void ClientPingResponse(olc::net::message<MessageTypes>& msg) {
            // bounce back
            Send(msg);
        }
        
};