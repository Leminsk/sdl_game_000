#include "olcPGEX_Network.h"
#include "MessageTypes.h"
#include "Client.hpp"

void Client::PingServer() {
    olc::net::message<MessageTypes> msg;
    msg.header.id = MessageTypes::ServerPing;

    // Caution with this...
    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();		

    msg << timeNow;
    Send(msg);
}

void Client::MessageAll() {
    olc::net::message<MessageTypes> msg;
    msg.header.id = MessageTypes::MessageAll;		
    Send(msg);
}

void Client::ClientGetOwnID() {
    olc::net::message<MessageTypes> msg;
    msg.header.id = MessageTypes::ClientGetOwnID;
    Send(msg);
}

void Client::WarnDisconnect() {
    olc::net::message<MessageTypes> msg;
    msg.header.id = MessageTypes::ClientWarnDisconnect;
    Send(msg);
}

void Client::GetUsersStatus() {
    olc::net::message<MessageTypes> msg;
    msg.header.id = MessageTypes::GetUsersStatus;
    Send(msg);
}

void Client::ClientPingResponse(olc::net::message<MessageTypes>& msg) {
    // bounce back
    Send(msg);
}