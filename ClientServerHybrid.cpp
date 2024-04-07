#include <iostream>

#include "networking/Client.hpp"
#include "networking/Server.hpp"
#include "networking/MessageTypes.h"


int main() {

    std::string mode;
    std::cout << "Mode: ";
    std::cin >> mode;

    if (mode == "client") {
        Client c;
        c.Connect("127.0.0.1", 60000);

        bool key[5] = { false, false, false, false, false };
        bool old_key[5] = { false, false, false, false, false };

        bool bQuit = false;
        while (!bQuit) {
            if (GetForegroundWindow() == GetConsoleWindow()) {
                key[0] = GetAsyncKeyState('1') & 0x8000;
                key[1] = GetAsyncKeyState('2') & 0x8000;
                key[2] = GetAsyncKeyState('3') & 0x8000;
                key[3] = GetAsyncKeyState('4') & 0x8000;
                key[4] = GetAsyncKeyState('0') & 0x8000;
            }

            if (key[0] && !old_key[0]) {
                c.PingServer();
                // std::cout << "Pressed 1\n";
            }
            if (key[1] && !old_key[1]) {
                c.MessageAll();
                // std::cout << "Pressed 2\n";
            }
            if (key[2] && !old_key[2]) {
                c.WarnDisconnect();
                c.Disconnect();
                // std::cout << "Pressed 3\n";
                bQuit = true;
            }
            if (key[3] && !old_key[3]) {
                c.ClientGetOwnID();
                // std::cout << "Pressed 4\n";
            }
            if (key[4] && !old_key[4]) {
                c.GetUsersStatus();
                // std::cout << "Pressed 0\n";
            }

            for (int i = 0; i < 5; i++) old_key[i] = key[i];

            if (c.IsConnected()) {
                if (!c.Incoming().empty()) {
                    auto msg = c.Incoming().pop_front().msg;

                    switch (msg.header.id) {
                        case MessageTypes::ServerAccept: {
                            std::string msg_content;
                            msg >= msg_content;
                            std::cout << msg_content << "\n";                            
                        }
                        break;

                        case MessageTypes::ServerPing: {
                            std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
                            std::chrono::system_clock::time_point timeThen;
                            msg >> timeThen;
                            std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
                        }
                        break;

                        case MessageTypes::ServerMessage: {
                            // uint32_t clientID;
                            // msg >> clientID;
                            // std::cout << "Hello from [" << clientID << "]\n";
                            std::string msg_content;
                            msg >= msg_content;
                            std::cout << msg_content << "\n";    
                        }
                        break;

                        case MessageTypes::ClientGetOwnID: {
                            uint32_t own_id;
                            msg >> own_id;
                            std::cout << "Own ID: " << own_id << "\n";
                        }
                        break;

                        case MessageTypes::ClientPing: {
                            c.ClientPingResponse(msg);
                        }
                        break;

                        case MessageTypes::UsersStatus: {
                            std::string msg_content;
                            msg >= msg_content;
                            std::cout << msg_content << "\n";
                        }
                        break;
                    }
                }
            } else {
                std::cout << "Server Down\n";
                bQuit = true;
            }
        }
    } else if (mode == "server") {
        Server server(60000); 
        server.Start();

        while (1) {
            server.Update(-1, true);
        }
    }
	
	


	return 0;
}