#pragma once
#include <unordered_map>
#include "olcPGEX_Network.h"
#include "MessageTypes.h"

class Server : public olc::net::server_interface<MessageTypes> {
    public:
        Server(uint16_t nPort=50000, std::string server_name="PLACEHOLDER") : olc::net::server_interface<MessageTypes>(nPort) {
            this->port = nPort;
            this->name = server_name;
        }
        ~Server() {};

    protected:
        std::string name; // dummy ip localhost
        uint16_t port;
        uint32_t clients_amount = 0;
        std::unordered_map<uint32_t, double> clients_ping = {};
        std::unordered_map<uint32_t, bool> requested_ping = {};


        virtual bool OnClientConnect(std::shared_ptr<olc::net::connection<MessageTypes>> client) {
            this->clients_amount++;

            olc::net::message<MessageTypes> msg, broadcast_msg;

            msg.header.id = MessageTypes::ServerAccept;
            std::stringstream msg_stream_content;
            msg_stream_content << "Connected to " << this->name << " as user id [" << this->nIDCounter << "]";
            std::string msg_content = msg_stream_content.str();
            msg <= msg_content;
            client->Send(msg);

            broadcast_msg.header.id = MessageTypes::ServerMessage;
            std::stringstream broadcast_stream_content;
            broadcast_stream_content << "New user connected [" << this->nIDCounter << "]";
            // std::string broadcast_content = broadcast_stream_content.str();
            broadcast_msg <= broadcast_stream_content.str();
            MessageAllClients(broadcast_msg, client);
            return true;
        }

        virtual void OnClientDisconnect(std::shared_ptr<olc::net::connection<MessageTypes>> client) {
            this->clients_amount--;
            std::cout << "Removing client [" << client->GetID() << "]\n";
            this->clients_ping.erase(client->GetID());
        }

        virtual void OnMessage(std::shared_ptr<olc::net::connection<MessageTypes>> client, olc::net::message<MessageTypes>& msg)  {
            uint32_t client_id = client->GetID();

            switch (msg.header.id) {
                case MessageTypes::ServerPing: {
                    std::cout << "[" << client_id << "]: Server Ping\n";
                    // Simply bounce message back to client
                    client->Send(msg);
                }
                break;

                case MessageTypes::MessageAll: {
                    std::cout << "[" << client_id << "]: Message All\n";
                    // Construct a new message and send it to all clients
                    olc::net::message<MessageTypes> broadcast_msg;
                    broadcast_msg.header.id = MessageTypes::ServerMessage;
                    broadcast_msg <= std::to_string(client_id);
                    MessageAllClients(broadcast_msg, client);
                }
                break;

                case MessageTypes::ClientGetOwnID: {
                    std::cout << "[" << client_id << "]: Client get own id\n";

                    olc::net::message<MessageTypes> response;
                    response.header.id = MessageTypes::ClientGetOwnID;
                    response << client_id;
                    client->Send(response);
                }
                break;

                case MessageTypes::ClientWarnDisconnect: {
                    std::cout << "[" << client_id << "] warn disconnect\n";
                    
                    olc::net::message<MessageTypes> broadcast_msg;
                    broadcast_msg.header.id = MessageTypes::ServerMessage;
                    std::stringstream broadcast_stream_content;
                    broadcast_stream_content << "User [" << client_id << "] disconnected";
                    // std::string broadcast_content = broadcast_stream_content.str();
                    broadcast_msg <= broadcast_stream_content.str();
                    MessageAllClients(broadcast_msg, client);
                }
                break;

                case MessageTypes::GetUsersStatus: {                    
                    std::cout << "[" << client_id << "]: GetUsersStatus\n";
                    olc::net::message<MessageTypes> broadcast_msg;
                    broadcast_msg.header.id = MessageTypes::ClientPing;
                    // store the original sender
                    broadcast_msg << client_id;

                    this->requested_ping[client_id] = true;

                    // Caution with this...
                    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                    broadcast_msg << timeNow;
                    MessageAllClients(broadcast_msg);
                }
                break;

                case MessageTypes::ClientPing: {
                    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                    std::chrono::system_clock::time_point timeThen;
                    msg >> timeThen;
                    this->clients_ping[client_id] = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - timeThen).count();

                    // got all clients' status
                    if (this->clients_ping.size() == this->clients_amount) {
                        uint32_t original_sender_id;
                        msg >> original_sender_id;

                        if (this->requested_ping[original_sender_id] == true) {
                            this->requested_ping[original_sender_id] = false;
                            olc::net::message<MessageTypes> ping_status_msg;
                            ping_status_msg.header.id = MessageTypes::UsersStatus;

                            std::stringstream status_content_stream;
                            status_content_stream << " User  |  Ping\n";

                            for (auto& [id, ping] : this->clients_ping) {
                                status_content_stream << " " << id << " | " << ping << " ms\n";
                            }

                            std::string status_content = status_content_stream.str();
                            std::cout << "ClientPing:\n" << status_content << "\n";
                            ping_status_msg <= status_content;
                            
                            MessageClientByID(original_sender_id, ping_status_msg);
                        }
                        
                    }
                }
                break;
            }
        }
        
};