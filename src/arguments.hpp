#pragma once

#include <iostream>
#include "macro.hpp"
#include "macro.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>

struct Arguments {
    std::string address;
    std::string resolved_address;
    uint16_t port = 53;
    std::string file;
    std::vector<std::string> blocked_domains;

    void help() const;
    Arguments(int argc, char* argv[]);
    void resolve_address();
    void printargs() const;
    void load_domains(const std::string& filename);
    void print_blocked_domains() const;
};