#pragma once

#include "util.h"
#include "world.h"

#define UUID_LENGTH 36

class Player {
    private:
        char UUID[UUID_LENGTH + 1] = { 0 };
        std::string username;

    public:
        void set_uuid(std::string value) {
            std::strncpy(UUID, value.c_str(), UUID_LENGTH);
            UUID[UUID_LENGTH] = '\0';
        }

        const std::string get_uuid() { 
            return std::string(UUID);
        }

        void set_username(std::string value) {
            this->username = value;
        }

        std::string get_username() { return this->username; }

};