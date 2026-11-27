#pragma once

#include "util.h"
#include <stdint.h>

#define SEGMENT_BITS 0x7F
#define CONTINUE_BIT 0x80

#define VARINT_MAX_N 5 // maximum varint size

using UUID = std::array<u8, 16>;

enum class Packet_id : u8 {
    SETUP = 0x00,
    PING = 0x01,

    /*// handshaking
    HANDSHAKE =       0x00,
    STATUS_REQUEST =  0X00,
    STATUS_RESPONSE = 0X01,
    PING_REQUEST =    0X01,
    PING_RESPONSE =   0X02,

    //logging_in
    LOGIN_START =          0x00,
    ENCRYPTION_REQUEST =   0x01,
    ENCRYPTION_RESPONSE =  0x02,
    LOGIN_SUCCESS =        0x02,
    SET_COMPRESSION =      0x03,
    LOGIN_PLUGIN_REQUEST = 0x04,*/
};

u8 read_byte(i32 fd) {
    char byte;
    i64 n = read(fd, &byte, 1);
    if (n < 0) {
        std::cout << "read() failed\n";
        exit(EXIT_FAILURE);
    } else if (n == 0) {
        std::cout << "connection closed\n";
        exit(EXIT_FAILURE);
    }
    return byte;
}

inline void write_byte(i32 fd, u8 byte) {
    i64 n = write(fd, &byte, 1);
    if (n < 0) {
        std::cerr << "write() failed\n";
        exit(EXIT_FAILURE);
    } else if (n == 0) {
        std::cerr << "connection closed while writing\n";
        exit(EXIT_FAILURE);
    }
}

//https://minecraft.wiki/w/Java_Edition_protocol/Packets#VarInt_and_VarLong


i32 read_varint_b(u8 *buffer, i32 &offset) {
    i32 value = 0;
    i32 position = 0;
    u8 current_byte;

    while (true) {
        current_byte = buffer[offset++];
        value |= (current_byte & SEGMENT_BITS) << position;

        if ((current_byte & CONTINUE_BIT) == 0) break;

        position += 7;

        if (position >= 32) {
            std::cerr << "VarInt is too big\n";
            exit(EXIT_FAILURE);
        }
    }

    return value;
}

i64 read_varlong_b(u8 *buffer, i32 &offset) {
    i64 value = 0;
    i32 position = 0;
    u8 current_byte;

    while (true) {
        current_byte = buffer[offset++];
        value |= (i64) (current_byte & SEGMENT_BITS) << position;

        if ((current_byte & CONTINUE_BIT) == 0) break;

        position += 7;

        if (position >= 64) {
            std::cerr << "VarLong is too big\n";
            exit(EXIT_FAILURE);
        }

    }

    return value;
}

i32 varint_len(i32 value) {
    i32 len = 0;
    do {
        len++;
        value >>= 7;
    } while(value != 0 && value != -1);
}

void write_varint(i32 fd, i32 value) {
    while(true) {
        if ((value & ~SEGMENT_BITS) == 0) {
            write_byte(fd, value);
            return ;
        }

        write_byte(fd, (value & SEGMENT_BITS) | CONTINUE_BIT);

        value >>= 7;
    }
}

void write_varlong(i32 fd, i64 value) {
    while(true) {
        if ((value & ~((i64) SEGMENT_BITS)) == 0) {
            write_byte(fd, value);
            return ;
        }

        write_byte(fd, (value & SEGMENT_BITS) | CONTINUE_BIT);

        value >>= 7;
    }
}

i64 write_string(i32 fd, std::string value) {
    write_varint(fd, value.size());
    return write(fd, value.data(), value.size());
}

std::string entry_to_string(u8 *message, i32 &offset) {
    i32 strlen = read_varint_b(message, offset);
    char buffer[strlen];

    for (i32 i = 0; i < strlen; i++) {
        buffer[i] = message[offset++];
    }

    return std::string(buffer, strlen);
}

void write_uuid(i32 fd, UUID uuid) {
    write(fd, uuid.data(), uuid.size());
}

UUID read_uuid(u8 *buffer, i32 &offset) {
    UUID uuid;
    std::copy_n(buffer + offset, 16, uuid.begin());

    offset += 16;
    return uuid;
}

std::string uuid_to_string(const UUID& uuid) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (i32 i = 0; i < 16; i++) {
        oss << std::setw(2) << static_cast<i32>(uuid[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9)
            oss << '-';
    }

    return oss.str();
}

// thank you sweet sweet ai overlords
int8_t entry_to_byte(uint8_t* message, int32_t& offset) {
	int8_t result = message[offset];
	offset++;
	return result;
}

int16_t entry_to_short(uint8_t* message, int32_t& offset) {
	int16_t result = (
		(int16_t)message[offset  ] << 8 |
		(int16_t)message[offset+1]
	);
	offset+=2;
	return result;
}

int32_t entry_to_integer(uint8_t* message, int32_t& offset) {
	int32_t result = (
		(int32_t)message[offset  ] << 24 |
		(int32_t)message[offset+1] << 16 |
		(int32_t)message[offset+2] <<  8 |
		(int32_t)message[offset+3]
	);
	offset+=4;
	return result;
}

int64_t entry_to_long(uint8_t* message, int32_t& offset) {
	int64_t result = (
		(int64_t)message[offset  ] << 56 |
		(int64_t)message[offset+1] << 48 |
		(int64_t)message[offset+2] << 40 |
		(int64_t)message[offset+3] << 32 |
		(int64_t)message[offset+4] << 24 |
		(int64_t)message[offset+5] << 16 |
		(int64_t)message[offset+6] <<  8 |
		(int64_t)message[offset+7]
	);
	offset+=8;
	return result;
}

float entry_to_float(uint8_t* message, int32_t& offset) {
    uint32_t intBits = (
		(static_cast<uint32_t>(message[offset    ]) << 24) |
		(static_cast<uint32_t>(message[offset + 1]) << 16) |
		(static_cast<uint32_t>(message[offset + 2]) <<  8) |
		(static_cast<uint32_t>(message[offset + 3])      )
	);
    float result;
    std::memcpy(&result, &intBits, sizeof(result)); // Copy bits into a float
    offset += 4;
    return result;
}

double entry_to_double(uint8_t* message, int32_t& offset) {
    uint64_t intBits = (
		(static_cast<uint64_t>(message[offset    ]) << 56) |
		(static_cast<uint64_t>(message[offset + 1]) << 48) |
		(static_cast<uint64_t>(message[offset + 2]) << 40) |
		(static_cast<uint64_t>(message[offset + 3]) << 32) |
		(static_cast<uint64_t>(message[offset + 4]) << 24) |
		(static_cast<uint64_t>(message[offset + 5]) << 16) |
		(static_cast<uint64_t>(message[offset + 6]) <<  8) |
		(static_cast<uint64_t>(message[offset + 7])      )
	);
    double result;
    std::memcpy(&result, &intBits, sizeof(result)); // Copy bits into a double
    offset += 8; // Correctly increment offset by 8 for double
    return result;
}


/*
for larger packets where its not really practical to calculate payload size ahead of time
*/
struct packet_framer {
    std::vector<uint8_t> data;

    void add_u8(uint8_t v) { data.push_back(v); }
    void add_i16(int16_t v) { add_u8(uint8_t(v >> 8)); add_u8(uint8_t(v)); }
    void add_i32(int32_t v) { for (int i = 3; i >= 0; --i) add_u8(uint8_t(v >> (i * 8))); }
    void add_u64(uint64_t v) { for (int i = 7; i >= 0; --i) add_u8(uint8_t(v >> (i * 8))); }
    void varint(int32_t v) {
        uint32_t u = uint32_t(v);
        while (u & ~0x7Fu) { add_u8(uint8_t((u & 0x7F) | 0x80)); u >>= 7; }
        add_u8(uint8_t(u));
    }
    void append(const packet_framer& o) { data.insert(data.end(), o.data.begin(), o.data.end()); }
    size_t size() const { return data.size(); }
};