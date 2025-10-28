#pragma once

#include "util.h"
#include "types.h"
#include "packet.h"

#include "client.h"

// In the future the port should prolly be read from a config
// parser or sum
#define PORT 25565 
#define BACKLOG 256
#define BUFFER_SIZE 1024

class Server {
    private:
        i32 server_fd;
        struct sockaddr_in address;

        std::vector<std::shared_ptr<Client>> connected_clients;

        std::mutex client_connect;

    public:
        Server() {
            server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd < 0) {
                std::cerr << "socket()\n";
            }

            i32 opt = 1;
            setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(i32));

            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(PORT);

            if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
                std::cerr << "bind()\n";    
            }

            if (listen(server_fd, BACKLOG) < 0) {
                std::cerr << "listen()\n";
            }
        }

        // cool trick I saw on reddit
        // now if instead of passing a pointer 20 times, you can just
        // get the server instance via Server::Instance() altough
        // it does require c++ 20
        static Server &Instance() {
            static Server instance;
            return instance;
        }

        // remove copy and move constructors so its impossible
        // to accidentally create another server instance

        Server(const Server &) = delete;
        Server(const Server &&) = delete;

        Server &operator=(const Server &) = delete;
        Server &operator=(const Server &&) = delete;

        // run ts on a seperate thread
        void run() {
            socklen_t addrlen = sizeof(address);
            while (true) {
                i32 client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
            
                if (client_fd < 0) {
                    std::cerr << "listen()\n";
                }
                
                {
                    std::lock_guard lockClient(client_connect);

                    std::shared_ptr<Client> client = std::make_shared<Client>(client_fd);
                    client->set_state(Client_state::handshaking);

                    connected_clients.push_back(client);

                    std::jthread client_thread(&Client::handle_client, client.get(), client_fd);
                    client_thread.detach();
                }
            }

            close(server_fd);
        }
};


/*
#include <queue>
#include <mutex>
#include <string>

std::queue<std::string> action_queue;
std::mutex action_mutex;

// Client thread pushes actions
void Server::handleClient(int fd) {
    while (true) {
        std::string action = readAction(fd);  // Your packet parsing
        if (action.empty()) break; // disconnect

        std::lock_guard<std::mutex> lock(action_mutex);
        action_queue.push(action);  // thread-safe push
    }
}

// Main thread pops actions
void Server::run() {
    while (true) {
        std::vector<std::string> actions_to_process;
        {
            std::lock_guard<std::mutex> lock(action_mutex);
            while (!action_queue.empty()) {
                actions_to_process.push_back(action_queue.front());
                action_queue.pop();
            }
        }

        for (auto& act : actions_to_process) {
            applyAction(act); // modify world state
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
*/