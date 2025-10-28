#pragma once

#include "util.h"
#include "types.h"

enum class net_read_state : u8 {
    SUCCESS,
    DISCONNECTED,
    INVALID_FORMAT,
    READ_ERROR
};

class Packet {
    private:
        i32 size;
        Packet_id id;

        net_read_state state;

    public:
        std::vector<u8> buffer;
        u32 offset = 0;

        const net_read_state get_status() { return this->state; }
        const Packet_id get_id() { return this->id; }

        // read packet into memory
        void get(i32 fd) {
            i32 position = 0;
            u8 current_byte;

            while (true) {
                i64 n = read(fd, &current_byte, 1);
                if (packet_status_errored(n)) return ;


                offset++;

                size |= (current_byte & SEGMENT_BITS) << position;

                if ((current_byte & CONTINUE_BIT) == 0) break;

                position += 7;

                if (position >= 32) {
                    size = -1;
                    state = net_read_state::INVALID_FORMAT;
                    break;
                }
            }

            id = (Packet_id)read_byte(fd);

            buffer.resize(size);
            i64 n = read(fd, buffer.data(), size);
        }


        private:

            // if this returns 1, the packet is either errored or client disconnected
            i32 packet_status_errored(i64 n) {
                if (n == 0) {
                    state = net_read_state::DISCONNECTED;
                    return 1;
                } else if (n < 0) {
                    state = net_read_state::READ_ERROR;
                    return 1;
                }

            }
 };