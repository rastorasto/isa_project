/**
 * Author:  Rastislav Uhliar
 * Login:   xuhliar00 
 *
 * Usage: Implementation for header file arguments.hpp
**/

#include "arguments.hpp"

void Arguments::help() const {
    std::cout << "Usage: ./dns -s <server> [-p <port>] -f <filter_file> [-v] [-h]\n"
              << "Options:\n"
              << "  -s <name>       IP Address or hostname of upstream DNS resolver\n"
              << "  -p <port>       Port number on what the program will receive queries (default: 53)\n"
              << "  -f <file>       Specify the input file of blocked domains\n"
              << "  -h              Show this help message\n"
              << "  -v              Show debug information (verbose mode)\n" << std::endl;
}

Arguments::Arguments(int argc, char* argv[]) {
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        printf_debug("Processing argument: %s", argv[i]);
        std::string arg = argv[i];
        if (arg == "-s") {
            if (i + 1 < argc) {
                address = argv[++i];
            }
        } else if (arg == "-p") {
            if (i + 1 < argc) {
                int parsed = std::stoi(argv[++i]);
                if (parsed < 1 || parsed > 65535) {
                    // Port range check
                    throw std::invalid_argument("Invalid port number. Must be between 1 and 65535.");
                }
                port = parsed;
                printf_debug("Parsed port: %d", port);
            }
        } else if (arg == "-f") {
            if (i + 1 < argc) {
                file = argv[++i];
            }
        } else if (arg == "-h") {
            help();
            exit(0);
        } else if (arg == "-v"){
            verbose = true;
        } else {
            throw std::invalid_argument("Invalid argument. Use -h for help.");
        }
    }
    if(address.empty() || file.empty()) {
        throw std::invalid_argument("Missing arguments. Use -h for help.");
    }
}

void Arguments::resolve_address() {
    // printf_debug("Resolving address\n");
    struct sockaddr_in sa;

    // Check if address is already a valid IPv4 address
    if (inet_pton(AF_INET, address.c_str(), &(sa.sin_addr)) == 1) {
        printf_debug("Address is already a valid IPv4 address\n");
        resolved_address = address;
        return;
    }

  
    struct addrinfo hints = {}, *res;
    hints.ai_family = AF_INET; // Handles just IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(address.c_str(), nullptr, &hints, &res) != 0) {
        throw std::invalid_argument("Invalid server address: " + address);
    }

    // Save the resolved address into the address string
    char address[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(((struct sockaddr_in*)res->ai_addr)->sin_addr), address, INET_ADDRSTRLEN) == nullptr) {
        printf_debug("Failed to convert resolved address to string.");
        freeaddrinfo(res);
        throw std::invalid_argument("Address resolution failed.");
    }
    resolved_address = address;
    freeaddrinfo(res);
}

void Arguments::printargs() const {
    printf_debug("Server address: %s", address.c_str());
    printf_debug("Resolved address: %s", resolved_address.c_str());
    printf_debug("Port: %d", port);
    printf_debug("Filter file: %s", file.c_str());
}

void Arguments::print_blocked_domains() const {
    printf_debug("Blocked domains:");
    for (const auto& domain : blocked_domains) {
        if (!gverbose) (void)domain;
        printf_debug(" - %s", domain.c_str());
    }
}


void Arguments::load_domains(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    // Goes thrue every line and if they are not comments or empty saves them to the blocked_domains vector
    while (std::getline(file, line)) {

        if (line.empty() || line[0] == '#') continue;

        blocked_domains.push_back(line);
    }
    file.close();
}
