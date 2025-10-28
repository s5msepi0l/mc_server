#pragma once

#include "util.h"
#include "types.h"
#include "packet.h"

#include "world.h"
#include "mc_server.h"

using namespace std::chrono;

#define BUFFER_SIZE 

enum class Client_state : u8{
    handshaking,
    status,
    logging_in,
    disconnected,
    connected
};

class Client {
    private:
        Client_state state;

        i32 fd;

    public:
        Client(i32 fd): fd(fd) {}

        void set_state(Client_state state) {
            this->state = state;
        }



        void handle_client(i32 fd) {
            std::cout << "handle_client(" << fd << ")\n";

            auto next_tick = steady_clock::now();

            struct pollfd pfd;
            pfd.fd = fd;
            pfd.events = POLLIN;

            while (true) {
                next_tick += milliseconds(50);
                
                i32 timeout_ms = duration_cast<milliseconds>(next_tick - steady_clock::now()).count();
                if (timeout_ms < 0) timeout_ms = 0;

                i32 ret = poll(&pfd, 1, timeout_ms);
                if (ret > 0 && (pfd.revents & POLLIN)) {
                    Packet packet;

                    packet.get(fd);
                    if (packet.get_status() == net_read_state::SUCCESS) {
                        switch(packet.get_id()) {
                            case Packet_id::HANDSHAKE:
                                break;
                            
                        }
                    }
                }

                
                /* get world data and stuff
                */

            }
        }
};