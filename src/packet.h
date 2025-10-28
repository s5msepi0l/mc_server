#pragma once

#include "util.h"
#include "types.h"

#define BUFFER_SIZE 4096

enum class net_read_state : u8 {
    SUCCESS,
    DISCONNECTED,
    INVALID_FORMAT,
    READ_ERROR
};

class Packet {
    private:
        i32 size = 0;
        Packet_id id;

        net_read_state state;

    public:
        std::vector<u8> buffer;
        i32 offset = 0;

        const net_read_state get_status() { return this->state; }
        const Packet_id get_id() { return this->id; }

        // read packet into memory
        void get(i32 fd) {
            buffer.resize(BUFFER_SIZE);
            i64 n = read(fd, buffer.data(), BUFFER_SIZE);
            if (packet_status_errored(n)) return ;

            size = read_varint_b(buffer.data(), offset);
            size += varint_len(size);
            std::cout << "varint size: " << size << std::endl;
            std::cout << "read() size: " << n << std::endl;

            id = (Packet_id)entry_to_byte(buffer.data(), offset);
        }

        u8 *get_buffer() { return this->buffer.data(); }

        void render_console() {
            std::cout << "length: " << size
                    << "\nid: " << static_cast<i32>(id)
                    << "\noffset: " << offset << '\n';

            std::cout << "Raw packet data (hex):\n";
            for (size_t i = 0; i < size; i++) {
                std::cout << std::hex << (int)buffer[i] << " ";
                if ((i + 1) % 10 == 0) std::cout << '\n';
            }
            std::cout << "\n\nPrintable ASCII:\n";
            for (size_t i = 0; i < size; i++) {
                char c = buffer[i];
                std::cout << (std::isprint(c) ? c : '.') << " ";
                if ((i + 1) % 10 == 0) std::cout << '\n';
            }
            std::cout << std::dec << "\n"; // reset number format
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
                return 0;
            }
 };