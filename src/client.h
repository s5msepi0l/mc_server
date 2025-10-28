#pragma once

#include "util.h"
#include "types.h"
#include "packet.h"

#include "mc_server.h"
#include "world.h"

using namespace std::chrono;

enum class Client_state : u8{
    handshaking = 0,
    status = 1,
    logging_in = 2,
    disconnected,
    connected
};

class Client {
    private:
        Client_state state;
        i32 fd;

        std::shared_ptr<Player> player;

    public:
        Client(i32 fd): fd(fd) {
            player = std::make_shared<Player>();
        }

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
                    packet.render_console();
                    if (packet.get_status() == net_read_state::SUCCESS) {
                        

                        switch(packet.get_id()) {
                            case Packet_id::SETUP:
                                handle_handshake(packet);
                                break;
                            
                        }
                    } else if (packet.get_status() == net_read_state::DISCONNECTED || packet.get_status() == net_read_state::READ_ERROR) break;
                }

                
                /* get world data and stuff
                */

            }
        }

        void handle_handshake(Packet &packet) {
            // Should probably in the future find some way to double check intent and stored intent
            if (state == Client_state::handshaking) {
                i32 version = read_varint_b(packet.get_buffer(), packet.offset); // 772
                std::string source = entry_to_string(packet.get_buffer(), packet.offset);
    
                u16 port = entry_to_short(packet.get_buffer(), packet.offset); // 25565
    
                i32 intent = read_varint_b(packet.get_buffer(), packet.offset); // 1, 2 or 3 but Im not coding allat

                switch((Client_state)intent) {
                    case Client_state::status:
                        set_state(Client_state::status);
                        break;
                    
                    case Client_state::logging_in:
                        set_state(Client_state::logging_in);
                        break;
                }

              std::cout << "source: " << source << "\nport: " << (i32) port << "\nintent: " << intent << std::endl;
                return ;
            }

            if (state == Client_state::status) {
                json status;

                status["version"] = {
                    {"name", "1.21.8"},
                    {"protocol", 772}
                };

                status["players"] = {
                    {"max", 20},
                    {"online", 1},
                    {"sample", json::array({
                        {
                            {"name", "cakeless"},
                            {"id", "0541ed27-7595-4e6a-9101-6c07f879b7b5"}
                        }
                    })}
                };

                status["description"] = {
                    {"text", "Hello, world!"}
                };

                std::string data = status.dump();

                // make reponse packet
                i32 len = 1; // id varint size
                len += varint_len(data.size()); // json length
                len += data.size();

                write_varint(fd, len); //id
                write_varint(fd, 0x00);
                write_string(fd, data);
                
                return ;
            }

            if (state == Client_state::logging_in) {
                std::cout << "intent: " << (i32)state << "\n";
                std::string username = entry_to_string(packet.get_buffer(), packet.offset);
                UUID uuid = read_uuid(packet.get_buffer(), packet.offset);
                std::string uuid_string = uuid_to_string(uuid);

                std::cout << "username: " << username <<
                "\nuuid: " << uuid_string << "\n";


                username = "hello";
                player->set_username(username);
                player->set_uuid(uuid_string);

                i32 payload_len = varint_len(0x02)
                    + 16
                    + varint_len(username.size()) + username.size()
                    + 1;

                write_varint(fd, payload_len);
                write_varint(fd, 0x02);
                write_uuid(fd, uuid);
                write_string(fd, username);
                write_varint(fd, 0); // empty array

            }

        }
};