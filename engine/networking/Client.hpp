#pragma once

#include <chrono>
#include "olcPGEX_Network.h"
#include "MessageTypes.h"

class Client : public olc::net::client_interface<MessageTypes> {
public:
Client() {}
~Client() {}

// attempts to connect to host. returns true on successfull connection
bool JoinGame(const std::string& host_ip, const uint16_t host_port=50000) {
    this->Connect(host_ip, host_port);
    // poor man's timeout
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point now;
    while(!this->IsConnected()) {
        now = std::chrono::steady_clock::now();
        if(std::chrono::duration_cast<std::chrono::seconds>(now - begin).count() > 4) {
            std::cout << "Could not connect to server (timed out).\n";
            return false;
        }
    }
    return true;
}

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