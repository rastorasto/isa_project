#pragma once


#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <arpa/inet.h>
#include <netdb.h>
#include "macro.hpp"

// Includes so it works on eva
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


// Structure for arguments
struct Arguments {
    std::string address;  // address from client
    std::string resolved_address;
    uint16_t port = 53;
    std::string file;
    std::vector<std::string> blocked_domains;
    bool verbose = false;

    // Prints help information
    void help() const;

    // Constructor that parses the arguments and saves them in the structure
    Arguments(int argc, char* argv[]);

    // Resolves the domain name to ip address
    void resolve_address();

    // Prints the arguments for debugging purposes
    void printargs() const;

    // Loads domains from the specified file
    void load_domains(const std::string& filename);

    // Prints the blocked domains for debugging purposes
    void print_blocked_domains() const;
};
