#pragma once
#include "dnsmessage.hpp"
#include "macro.hpp"
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>

// Includes for eva
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct Client {
    Client(int port, std::string server_address, const std::vector<std::string>& blocked);
    ~Client(); 

    // handles message from the client
    void handle_client(int sock);
    // sends client message that the program refused the domain
    void send_refused(const DNSMessage& message, const sockaddr_storage& client_addr, socklen_t client_len, int sock);
    // forwards the client message to the upstream dns server
    void forward_to_upstream(const uint8_t* query, size_t query_len, const sockaddr_storage& client_addr, socklen_t client_len, int client_sock);

    // sockets that are used
    int sockipv4;
    int sockipv6;
    int sockserver;
    // vector of blocked_domains
    std::vector<std::string> blocked_domains;
    // upstream server name
    std::string upstream_server;
    // port on which the upstream server receives the dns query on
    uint16_t upstream_port;
};

