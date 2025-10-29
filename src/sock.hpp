#pragma once
#include "dnsmessage.hpp"
#include "macro.hpp"
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>

struct Client {
    Client(int port, std::string server_address, const std::vector<std::string>& blocked);
    ~Client(); 

    void handle_client(int sock);
    void send_refused(const DNSMessage& message, const sockaddr_storage& client_addr, socklen_t client_len, int sock);
    void forward_to_upstream(const uint8_t* query, size_t query_len, const sockaddr_storage& client_addr, socklen_t client_len, int client_sock);

    int sockipv4;
    int sockipv6;
    int sockserver;
    std::vector<std::string> blocked_domains;
    std::string upstream_server;
    uint16_t upstream_port;
};

