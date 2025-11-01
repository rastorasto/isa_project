#include "sock.hpp"



Client::Client(int port, std::string server_address, const std::vector<std::string>& blocked) : blocked_domains(blocked), upstream_server(server_address), upstream_port(53) {

    // Create IPv4 socket
    sockipv4 = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockipv4 < 0) {
        throw std::runtime_error("Error creating IPv4 socket");
    }
    printf_debug("IPv4 socket created: %d", sockipv4);

    // Create IPv6 socket
    sockipv6 = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sockipv6 < 0) {
        throw std::runtime_error("Error creating IPv6 socket");
    }
    printf_debug("IPv6 socket created: %d", sockipv6);


    // Set reuse option in both sockets
    int opt = 1;
    if (setsockopt(sockipv4, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Error setting SO_REUSEADDR on IPv4 socket");
    }
    if (setsockopt(sockipv6, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Error setting SO_REUSEADDR on IPv6 socket");
    }

    // Bind ipv4 socket to port
    sockaddr_in addr4{};
    addr4.sin_family = AF_INET;
    addr4.sin_addr.s_addr = INADDR_ANY; // listen on all IPv4 interfaces
    addr4.sin_port = htons(port);       // convert port to network byte order

    if (bind(sockipv4, (sockaddr*)&addr4, sizeof(addr4)) < 0) {
        throw std::runtime_error("Error binding IPv4 socket");
    }
    printf_debug("IPv4 socket bound to port %d", port);

    // Bind ipv6 socket to port
    sockaddr_in6 addr6{};
    addr6.sin6_family = AF_INET6;
    addr6.sin6_addr = in6addr_any;      // listen on all IPv6 interfaces
    addr6.sin6_port = htons(port);

    if (bind(sockipv6, (sockaddr*)&addr6, sizeof(addr6)) < 0) {
        throw std::runtime_error("Error binding IPv6 socket");
    }
    printf_debug("IPv6 socket bound to port %d", port);


    sockserver = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockserver < 0) {
        close(sockipv4);
        close(sockipv6);
        throw std::runtime_error("Error creating server socket");
    }
    printf_debug("Server socket created: %d", sockserver);

    // Set timeout on server socket to prevent indefinite blocking
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (setsockopt(sockserver, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        close(sockipv4);
        close(sockipv6);
        close(sockserver);
        throw std::runtime_error("Error setting socket timeout");
    }
}

Client::~Client() {
    printf_debug("Closing sockets");
    close(sockipv4);
    close(sockipv6);
    close(sockserver);
}

void Client::send_refused(const DNSMessage& message, const sockaddr_storage& client_addr, socklen_t client_len, int sock) {
    uint8_t response[512];
    size_t pos = 0;

    // copies id from the original query
    response[pos++] = (message.id >> 8) & 0xFF;
    response[pos++] = message.id & 0xFF;

    // Flags: QR=1, OPCODE=0, AA=0, TC=0, RD=1, RA=0, Z=0, RCODE=5
    // Response and refused
    // 1000 0001 0000 0101 = 0x8105
    response[pos++] = 0x81;
    response[pos++] = 0x05;

    // Question count (will always be just 1 as the forum says)
    response[pos++] = (message.question_count >> 8) & 0xFF;
    response[pos++] = message.question_count & 0xFF;

    // sets answer, authority and additional counts to 0
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x00;

    // Copy the question section from original query
    // Parse the domain name and reconstruct it
    const std::string& qname = message.question.query_name;
    size_t start = 0;
    size_t dot_pos;
    
    while ((dot_pos = qname.find('.', start)) != std::string::npos) {
        size_t label_len = dot_pos - start;
        if (label_len > 0 && label_len <= 63) {
            response[pos++] = label_len;
            memcpy(&response[pos], qname.c_str() + start, label_len);
            pos += label_len;
        }
        start = dot_pos + 1;
    }
    
    // Handle the last label (after the last dot)
    if (start < qname.length()) {
        size_t label_len = qname.length() - start;
        if (label_len > 0 && label_len <= 63) {
            response[pos++] = label_len;
            memcpy(&response[pos], qname.c_str() + start, label_len);
            pos += label_len;
        }
    }
    
    response[pos++] = 0x00; // null terminator

    // QTYPE
    response[pos++] = (message.question.query_type >> 8) & 0xFF;
    response[pos++] = message.question.query_type & 0xFF;

    // QCLASS
    response[pos++] = (message.question.query_class >> 8) & 0xFF;
    response[pos++] = message.question.query_class & 0xFF;

    // Send response back to client
    ssize_t sent = sendto(sock, response, pos, 0, 
                         (const sockaddr*)&client_addr, client_len);
    
    if (sent < 0) {
        printf_debug("Error sending REFUSED response");
    } else {
        printf_debug("Sent REFUSED response (%zd bytes)", sent);
    }
}

void Client::forward_to_upstream(const uint8_t* query, size_t query_len, const sockaddr_storage& client_addr, socklen_t client_len, int client_sock) {
    // Setup upstream server address
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(upstream_port);
    
    if (inet_pton(AF_INET, upstream_server.c_str(), &server_addr.sin_addr) != 1) {
        throw std::runtime_error("Invalid upstream server address");
        return;
    }

    // Send query to upstream server
    ssize_t sent = sendto(sockserver, query, query_len, 0,
                         (const sockaddr*)&server_addr, sizeof(server_addr));
    
    if (sent < 0) {
        printf_debug("Error forwarding to upstream server");
        return;
    }
    
    printf_debug("Forwarded query to upstream server (%zd bytes)", sent);

    // Wait for response from upstream server
    uint8_t response[512];
    sockaddr_storage server_response_addr;
    socklen_t server_response_len = sizeof(server_response_addr);
    
    ssize_t recv_len = recvfrom(sockserver, response, sizeof(response), 0, (sockaddr*)&server_response_addr, &server_response_len);
    
    if (recv_len < 0) {
        printf_debug("Error or timeout receiving from upstream server");
        return;
    }
    
    printf_debug("Received response from upstream server (%zd bytes)", recv_len);

    // Forward response back to original client
    ssize_t sent_to_client = sendto(client_sock, response, recv_len, 0, (const sockaddr*)&client_addr, client_len);
    
    if (sent_to_client < 0) {
        printf_debug("Error sending response to client");
    } else {
        printf_debug("Forwarded response to client (%zd bytes)", sent_to_client);
    }
}

void Client::handle_client(int sock) {
    uint8_t buffer[512];
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Receive packet form client
    ssize_t n = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &addr_len);
    if (n < 0) return;


    char addr_str[INET6_ADDRSTRLEN];
    uint16_t port = 0;

    // Checks if the query is sent from ipv4 of ipv6 interface and sets the structure for sending
    if (client_addr.ss_family == AF_INET) {
        struct sockaddr_in *a = (struct sockaddr_in*)&client_addr;
        inet_ntop(AF_INET, &a->sin_addr, addr_str, sizeof(addr_str));
        port = ntohs(a->sin_port);
        printf_debug("[IPv4] From %s:%u — %zd bytes\n", addr_str, port, n);
    } else if (client_addr.ss_family == AF_INET6) {
        struct sockaddr_in6 *a6 = (struct sockaddr_in6*)&client_addr;
        inet_ntop(AF_INET6, &a6->sin6_addr, addr_str, sizeof(addr_str));
        port = ntohs(a6->sin6_port);
        printf_debug("[IPv6] From [%s]:%u — %zd bytes\n", addr_str, port, n);
    } else {
        printf_debug("Unknown address family\n");
        return;
    }

    // parses the message from the client
    DNSMessage message = parse_dns_query(buffer, n);

    // compares the query name from the client's message with the blocked domains
    bool is_blocked = domain_blocked(message.question.query_name, blocked_domains);
    
    if (is_blocked) {
        // If the domain is blocked sends refused
        printf_debug("Domain %s is BLOCKED", message.question.query_name.c_str());
        send_refused(message, client_addr, addr_len, sock);
    } else {
        // If the domain is not blocked forwards the message to the upstream dns server
        printf_debug("Domain %s is ALLOWED, forwarding to upstream", 
                    message.question.query_name.c_str());
        forward_to_upstream(buffer, n, client_addr, addr_len, sock);    
    }
}
