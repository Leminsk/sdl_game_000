#pragma once
#include <unordered_map>
#include <vector>
#include "olcPGEX_Network.h"
#include "MessageTypes.h"
#include "../Game.hpp"
#include "../Vector2D.hpp"
#include "../ECS/DroneComponent.hpp"
#include "../GroupLabels.hpp"


class Server : public olc::net::server_interface<MessageTypes> {
    public:
        Server(uint16_t nPort=50000, std::string server_name="PLACEHOLDER") : olc::net::server_interface<MessageTypes>(nPort) {
            this->port = nPort;
            this->name = server_name;
        }
        ~Server() {};

        void PingAllClients() {
            olc::net::message<MessageTypes> broadcast_msg;
            broadcast_msg.header.id = MessageTypes::ClientPing;
            // Caution with this...
            std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
            broadcast_msg << timeNow;
            MessageAllClients(broadcast_msg);
        }

    protected:
        std::string name; // dummy ip localhost
        uint16_t port;
        uint32_t clients_amount = 0;
        std::unordered_map<uint32_t, int> clients_ping = {};
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



        virtual void OnMessage(std::shared_ptr<olc::net::connection<MessageTypes>> client, olc::net::message<MessageTypes>& msg) {
            uint32_t client_id = client->GetID();

            switch (msg.header.id) {
                case MessageTypes::ServerPing: {
                    std::cout << "[" << client_id << "]: Server Ping\n";
                    // Simply bounce message back to client
                    client->Send(msg);
                } break;

                case MessageTypes::MessageAll: {
                    std::cout << "[" << client_id << "]: Message All\n";
                    // Construct a new message and send it to all clients
                    olc::net::message<MessageTypes> broadcast_msg;
                    broadcast_msg.header.id = MessageTypes::ServerMessage;
                    broadcast_msg <= std::to_string(client_id);
                    MessageAllClients(broadcast_msg, client);
                } break;

                case MessageTypes::ClientGetOwnID: {
                    std::cout << "[" << client_id << "]: Client get own id\n";

                    olc::net::message<MessageTypes> response;
                    response.header.id = MessageTypes::ClientGetOwnID;
                    response << client_id;
                    client->Send(response);
                } break;

                case MessageTypes::ClientWarnDisconnect: {
                    std::cout << "[" << client_id << "] warn disconnect\n";
                    
                    olc::net::message<MessageTypes> broadcast_msg;
                    broadcast_msg.header.id = MessageTypes::ServerMessage;
                    std::stringstream broadcast_stream_content;
                    broadcast_stream_content << "User [" << client_id << "] disconnected";
                    // std::string broadcast_content = broadcast_stream_content.str();
                    broadcast_msg <= broadcast_stream_content.str();
                    MessageAllClients(broadcast_msg, client);
                } break;

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
                } break;

                case MessageTypes::ClientPing: {
                    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                    std::chrono::system_clock::time_point timeThen;
                    msg >> timeThen;
                    this->clients_ping[client_id] = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - timeThen).count();
                    // got all clients' status
                    if (this->clients_ping.size() == this->clients_amount) {
                        olc::net::message<MessageTypes> ping_status_msg;
                        ping_status_msg.header.id = MessageTypes::UsersStatus;
                        for (auto& [id, ping] : this->clients_ping) {
                            ping_status_msg << ping;
                            ping_status_msg << id;
                        }
                        ping_status_msg << this->clients_amount;
                        MessageAllClients(ping_status_msg);
                        // empty the hashmap to only send once per client per ping loop
                        this->clients_ping.clear();
                    }
                } break;

                case MessageTypes::ClientState_Drones: {
                    // since the packet takes some time to arrive, 
                    // the drone should be moved forward on its path by the number of frames equivalent to the time it took to get the packet
                    // in order to sync it with the client
                    int average_frames_passed = static_cast<int>((this->clients_ping[client_id]/1000.0f) * Game::AVERAGE_FPS);
                    DroneComponent* drone;
                    int drone_counter;
                    int drone_path_size;
                    std::string drone_id;
                    std::vector<Vector2D> drone_path;
                    Vector2D v;
                    Vector2D previous_pos;
                    msg >> drone_counter;
                    for(int i=0; i<drone_counter; ++i) {
                        msg >= drone_id;
                        msg >> drone_path_size;
                        drone_path = {};
                        for(int j=0; j<drone_path_size; ++j) {
                            msg >> v.x;
                            msg >> v.y;
                            drone_path.push_back(v);
                        }
                        drone = &Game::manager->getEntity(drone_id)->getComponent<DroneComponent>();
                        drone->moveToPointWithPath(drone_path);
                        // sync on path
                        for(int j=0; j<average_frames_passed; ++j) {
                            previous_pos = drone->transform->position;
                            drone->preUpdate();
                            drone->update();
                            drone->handleStaticCollisions(previous_pos, Game::manager->getGroup(groupTiles), Game::manager->getGroup(groupBuildings));
                            drone->handleDynamicCollisions(Game::manager->getGroup(groupDrones));
                            drone->handleCollisionTranslations();
                            drone->handleOutOfBounds(Game::world_map_layout_width, Game::world_map_layout_height);
                        }                        
                    }
                } break;
            }
        }

};