#pragma once
#include <iostream>
#include <cstdint>
class log {
    private:
        static uint32_t level;
    public:
        static void info(const char* message) {
            if (level >= 1) {   
                std::cout << "INFO: " << message << std::endl;
            }
        }
        static void error(const char* message) {
            if (level >= 2) {
                std::cerr << "ERROR: " << message << std::endl;
            }
        }
        static void debug(const char* message);
};